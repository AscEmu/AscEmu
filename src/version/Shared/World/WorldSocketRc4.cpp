/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "world/Server/WorldSocket.hpp"

#include "Cryptography/Sha256.hpp"
#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "Utilities/Util.hpp"
#include "world/Objects/Units/Players/PlayerDefines.hpp"
#include "world/Server/BattleNetCommClient/BattleNetCommClient.hpp"
#include "world/Server/LogonCommClient/LogonCommHandler.h"
#include "world/Server/Opcodes.hpp"
#include "world/Server/Packets/CmsgAuthSession.h"
#include "world/Server/Packets/SmsgAuthChallenge.h"
#include "world/Server/World.h"
#include "world/Server/WorldConfig.h"
#include "world/Server/WorldSession.h"
#include "world/Version/VersionRegistry.hpp"
#include "version/Shared/World/WorldConnectKey.hpp"

#if AE_WORLD_PROFILE_WOD
#include "version/WoD/World/WorldProfile.hpp"
#elif AE_WORLD_PROFILE_LEGION
#include "version/Legion/World/WorldProfile.hpp"
#endif

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <zlib.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <memory>
#include <string_view>
#include <vector>

using namespace AscEmu::Packets;

#if AE_WORLD_PROFILE_WOD || AE_WORLD_PROFILE_LEGION

namespace
{
    namespace Profile = AscEmu::Version::WorldProfile;

    namespace Rc4World
    {
        constexpr size_t ClientInitializerPacketSize = (Profile::InitializerHasMagic ? sizeof(uint32_t) + sizeof(uint16_t) : 0) + Profile::ClientInitializer.size();

        constexpr uint32_t SetupHeaderSize = sizeof(uint16_t) + sizeof(uint16_t);
        constexpr uint32_t NormalHeaderSize = sizeof(uint32_t) + sizeof(uint16_t);

        // encrypted packets above this size are sent compressed
        constexpr uint32_t MinSizeForCompression = 0x400;
        constexpr uLong CompressionAdlerSeed = 0x9827D8F1;

        constexpr std::array<uint8_t, 16> AuthCheckSeed = { 0xC5, 0xC6, 0x98, 0x95, 0x76, 0x3F, 0x1D, 0xCD, 0xB6, 0xA1, 0x37, 0x28, 0xB3, 0x12, 0xFF, 0x8A };
        constexpr std::array<uint8_t, 16> SessionKeySeed = { 0x58, 0xCB, 0xCF, 0x40, 0xFE, 0x2E, 0xCE, 0xA6, 0x5A, 0x90, 0xB8, 0x01, 0x68, 0x6C, 0x28, 0x0B };
    }

    uint16_t readUInt16LE(const uint8_t* data)
    {
        return static_cast<uint16_t>(data[0] | (data[1] << 8));
    }

    uint32_t readUInt32LE(const uint8_t* data)
    {
        return static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8) | (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[3]) << 24);
    }

    void appendUInt16LE(std::vector<uint8_t>& out, uint16_t value)
    {
        out.push_back(static_cast<uint8_t>(value & 0xFF));
        out.push_back(static_cast<uint8_t>(value >> 8));
    }

    void appendUInt32LE(std::vector<uint8_t>& out, uint32_t value)
    {
        for (uint32_t shift = 0; shift < 32; shift += 8)
            out.push_back(static_cast<uint8_t>((value >> shift) & 0xFF));
    }

    // 40 byte key stream: o0 = SHA256(o1 | o0 | o2) with o1/o2 the SHA256 of both halves of the seed
    void generateSessionKey(const uint8_t* seed, size_t seedSize, std::array<uint8_t, 40>& sessionKey)
    {
        const size_t half = seedSize / 2;

        std::array<uint8_t, Sha256Hash::DigestLength> o0{};
        std::array<uint8_t, Sha256Hash::DigestLength> o1{};
        std::array<uint8_t, Sha256Hash::DigestLength> o2{};
        Sha256Hash::hash(seed, half, o1.data());
        Sha256Hash::hash(seed + half, seedSize - half, o2.data());

        size_t taken = o0.size();
        for (uint8_t& value : sessionKey)
        {
            if (taken == o0.size())
            {
                std::array<uint8_t, Sha256Hash::DigestLength * 3> input{};
                std::memcpy(input.data(), o1.data(), o1.size());
                std::memcpy(input.data() + o1.size(), o0.data(), o0.size());
                std::memcpy(input.data() + o1.size() + o0.size(), o2.data(), o2.size());
                Sha256Hash::hash(input.data(), input.size(), o0.data());
                taken = 0;
            }

            value = o0[taken++];
        }
    }

    // digest of CMSG_AUTH_SESSION: HMAC-SHA256(local challenge | server challenge | check seed), first 24 bytes compared
    bool digestMatches(const std::array<uint8_t, Sha256Hash::DigestLength>& key, const std::array<uint8_t, 16>& localChallenge, const std::array<uint8_t, 16>& serverChallenge, const std::array<uint8_t, 24>& received)
    {
        std::array<uint8_t, 48> input{};
        std::memcpy(input.data(), localChallenge.data(), 16);
        std::memcpy(input.data() + 16, serverChallenge.data(), 16);
        std::memcpy(input.data() + 32, Rc4World::AuthCheckSeed.data(), 16);

        std::array<uint8_t, Sha256Hash::DigestLength> digest{};
        Sha256Hash::hmac(key.data(), key.size(), input.data(), input.size(), digest.data());

        return CRYPTO_memcmp(digest.data(), received.data(), received.size()) == 0;
    }

    void destroyCompressionStream(void* pointer)
    {
        auto* stream = static_cast<z_stream*>(pointer);
        if (stream == nullptr)
            return;

        deflateEnd(stream);
        delete stream;
    }
}

bool WorldSocket::initializeVersionedConnection()
{
    m_protocolSetByLogonComm = false;
    setClientProtocol(WoW::ClientProtocol{.expansion = Profile::Expansion});

    m_rc4WorldState = Rc4WorldState::AwaitClientInitializer;
    m_rc4LargeClientHeader = !Profile::SetupHeaderBeforeAuthSession;
    m_rc4HeaderReady = false;
    m_rc4PacketOpcode = 0;
    m_rc4PacketSize = 0;
    m_rc4PendingOutput.clear();
    m_rc4ServerChallenge.fill(0);
    m_rc4SessionKey.fill(0);
    m_rc4GameAccountId = 0;
    m_rc4GameAccountName.clear();
    m_rc4ChallengeSeeds.fill(0);
    m_rc4InstanceConnection = false;
    m_rc4ConnectToKey = 0;

    sLogger.debug("WorldSocket::{}: connection from {}:{}", Profile::Name, getRemoteIp(), getRemotePort());

    if (!sendRc4Initializer())
    {
        sLogger.failure("WorldSocket::{}: failed to send the connection initializer to {}:{}", Profile::Name, getRemoteIp(), getRemotePort());
        m_rc4WorldState = Rc4WorldState::Disabled;
        disconnect();
    }

    return true;
}

bool WorldSocket::sendRc4Initializer()
{
    std::vector<uint8_t> initializer;
    if constexpr (Profile::InitializerHasMagic)
    {
        appendUInt32LE(initializer, Profile::InitializerMagic);
        appendUInt16LE(initializer, static_cast<uint16_t>(Profile::ServerInitializer.size()));
    }
    initializer.insert(initializer.end(), Profile::ServerInitializer.begin(), Profile::ServerInitializer.end());

    burstBegin();
    const bool sent = burstSend(initializer.data(), static_cast<uint32_t>(initializer.size()));
    if (sent)
        burstPush();
    burstEnd();

    return sent;
}

bool WorldSocket::processVersionedRead()
{
    if (m_rc4WorldState == Rc4WorldState::Disabled)
        return false;

    flushRc4Output();

    if (m_rc4WorldState == Rc4WorldState::AwaitClientInitializer && !processRc4Initializer())
        return true;

    while (m_rc4WorldState != Rc4WorldState::Disabled && processRc4Packet())
    {
    }

    return true;
}

bool WorldSocket::processRc4Initializer()
{
    if (readBuffer.GetSize() < Rc4World::ClientInitializerPacketSize)
        return false;

    std::array<uint8_t, Rc4World::ClientInitializerPacketSize> initializer{};
    if (!readBuffer.Read(initializer.data(), static_cast<uint32_t>(initializer.size())))
        return false;

    bool valid = true;
    size_t offset = 0;
    if constexpr (Profile::InitializerHasMagic)
    {
        valid = readUInt32LE(initializer.data()) == Profile::InitializerMagic
            && readUInt16LE(initializer.data() + sizeof(uint32_t)) == Profile::ClientInitializer.size();
        offset = sizeof(uint32_t) + sizeof(uint16_t);
    }

    const std::string_view received(reinterpret_cast<const char*>(initializer.data() + offset), Profile::ClientInitializer.size());
    if (!valid || received != Profile::ClientInitializer)
    {
        sLogger.failure("WorldSocket::{}: invalid connection initializer from {}:{}", Profile::Name, getRemoteIp(), getRemotePort());
        m_rc4WorldState = Rc4WorldState::Disabled;
        disconnect();
        return false;
    }

    auto* stream = new z_stream();
    std::memset(stream, 0, sizeof(z_stream));
    const int result = deflateInit2(stream, Z_BEST_SPEED, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    if (result != Z_OK)
    {
        delete stream;
        sLogger.failure("WorldSocket::{}: cannot initialize packet compression (zlib error {})", Profile::Name, result);
        m_rc4WorldState = Rc4WorldState::Disabled;
        disconnect();
        return false;
    }
    m_rc4CompressionStream = std::shared_ptr<void>(stream, destroyCompressionStream);

    // the server challenge and the two seeds of an instance connection
    SmsgAuthChallenge challenge;
    if (RAND_bytes(m_rc4ServerChallenge.data(), static_cast<int>(m_rc4ServerChallenge.size())) != 1
        || RAND_bytes(challenge.dosChallenge.data(), static_cast<int>(challenge.dosChallenge.size())) != 1)
    {
        sLogger.failure("WorldSocket::{}: RAND_bytes failed while creating the auth challenge", Profile::Name);
        m_rc4WorldState = Rc4WorldState::Disabled;
        disconnect();
        return false;
    }
    challenge.challenge = m_rc4ServerChallenge;
    m_rc4ChallengeSeeds = challenge.dosChallenge;

    m_rc4WorldState = Rc4WorldState::AwaitAuthSession;
    sendManagedPacket(challenge);
    return true;
}

bool WorldSocket::processRc4Packet()
{
    if (!m_rc4HeaderReady)
    {
        const uint32_t headerSize = m_rc4LargeClientHeader ? Rc4World::NormalHeaderSize : Rc4World::SetupHeaderSize;
        if (readBuffer.GetSize() < headerSize)
            return false;

        std::array<uint8_t, Rc4World::NormalHeaderSize> header{};
        if (!readBuffer.Read(header.data(), headerSize))
            return false;

        // only the size field is encrypted
        if (m_crypt.isInitialized())
            m_crypt.decryptWotlkReceive(header.data(), sizeof(uint32_t));

        uint32_t size;
        if (m_rc4LargeClientHeader)
        {
            size = readUInt32LE(header.data());
            m_rc4PacketOpcode = readUInt16LE(header.data() + sizeof(uint32_t));
        }
        else
        {
            size = readUInt16LE(header.data());
            m_rc4PacketOpcode = readUInt16LE(header.data() + sizeof(uint16_t));
        }

        // the size counts the opcode
        if (size < sizeof(uint16_t) || size - sizeof(uint16_t) >= Profile::MaxClientPacketSize)
        {
            sLogger.failure("WorldSocket::{}: {}:{} sent a malformed packet (size {}, opcode 0x{:04X})", Profile::Name, getRemoteIp(), getRemotePort(), size, m_rc4PacketOpcode);
            m_rc4WorldState = Rc4WorldState::Disabled;
            disconnect();
            return false;
        }

        m_rc4PacketSize = size - static_cast<uint32_t>(sizeof(uint16_t));
        m_rc4HeaderReady = true;
    }

    if (readBuffer.GetSize() < m_rc4PacketSize)
        return false;

    auto packet = std::make_unique<WorldPacket>(m_rc4PacketOpcode, m_rc4PacketSize);
    packet->resize(m_rc4PacketSize);
    if (m_rc4PacketSize != 0)
        readBuffer.Read(packet->contents(), m_rc4PacketSize);

    m_rc4HeaderReady = false;

    const uint32_t opcode = Version::opcodeIdForHex(m_rc4PacketOpcode, m_protocol);
    sWorldPacketLog.logPacket(m_rc4PacketSize, static_cast<uint16_t>(opcode), m_rc4PacketSize ? packet->contents() : nullptr, 0, (m_session ? m_session->GetAccountId() : 0), m_protocol.expansion);

    if (opcode == CMSG_AUTH_SESSION)
    {
        m_rc4LargeClientHeader = true;

        if (m_rc4WorldState != Rc4WorldState::AwaitAuthSession)
        {
            sLogger.failure("WorldSocket::{}: {}:{} sent a second CMSG_AUTH_SESSION", Profile::Name, getRemoteIp(), getRemotePort());
            m_rc4WorldState = Rc4WorldState::Disabled;
            disconnect();
            return false;
        }

        handleRc4AuthSession(*packet);
        return m_rc4WorldState != Rc4WorldState::Disabled;
    }

    if (opcode == CMSG_AUTH_CONTINUED_SESSION)
    {
        m_rc4LargeClientHeader = true;

        if (m_rc4WorldState != Rc4WorldState::AwaitAuthSession)
        {
            sLogger.failure("WorldSocket::{}: {}:{} sent an unexpected CMSG_AUTH_CONTINUED_SESSION", Profile::Name, getRemoteIp(), getRemotePort());
            m_rc4WorldState = Rc4WorldState::Disabled;
            disconnect();
            return false;
        }

        handleRc4AuthContinuedSession(*packet);
        return m_rc4WorldState != Rc4WorldState::Disabled;
    }

    if (opcode == CMSG_ENABLE_ENCRYPTION_ACK)
    {
        if (m_rc4WorldState != Rc4WorldState::AwaitEncryptionAck)
        {
            sLogger.failure("WorldSocket::{}: {}:{} sent an unexpected CMSG_ENABLE_ENCRYPTION_ACK", Profile::Name, getRemoteIp(), getRemotePort());
            m_rc4WorldState = Rc4WorldState::Disabled;
            disconnect();
            return false;
        }

        handleRc4EnableEncryptionAck();
        return m_rc4WorldState != Rc4WorldState::Disabled;
    }

    dispatchPacket(std::move(packet));
    return true;
}

void WorldSocket::handleRc4AuthSession(WorldPacket& packet)
{
    CmsgAuthSession authSession;
    if (!parsePacket(packet, authSession))
    {
        sLogger.failure("WorldSocket::{}: malformed CMSG_AUTH_SESSION from {}:{}: {}", Profile::Name, getRemoteIp(), getRemotePort(), authSession.errorMsg);
        m_rc4WorldState = Rc4WorldState::Disabled;
        disconnect();
        return;
    }

    m_latency = Util::getMSTime() - m_latency;

    // the Battle.net server announced the session with the realm join ticket before the client connected;
    // it is consumed only after the digest matched
    AscEmu::BattlenetComm::PendingWorldSession pending;
    if (!AscEmu::BattlenetComm::sBattleNetCommClient.getPendingSession(authSession.realmJoinTicket, pending, false))
    {
        sLogger.failure("WorldSocket::{}: realm join ticket from {}:{} has no pending Battle.net session", Profile::Name, getRemoteIp(), getRemotePort());
        m_rc4WorldState = Rc4WorldState::Disabled;
        disconnect();
        return;
    }

    // key data of the join: client secret and join secret
    std::array<uint8_t, Sha256Hash::DigestLength> keyDataHash{};
    Sha256Hash::hash(pending.worldAuthKeyData.data(), pending.worldAuthKeyData.size(), keyDataHash.data());

    bool verified = false;
    if constexpr (Profile::AuthSeeds.empty())
    {
        verified = digestMatches(keyDataHash, authSession.localChallenge, m_rc4ServerChallenge, authSession.sessionDigest);
    }
    else
    {
        // the seed depends on the client platform, the session does not carry it
        for (const auto& seed : Profile::AuthSeeds)
        {
            std::array<uint8_t, 64 + 16> input{};
            std::memcpy(input.data(), pending.worldAuthKeyData.data(), 64);
            std::memcpy(input.data() + 64, seed.data(), seed.size());

            std::array<uint8_t, Sha256Hash::DigestLength> digestKey{};
            Sha256Hash::hash(input.data(), input.size(), digestKey.data());

            if (digestMatches(digestKey, authSession.localChallenge, m_rc4ServerChallenge, authSession.sessionDigest))
            {
                verified = true;
                break;
            }
        }
    }

    if (!verified)
    {
        sLogger.failure("WorldSocket::{}: digest mismatch for game account {} ('{}') from {}:{}", Profile::Name, pending.gameAccountId, pending.gameAccountName, getRemoteIp(), getRemotePort());
        m_rc4WorldState = Rc4WorldState::Disabled;
        disconnect();
        return;
    }

    AscEmu::BattlenetComm::PendingWorldSession consumed;
    if (!AscEmu::BattlenetComm::sBattleNetCommClient.getPendingSession(authSession.realmJoinTicket, consumed, true))
    {
        sLogger.failure("WorldSocket::{}: pending session of game account {} expired during authentication", Profile::Name, pending.gameAccountId);
        m_rc4WorldState = Rc4WorldState::Disabled;
        disconnect();
        return;
    }

    // world session key: key stream seeded with HMAC-SHA256(server challenge | local challenge | seed)
    std::array<uint8_t, 48> sessionInput{};
    std::memcpy(sessionInput.data(), m_rc4ServerChallenge.data(), 16);
    std::memcpy(sessionInput.data() + 16, authSession.localChallenge.data(), 16);
    std::memcpy(sessionInput.data() + 32, Rc4World::SessionKeySeed.data(), 16);

    std::array<uint8_t, Sha256Hash::DigestLength> sessionSeed{};
    Sha256Hash::hmac(keyDataHash.data(), keyDataHash.size(), sessionInput.data(), sessionInput.size(), sessionSeed.data());
    generateSessionKey(sessionSeed.data(), sessionSeed.size(), m_rc4SessionKey);

    auto protocol = getClientProtocol();
    protocol.regionId = authSession.regionId;
    protocol.battlegroupId = authSession.battlegroupId;
    protocol.realmId = authSession.realmId;
    setClientProtocol(protocol);

    m_accountName = pending.gameAccountName;
    m_clientBuild = authSession.clientBuild;
    m_addonInfoBuffer = std::move(authSession.addonInfoBuffer);
    m_rc4GameAccountId = pending.gameAccountId;
    m_rc4GameAccountName = pending.gameAccountName;

    sLogger.debug("WorldSocket::{}: game account {} ('{}') authenticated from {}:{}", Profile::Name, pending.gameAccountId, pending.gameAccountName, getRemoteIp(), getRemotePort());

    if constexpr (Profile::EnableEncryptionHandshake)
    {
        // the client acknowledges before the encryption starts in both directions
        m_rc4WorldState = Rc4WorldState::AwaitEncryptionAck;

        WorldPacket enableEncryption(SMSG_ENABLE_ENCRYPTION, 0);
        sendPacket(&enableEncryption);
    }
    else
    {
        m_crypt.initForClientVersion(static_cast<uint8_t>(m_protocol.expansion), m_rc4SessionKey.data());
        completeRc4Authentication();
    }
}

void WorldSocket::handleRc4EnableEncryptionAck()
{
    if (m_rc4InstanceConnection)
    {
        attachRc4InstanceConnection();
        return;
    }

    m_crypt.initForClientVersion(static_cast<uint8_t>(m_protocol.expansion), m_rc4SessionKey.data());
    completeRc4Authentication();
}

void WorldSocket::attachRc4InstanceConnection()
{
    // the seeds of the challenge key this connection; it joins the session that asked for it
    m_crypt.initSeededCrypt(m_rc4SessionKey.data(), m_rc4ChallengeSeeds.data(), m_rc4ChallengeSeeds.data() + 16);

    WorldSession* session = sWorld.getSessionByAccountId(m_rc4GameAccountId);
    if (!m_crypt.isInitialized() || session == nullptr || session->getInstanceConnectKey() != m_rc4ConnectToKey)
    {
        sLogger.failure("WorldSocket::{}: the session of game account {} is gone, second connection closed", Profile::Name, m_rc4GameAccountId);
        m_rc4WorldState = Rc4WorldState::Disabled;
        disconnect();
        return;
    }

    m_rc4WorldState = Rc4WorldState::Authenticated;
    m_session = session;
    session->setInstanceSocket(this);

    // the session continues the login in its own update
    session->QueuePacket(std::make_unique<WorldPacket>(Version::opcodeHexFor(CMSG_AUTH_CONTINUED_SESSION, m_protocol), 0));
}

namespace
{
    // SMSG_CONNECT_TO carries the address signed with the key pair whose public part the client knows
    namespace ConnectTo
    {
        constexpr std::array<uint8_t, 16> ContinuedSessionSeed = { 0x16, 0xAD, 0x0C, 0xD4, 0x46, 0xF9, 0x4F, 0xB2, 0xEF, 0x7D, 0xEA, 0x2A, 0x17, 0x66, 0x4D, 0x2F };

        constexpr uint8_t ConnectionTypeInstance = 1;
        constexpr uint8_t AddressTypeIPv4 = 1;
        constexpr uint32_t PayloadChecksum = 0xA0A66C10;
        constexpr uint8_t XorMagic = 0x2A;
        constexpr size_t SignatureSize = 256;

        constexpr std::array<uint8_t, 64> WhereHmacKey =
        {
            0x2C, 0x1F, 0x1D, 0x80, 0xC3, 0x8C, 0x23, 0x64, 0xDA, 0x90, 0xCA, 0x8E, 0x2C, 0xFC, 0x0C, 0xCE,
            0x09, 0xD3, 0x62, 0xF9, 0xF3, 0x8B, 0xBE, 0x9F, 0x19, 0xEF, 0x58, 0xA1, 0x1C, 0x34, 0x14, 0x41,
            0x3F, 0x23, 0xFD, 0xD3, 0xE8, 0x14, 0xEC, 0x2A, 0xFD, 0x4F, 0x95, 0xBA, 0x30, 0x7E, 0x56, 0x5D,
            0x83, 0x95, 0x81, 0x69, 0xB0, 0x5A, 0xB4, 0x9D, 0xA8, 0x55, 0xFF, 0xFC, 0xEE, 0x58, 0x0A, 0x2F
        };

        constexpr std::array<uint8_t, 32> PanamaKey =
        {
            0xF4, 0x1D, 0xCB, 0x2D, 0x72, 0x8C, 0xF3, 0x33, 0x7A, 0x4F, 0xF3, 0x38, 0xFA, 0x89, 0xDB, 0x01,
            0xBB, 0xBE, 0x9C, 0x3B, 0x65, 0xE9, 0xDA, 0x96, 0x26, 0x86, 0x87, 0x35, 0x3E, 0x48, 0xB9, 0x4C
        };

        // 68 characters and three zero bytes
        constexpr char Haiku[71] = "An island of peace\nCorruption is brought ashore\nPandarens will rise\n\0";

        constexpr std::array<uint8_t, 108> PiDigits =
        {
            0x31, 0x41, 0x59, 0x26, 0x53, 0x58, 0x97, 0x93, 0x23, 0x84, 0x62, 0x64, 0x33, 0x83, 0x27, 0x95, 0x02, 0x88,
            0x41, 0x97, 0x16, 0x93, 0x99, 0x37, 0x51, 0x05, 0x82, 0x09, 0x74, 0x94, 0x45, 0x92, 0x30, 0x78, 0x16, 0x40,
            0x62, 0x86, 0x20, 0x89, 0x98, 0x62, 0x80, 0x34, 0x82, 0x53, 0x42, 0x11, 0x70, 0x67, 0x98, 0x21, 0x48, 0x08,
            0x65, 0x13, 0x28, 0x23, 0x06, 0x64, 0x70, 0x93, 0x84, 0x46, 0x09, 0x55, 0x05, 0x82, 0x23, 0x17, 0x25, 0x35,
            0x94, 0x08, 0x12, 0x84, 0x81, 0x11, 0x74, 0x50, 0x28, 0x41, 0x02, 0x70, 0x19, 0x38, 0x52, 0x11, 0x05, 0x55,
            0x96, 0x44, 0x62, 0x29, 0x48, 0x95, 0x49, 0x30, 0x38, 0x19, 0x64, 0x42, 0x88, 0x10, 0x97, 0x56, 0x65, 0x93
        };

        constexpr const char* PrivateKey = AscEmu::Version::WorldConnectKey::PrivateKey;

        // raw private key operation over the little endian payload, the result is little endian as well
        bool sign(std::array<uint8_t, SignatureSize> payload, uint8_t* output)
        {
            std::reverse(payload.begin(), payload.end());

            BIO* keyBio = BIO_new_mem_buf(PrivateKey, -1);
            if (keyBio == nullptr)
                return false;

            EVP_PKEY* key = PEM_read_bio_PrivateKey(keyBio, nullptr, nullptr, nullptr);
            BIO_free(keyBio);
            if (key == nullptr)
                return false;

            bool signedPayload = false;
            if (EVP_PKEY_CTX* context = EVP_PKEY_CTX_new(key, nullptr))
            {
                size_t outputSize = SignatureSize;
                signedPayload = EVP_PKEY_sign_init(context) > 0
                    && EVP_PKEY_CTX_set_rsa_padding(context, RSA_NO_PADDING) > 0
                    && EVP_PKEY_sign(context, output, &outputSize, payload.data(), payload.size()) > 0
                    && outputSize == SignatureSize;

                EVP_PKEY_CTX_free(context);
            }

            EVP_PKEY_free(key);

            if (signedPayload)
                std::reverse(output, output + SignatureSize);

            return signedPayload;
        }
    }
}

bool WorldSocket::sendConnectTo(uint64_t key, uint32_t serial)
{
    // the address the client reached this connection with, the second connection uses the world port as well
    sockaddr_in local{};
#ifdef _WIN32
    int localSize = sizeof(local);
#else
    socklen_t localSize = sizeof(local);
#endif
    if (getsockname(getFd(), reinterpret_cast<sockaddr*>(&local), &localSize) != 0 || local.sin_family != AF_INET)
    {
        sLogger.failure("WorldSocket::{}: cannot read the local address for SMSG_CONNECT_TO", Profile::Name);
        return false;
    }

    std::array<uint8_t, 16> where{};
    std::memcpy(where.data(), &local.sin_addr, 4);

    const uint8_t addressType = ConnectTo::AddressTypeIPv4;
    const uint16_t port = static_cast<uint16_t>(worldConfig.listen.listenPort);
    const uint8_t portBytes[2] = { static_cast<uint8_t>(port & 0xFF), static_cast<uint8_t>(port >> 8) };

    // HMAC-SHA1 over address, type, port, haiku, panama key, pi digits and the xor magic
    std::vector<uint8_t> hmacInput;
    hmacInput.insert(hmacInput.end(), where.begin(), where.end());
    hmacInput.push_back(addressType);
    hmacInput.insert(hmacInput.end(), portBytes, portBytes + 2);
    hmacInput.insert(hmacInput.end(), ConnectTo::Haiku, ConnectTo::Haiku + sizeof(ConnectTo::Haiku));
    hmacInput.insert(hmacInput.end(), ConnectTo::PanamaKey.begin(), ConnectTo::PanamaKey.end());
    hmacInput.insert(hmacInput.end(), ConnectTo::PiDigits.begin(), ConnectTo::PiDigits.end());
    hmacInput.push_back(ConnectTo::XorMagic);

    std::array<uint8_t, 20> hmacDigest{};
    unsigned int hmacSize = 0;
    if (HMAC(EVP_sha1(), ConnectTo::WhereHmacKey.data(), static_cast<int>(ConnectTo::WhereHmacKey.size()), hmacInput.data(), hmacInput.size(), hmacDigest.data(), &hmacSize) == nullptr || hmacSize != hmacDigest.size())
        return false;

    // checksum, type, address, port, haiku, panama key, pi digits, xor magic, hmac; zero filled up to the key size
    std::vector<uint8_t> payload;
    appendUInt32LE(payload, ConnectTo::PayloadChecksum);
    payload.push_back(addressType);
    payload.insert(payload.end(), where.begin(), where.end());
    payload.insert(payload.end(), portBytes, portBytes + 2);
    payload.insert(payload.end(), ConnectTo::Haiku, ConnectTo::Haiku + sizeof(ConnectTo::Haiku));
    payload.insert(payload.end(), ConnectTo::PanamaKey.begin(), ConnectTo::PanamaKey.end());
    payload.insert(payload.end(), ConnectTo::PiDigits.begin(), ConnectTo::PiDigits.end());
    payload.push_back(ConnectTo::XorMagic);
    payload.insert(payload.end(), hmacDigest.begin(), hmacDigest.end());

    std::array<uint8_t, ConnectTo::SignatureSize> block{};
    if (payload.size() > block.size())
        return false;
    std::memcpy(block.data(), payload.data(), payload.size());

    std::array<uint8_t, ConnectTo::SignatureSize> signature{};
    if (!ConnectTo::sign(block, signature.data()))
    {
        sLogger.failure("WorldSocket::{}: cannot sign SMSG_CONNECT_TO", Profile::Name);
        return false;
    }

    WorldPacket connectTo(SMSG_CONNECT_TO, 8 + 4 + ConnectTo::SignatureSize + 1);
    connectTo << uint64_t(key);
    connectTo << uint32_t(serial);
    connectTo.append(signature.data(), signature.size());
    connectTo << uint8_t(ConnectTo::ConnectionTypeInstance);
    sendPacket(&connectTo);

    sLogger.debug("WorldSocket::{}: sent SMSG_CONNECT_TO (serial {}) to game account {}", Profile::Name, serial, m_rc4GameAccountId);
    return true;
}

void WorldSocket::handleRc4AuthContinuedSession(WorldPacket& packet)
{
    const auto reject = [this](const char* reason)
    {
        sLogger.failure("WorldSocket::{}: continued session from {}:{} rejected: {}", Profile::Name, getRemoteIp(), getRemotePort(), reason);
        m_rc4WorldState = Rc4WorldState::Disabled;
        disconnect();
    };

    // dos response, key, local challenge, digest
    uint64_t dosResponse = 0;
    uint64_t key = 0;
    std::array<uint8_t, 16> localChallenge{};
    std::array<uint8_t, 24> digest{};

    if (packet.size() < sizeof(dosResponse) + sizeof(key) + localChallenge.size() + digest.size())
    {
        reject("malformed CMSG_AUTH_CONTINUED_SESSION");
        return;
    }

    packet >> dosResponse >> key;
    packet.read(localChallenge.data(), localChallenge.size());
    packet.read(digest.data(), digest.size());

    // key: account (32 bit), connection type (1 bit), random part (31 bit)
    const uint32_t accountId = static_cast<uint32_t>(key & 0xFFFFFFFF);
    const uint8_t connectionType = static_cast<uint8_t>((key >> 32) & 1);
    if (connectionType != ConnectTo::ConnectionTypeInstance)
    {
        reject("unexpected connection type");
        return;
    }

    WorldSession* session = sWorld.getSessionByAccountId(accountId);
    WorldSocket* realmSocket = session != nullptr ? session->GetSocket() : nullptr;
    if (session == nullptr || realmSocket == nullptr || session->getInstanceConnectKey() != key || session->getInstanceConnectKey() == 0)
    {
        reject("no session waits for this key");
        return;
    }

    // HMAC-SHA256 keyed by the session key over key, local challenge, server challenge and the seed
    std::array<uint8_t, 8 + 16 + 16 + 16> input{};
    std::memcpy(input.data(), &key, 8);
    std::memcpy(input.data() + 8, localChallenge.data(), 16);
    std::memcpy(input.data() + 24, m_rc4ServerChallenge.data(), 16);
    std::memcpy(input.data() + 40, ConnectTo::ContinuedSessionSeed.data(), 16);

    std::array<uint8_t, Sha256Hash::DigestLength> expected{};
    Sha256Hash::hmac(realmSocket->m_rc4SessionKey.data(), realmSocket->m_rc4SessionKey.size(), input.data(), input.size(), expected.data());

    if (CRYPTO_memcmp(expected.data(), digest.data(), digest.size()) != 0)
    {
        reject("digest mismatch");
        return;
    }

    setClientProtocol(realmSocket->getClientProtocol());
    m_rc4SessionKey = realmSocket->m_rc4SessionKey;
    m_rc4GameAccountId = accountId;
    m_rc4GameAccountName = realmSocket->m_rc4GameAccountName;
    m_rc4InstanceConnection = true;
    m_rc4ConnectToKey = key;

    sLogger.debug("WorldSocket::{}: second connection of game account {} authenticated from {}:{}", Profile::Name, accountId, getRemoteIp(), getRemotePort());

    if constexpr (Profile::EnableEncryptionHandshake)
    {
        m_rc4WorldState = Rc4WorldState::AwaitEncryptionAck;

        WorldPacket enableEncryption(SMSG_ENABLE_ENCRYPTION, 0);
        sendPacket(&enableEncryption);
    }
    else
    {
        // without the handshake the encryption starts with the next packet in both directions
        attachRc4InstanceConnection();
    }
}

void WorldSocket::completeRc4Authentication()
{
    if (!m_crypt.isInitialized())
    {
        sLogger.failure("WorldSocket::{}: cannot initialize the packet encryption for game account {}", Profile::Name, m_rc4GameAccountId);
        m_rc4WorldState = Rc4WorldState::Disabled;
        disconnect();
        return;
    }

    m_rc4WorldState = Rc4WorldState::Authenticated;

    // Battle.net accounts own every expansion of the client
    constexpr uint8_t accountFlags = ACCOUNT_FLAG_XPACK_01 | ACCOUNT_FLAG_XPACK_02 | ACCOUNT_FLAG_XPACK_03 | ACCOUNT_FLAG_XPACK_04;

    completeAuthentication(m_rc4GameAccountId, m_rc4GameAccountName, sLogonCommHandler.getPermissionStringForAccountId(m_rc4GameAccountId), accountFlags, "enUS", 0);
}

bool WorldSocket::sendVersionedPacket(WorldPacket* packet)
{
    if (m_rc4WorldState == Rc4WorldState::Disabled)
        return false;

    if (packet == nullptr)
        return true;

    const uint16_t rawOpcode = Version::opcodeHexFor(packet->getOpcode(), m_protocol);
    if (rawOpcode == 0)
    {
        sLogger.debug("WorldSocket::{}: dropped {} without an opcode of this client ({} bytes)", Profile::Name, Version::opcodeNameForId(packet->getOpcode(), m_protocol.expansion), packet->size());
        return true;
    }

    sWorldPacketLog.logPacket(static_cast<uint32_t>(packet->size()), static_cast<uint16_t>(packet->getOpcode()), packet->size() ? packet->contents() : nullptr, 1, (m_session ? m_session->GetAccountId() : 0), m_protocol.expansion);

    if (!writeRc4Packet(rawOpcode, packet->size() ? packet->contents() : nullptr, static_cast<uint32_t>(packet->size())))
    {
        sLogger.failure("WorldSocket::{}: cannot send {} to {}:{}", Profile::Name, Version::opcodeNameForId(packet->getOpcode(), m_protocol.expansion), getRemoteIp(), getRemotePort());
        m_rc4WorldState = Rc4WorldState::Disabled;
        disconnect();
    }

    return true;
}

bool WorldSocket::writeRc4Packet(uint16_t rawOpcode, const uint8_t* payload, uint32_t payloadSize)
{
    burstBegin();

    const bool encrypted = m_crypt.isInitialized();

    std::vector<uint8_t> frame;
    frame.reserve(Rc4World::NormalHeaderSize + payloadSize + 16);

    uint16_t opcode = rawOpcode;
    std::vector<uint8_t> body;

    auto* stream = static_cast<z_stream*>(m_rc4CompressionStream.get());
    if (encrypted && payloadSize > Rc4World::MinSizeForCompression && stream != nullptr)
    {
        // uncompressed size with the opcode, checksum of opcode and payload, checksum of the deflate data
        const uint32_t uncompressedAdler = static_cast<uint32_t>(adler32(adler32(Rc4World::CompressionAdlerSeed, reinterpret_cast<const Bytef*>(&rawOpcode), sizeof(uint16_t)), payload, payloadSize));

        std::vector<uint8_t> compressed(deflateBound(stream, payloadSize + sizeof(uint16_t)));
        stream->next_out = compressed.data();
        stream->avail_out = static_cast<uInt>(compressed.size());
        stream->next_in = reinterpret_cast<Bytef*>(&rawOpcode);
        stream->avail_in = sizeof(uint16_t);

        bool compressedOk = deflate(stream, Z_NO_FLUSH) == Z_OK;
        if (compressedOk)
        {
            stream->next_in = const_cast<Bytef*>(payload);
            stream->avail_in = payloadSize;
            compressedOk = deflate(stream, Z_SYNC_FLUSH) == Z_OK;
        }

        if (!compressedOk)
        {
            burstEnd();
            sLogger.failure("WorldSocket::{}: packet compression failed: {}", Profile::Name, stream->msg ? stream->msg : "unknown error");
            return false;
        }

        compressed.resize(compressed.size() - stream->avail_out);

        appendUInt32LE(body, payloadSize + static_cast<uint32_t>(sizeof(uint16_t)));
        appendUInt32LE(body, uncompressedAdler);
        appendUInt32LE(body, static_cast<uint32_t>(adler32(Rc4World::CompressionAdlerSeed, compressed.data(), static_cast<uInt>(compressed.size()))));
        body.insert(body.end(), compressed.begin(), compressed.end());

        opcode = Version::opcodeHexFor(SMSG_COMPRESSED_PACKET, m_protocol);
        payload = body.data();
        payloadSize = static_cast<uint32_t>(body.size());
    }

    // the size counts the opcode; the setup header is used until the encryption starts
    const uint32_t size = payloadSize + static_cast<uint32_t>(sizeof(uint16_t));
    if (encrypted || !Profile::SetupHeaderBeforeAuthSession)
    {
        appendUInt32LE(frame, size);
        appendUInt16LE(frame, opcode);
        if (encrypted)
            m_crypt.encryptWotlkSend(frame.data(), sizeof(uint32_t));
    }
    else
    {
        appendUInt16LE(frame, static_cast<uint16_t>(size));
        appendUInt16LE(frame, opcode);
    }

    if (payloadSize != 0)
        frame.insert(frame.end(), payload, payload + payloadSize);

    // keep the byte order of the stream cipher: queue behind frames that are still waiting
    bool sent = true;
    if (m_rc4PendingOutput.empty() && writeBuffer.GetSpace() >= frame.size())
        sent = burstSend(frame.data(), static_cast<uint32_t>(frame.size()));
    else
        m_rc4PendingOutput.insert(m_rc4PendingOutput.end(), frame.begin(), frame.end());

    if (sent)
        burstPush();

    burstEnd();

    if (!m_rc4PendingOutput.empty())
        flushRc4Output();

    return sent;
}

bool WorldSocket::flushRc4Output()
{
    burstBegin();

    if (m_rc4PendingOutput.empty())
    {
        burstEnd();
        return true;
    }

    const size_t amount = std::min(m_rc4PendingOutput.size(), writeBuffer.GetSpace());
    bool sent = amount == 0;
    if (amount != 0)
    {
        sent = burstSend(m_rc4PendingOutput.data(), static_cast<uint32_t>(amount));
        if (sent)
        {
            m_rc4PendingOutput.erase(m_rc4PendingOutput.begin(), m_rc4PendingOutput.begin() + static_cast<std::ptrdiff_t>(amount));
            burstPush();
        }
    }

    burstEnd();
    return sent;
}

#endif
