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
#include "world/Server/WorldConfig.h"
#include "world/Server/WorldSession.h"
#include "world/Version/VersionRegistry.hpp"

#if AE_WORLD_PROFILE_WOD
#include "version/WoD/World/WorldProfile.hpp"
#elif AE_WORLD_PROFILE_LEGION
#include "version/Legion/World/WorldProfile.hpp"
#endif

#include <openssl/crypto.h>
#include <openssl/rand.h>
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
    m_crypt.initForClientVersion(static_cast<uint8_t>(m_protocol.expansion), m_rc4SessionKey.data());
    completeRc4Authentication();
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
