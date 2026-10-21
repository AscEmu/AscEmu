/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "world/Server/WorldSocket.hpp"

#include "Cryptography/Ed25519.hpp"
#include "Cryptography/Sha256.hpp"
#include "Cryptography/Sha512.hpp"
#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "Utilities/Util.hpp"
#include "world/Objects/Units/Players/PlayerDefines.hpp"
#include "world/Server/LogonCommClient/LogonCommHandler.h"
#include "world/Server/Opcodes.hpp"
#include "world/Server/Packets/CmsgAuthSession.h"
#include "world/Server/Packets/SmsgAuthChallenge.h"
#include "world/Server/World.h"
#include "world/Server/WorldConfig.h"
#include "world/Server/WorldSession.h"
#include "world/Version/VersionRegistry.hpp"
#include "version/Shared/World/WorldConnectKey.hpp"

#if AE_WORLD_PROFILE_BFA
#include "version/BfA/World/WorldProfile.hpp"
#elif AE_WORLD_PROFILE_SHADOWLANDS
#include "version/Shadowlands/World/WorldProfile.hpp"
#elif AE_WORLD_PROFILE_DRAGONFLIGHT
#include "version/Dragonflight/World/WorldProfile.hpp"
#elif AE_WORLD_PROFILE_THEWARWITHIN
#include "version/TheWarWithin/World/WorldProfile.hpp"
#elif AE_WORLD_PROFILE_MIDNIGHT
#include "version/Midnight/World/WorldProfile.hpp"
#endif

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <zlib.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <memory>
#include <string_view>
#include <type_traits>
#include <vector>

using namespace AscEmu::Packets;

#if AE_WORLD_PROFILE_BFA || AE_WORLD_PROFILE_SHADOWLANDS || AE_WORLD_PROFILE_DRAGONFLIGHT || AE_WORLD_PROFILE_THEWARWITHIN || AE_WORLD_PROFILE_MIDNIGHT

namespace
{
    namespace Profile = AscEmu::Version::WorldProfile;

    namespace AesWorld
    {
        // uint32 size with the opcode, then the tag of the encrypted data
        constexpr uint32_t HeaderSize = sizeof(uint32_t) + AesGcmCrypt::TagSize;

        // encrypted packets above this size are sent compressed
        constexpr uint32_t MinSizeForCompression = 0x400;
        constexpr uLong CompressionAdlerSeed = 0x9827D8F1;

        // 11.x widened the transport: 32 bit opcodes, 32 byte challenges and seeds, SHA512 for the digests and the
        // session key, AES-256-GCM for the packets
        constexpr bool WideTransport = Profile::Expansion == WoW::Expansion::_TheWarWithin || Profile::Expansion == WoW::Expansion::_Midnight;
        constexpr size_t OpcodeSize = WideTransport ? sizeof(uint32_t) : sizeof(uint16_t);
        constexpr size_t ChallengeSize = WideTransport ? 32 : 16;
        constexpr size_t EncryptKeySize = WideTransport ? AesGcmCrypt::KeySize256 : AesGcmCrypt::KeySize;
        using Hash = std::conditional_t<WideTransport, Sha512Hash, Sha256Hash>;

        template <bool Wide>
        struct TransportSeeds;

        // 8.x to 10.x
        template <>
        struct TransportSeeds<false>
        {
            static constexpr std::array<uint8_t, 16> AuthCheckSeed = { 0xC5, 0xC6, 0x98, 0x95, 0x76, 0x3F, 0x1D, 0xCD, 0xB6, 0xA1, 0x37, 0x28, 0xB3, 0x12, 0xFF, 0x8A };
            static constexpr std::array<uint8_t, 16> SessionKeySeed = { 0x58, 0xCB, 0xCF, 0x40, 0xFE, 0x2E, 0xCE, 0xA6, 0x5A, 0x90, 0xB8, 0x01, 0x68, 0x6C, 0x28, 0x0B };
            static constexpr std::array<uint8_t, 16> ContinuedSessionSeed = { 0x16, 0xAD, 0x0C, 0xD4, 0x46, 0xF9, 0x4F, 0xB2, 0xEF, 0x7D, 0xEA, 0x2A, 0x17, 0x66, 0x4D, 0x2F };
            static constexpr std::array<uint8_t, 16> EncryptionKeySeed = { 0xE9, 0x75, 0x3C, 0x50, 0x90, 0x93, 0x61, 0xDA, 0x3B, 0x07, 0xEE, 0xFA, 0xFF, 0x9D, 0x41, 0xB8 };
            static constexpr std::array<uint8_t, 16> EnableEncryptionSeed = { 0x90, 0x9C, 0xD0, 0x50, 0x5A, 0x2C, 0x14, 0xDD, 0x5C, 0x2C, 0xC0, 0x64, 0x14, 0xF3, 0xFE, 0xC9 };
        };

        // 11.x
        template <>
        struct TransportSeeds<true>
        {
            static constexpr std::array<uint8_t, 32> AuthCheckSeed = { 0xDE, 0x3A, 0x2A, 0x8E, 0x6B, 0x89, 0x52, 0x66, 0x88, 0x9D, 0x7E, 0x7A, 0x77, 0x1D, 0x5D, 0x1F,
                0x4E, 0xD9, 0x0C, 0x23, 0x9B, 0xCD, 0x0E, 0xDC, 0xD2, 0xE8, 0x04, 0x3A, 0x68, 0x64, 0xC7, 0xB0 };
            static constexpr std::array<uint8_t, 32> SessionKeySeed = { 0xE8, 0x1E, 0x8B, 0x59, 0x27, 0x62, 0x1E, 0xAA, 0x86, 0x15, 0x18, 0xEA, 0xC0, 0xBF, 0x66, 0x8C,
                0x6D, 0xBF, 0x83, 0x93, 0xBC, 0xAA, 0x80, 0x52, 0x5B, 0x1E, 0xDC, 0x23, 0xA0, 0x12, 0xB7, 0x50 };
            static constexpr std::array<uint8_t, 32> ContinuedSessionSeed = { 0x56, 0x5C, 0x61, 0x9C, 0x48, 0x3A, 0x52, 0x1F, 0x61, 0x5D, 0x05, 0x49, 0xB2, 0x9A, 0x39, 0xBF,
                0x4B, 0x97, 0xB0, 0x1B, 0xF9, 0x6C, 0xDE, 0xD6, 0x80, 0x1D, 0xAB, 0x26, 0x02, 0xA9, 0x9B, 0x9D };
            static constexpr std::array<uint8_t, 32> EncryptionKeySeed = { 0x71, 0xC9, 0xED, 0x5A, 0xA7, 0x0E, 0x4D, 0xFF, 0x4C, 0x36, 0xA6, 0x5A, 0x3E, 0x46, 0x8A, 0x4A,
                0x5D, 0xA1, 0x48, 0xC8, 0x30, 0x47, 0x4A, 0xDE, 0xF6, 0x0D, 0x6C, 0xBE, 0x6F, 0xE4, 0x55, 0x73 };
            static constexpr std::array<uint8_t, 32> EnableEncryptionSeed = { 0x66, 0xBE, 0x29, 0x79, 0xEF, 0xF2, 0xD5, 0xB5, 0x61, 0x53, 0xF6, 0x5F, 0x45, 0xAE, 0x81, 0xCB,
                0x32, 0xEC, 0x94, 0xEC, 0x75, 0xB3, 0x5F, 0x44, 0x6A, 0x63, 0x43, 0x67, 0x17, 0x20, 0x44, 0x34 };
        };

        using Seeds = TransportSeeds<WideTransport>;
        constexpr size_t SeedSize = Seeds::AuthCheckSeed.size();

        constexpr size_t SignatureSize = 256;
        constexpr uint8_t ConnectionTypeInstance = 1;
        constexpr uint8_t AddressTypeIPv4 = 1;
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

    uint32_t readUInt32LE(const uint8_t* data)
    {
        return static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8) | (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[3]) << 24);
    }

    // the opcode on the wire: 16 bit up to 10.x, 32 bit from 11.x on
    void appendOpcode(std::vector<uint8_t>& out, uint32_t opcode)
    {
        if constexpr (AesWorld::OpcodeSize == sizeof(uint32_t))
            appendUInt32LE(out, opcode);
        else
            appendUInt16LE(out, static_cast<uint16_t>(opcode));
    }

    uint32_t readOpcode(const uint8_t* data)
    {
        if constexpr (AesWorld::OpcodeSize == sizeof(uint32_t))
            return readUInt32LE(data);
        else
            return static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8);
    }

    // the two challenges and a seed, the input of the digest and key derivations
    using ChallengeInput = std::array<uint8_t, AesWorld::ChallengeSize * 2 + AesWorld::SeedSize>;

    ChallengeInput challengeInput(const uint8_t* first, const uint8_t* second, const uint8_t* seed)
    {
        ChallengeInput input{};
        std::memcpy(input.data(), first, AesWorld::ChallengeSize);
        std::memcpy(input.data() + AesWorld::ChallengeSize, second, AesWorld::ChallengeSize);
        std::memcpy(input.data() + AesWorld::ChallengeSize * 2, seed, AesWorld::SeedSize);
        return input;
    }

    // 40 byte key stream: o0 = H(o1 | o0 | o2) with o1/o2 the hash of both halves of the seed (SHA256, 11.x: SHA512)
    void generateSessionKey(const uint8_t* seed, size_t seedSize, std::array<uint8_t, 40>& sessionKey)
    {
        using Hash = AesWorld::Hash;
        const size_t half = seedSize / 2;

        std::array<uint8_t, Hash::DigestLength> o0{};
        std::array<uint8_t, Hash::DigestLength> o1{};
        std::array<uint8_t, Hash::DigestLength> o2{};
        Hash::hash(seed, half, o1.data());
        Hash::hash(seed + half, seedSize - half, o2.data());

        size_t taken = o0.size();
        for (uint8_t& value : sessionKey)
        {
            if (taken == o0.size())
            {
                std::array<uint8_t, Hash::DigestLength * 3> input{};
                std::memcpy(input.data(), o1.data(), o1.size());
                std::memcpy(input.data() + o1.size(), o0.data(), o0.size());
                std::memcpy(input.data() + o1.size() + o0.size(), o2.data(), o2.size());
                Hash::hash(input.data(), input.size(), o0.data());
                taken = 0;
            }

            value = o0[taken++];
        }
    }

    // digest of CMSG_AUTH_SESSION: HMAC(local challenge | server challenge | check seed), first 24 bytes compared
    bool digestMatches(const std::array<uint8_t, AesWorld::Hash::DigestLength>& key, const uint8_t* localChallenge, const uint8_t* serverChallenge, const std::array<uint8_t, 24>& received)
    {
        const ChallengeInput input = challengeInput(localChallenge, serverChallenge, AesWorld::Seeds::AuthCheckSeed.data());

        std::array<uint8_t, AesWorld::Hash::DigestLength> digest{};
        AesWorld::Hash::hmac(key.data(), key.size(), input.data(), input.size(), digest.data());

        return CRYPTO_memcmp(digest.data(), received.data(), received.size()) == 0;
    }

    // key of the packet encryption: HMAC(session key, local challenge | server challenge | seed), the first 16 bytes
    // (11.x: 32 bytes)
    void deriveEncryptKey(const std::array<uint8_t, 40>& sessionKey, const uint8_t* localChallenge, const uint8_t* serverChallenge, std::array<uint8_t, 32>& encryptKey)
    {
        const ChallengeInput input = challengeInput(localChallenge, serverChallenge, AesWorld::Seeds::EncryptionKeySeed.data());

        std::array<uint8_t, AesWorld::Hash::DigestLength> digest{};
        AesWorld::Hash::hmac(sessionKey.data(), sessionKey.size(), input.data(), input.size(), digest.data());
        encryptKey.fill(0);
        std::memcpy(encryptKey.data(), digest.data(), AesWorld::EncryptKeySize);
    }

    void destroyCompressionStream(void* pointer)
    {
        auto* stream = static_cast<z_stream*>(pointer);
        if (stream == nullptr)
            return;

        deflateEnd(stream);
        delete stream;
    }

    struct EvpKeyDeleter
    {
        void operator()(EVP_PKEY* key) const { EVP_PKEY_free(key); }
    };

    EVP_PKEY* connectKey()
    {
        static const std::unique_ptr<EVP_PKEY, EvpKeyDeleter> key = []
        {
            BIO* keyBio = BIO_new_mem_buf(AscEmu::Version::WorldConnectKey::PrivateKey, -1);
            if (keyBio == nullptr)
                return std::unique_ptr<EVP_PKEY, EvpKeyDeleter>();

            EVP_PKEY* loaded = PEM_read_bio_PrivateKey(keyBio, nullptr, nullptr, nullptr);
            BIO_free(keyBio);
            return std::unique_ptr<EVP_PKEY, EvpKeyDeleter>(loaded);
        }();

        return key.get();
    }

    // PKCS#1 v1.5 signature with the SHA256 algorithm id over a digest that was computed by the caller, byte reversed
    bool signDigest(const std::array<uint8_t, Sha256Hash::DigestLength>& digest, uint8_t* output)
    {
        EVP_PKEY* key = connectKey();
        if (key == nullptr)
            return false;

        bool result = false;
        if (EVP_PKEY_CTX* context = EVP_PKEY_CTX_new(key, nullptr))
        {
            size_t outputSize = AesWorld::SignatureSize;
            result = EVP_PKEY_sign_init(context) > 0
                && EVP_PKEY_CTX_set_rsa_padding(context, RSA_PKCS1_PADDING) > 0
                && EVP_PKEY_CTX_set_signature_md(context, EVP_sha256()) > 0
                && EVP_PKEY_sign(context, output, &outputSize, digest.data(), digest.size()) > 0
                && outputSize == AesWorld::SignatureSize;

            EVP_PKEY_CTX_free(context);
        }

        // the client reads the signature as little endian number
        if (result)
            std::reverse(output, output + AesWorld::SignatureSize);

        return result;
    }

    // PKCS#1 v1.5 SHA256 signature of a message
    bool signMessage(const uint8_t* message, size_t messageSize, uint8_t* output)
    {
        std::array<uint8_t, Sha256Hash::DigestLength> digest{};
        Sha256Hash::hash(message, messageSize, digest.data());
        return signDigest(digest, output);
    }

    // 9.x: Ed25519 signature with a context (Ed25519ctx) over the digest, 64 bytes as they are
    constexpr size_t Ed25519SignatureSize = 64;

    bool signDigestEd25519(const uint8_t* digest, size_t digestSize, uint8_t* output)
    {
        using namespace AscEmu::Version::WorldConnectKey;

        static_assert(EnterEncryptedModeKey.size() == Ed25519::KeyLength && Ed25519::SignatureLength == Ed25519SignatureSize);
        return Ed25519::signWithContext(EnterEncryptedModeKey.data(), EnterEncryptedModeContext.data(), EnterEncryptedModeContext.size(), digest, digestSize, output);
    }
}

bool WorldSocket::initializeVersionedConnection()
{
    m_protocolSetByLogonComm = false;
    setClientProtocol(WoW::ClientProtocol{.expansion = Profile::Expansion});

    m_aesWorldState = AesWorldState::AwaitClientInitializer;
    m_aesHeaderReady = false;
    m_aesPacketSize = 0;
    m_aesPacketTag.fill(0);
    m_aesPendingOutput.clear();
    m_aesServerChallenge.fill(0);
    m_aesSessionKey.fill(0);
    m_aesEncryptKey.fill(0);
    m_aesGameAccountId = 0;
    m_aesGameAccountName.clear();
    m_aesInstanceConnection = false;
    m_aesConnectToKey = 0;

    sLogger.debug("WorldSocket::{}: connection from {}:{}", Profile::Name, getRemoteIp(), getRemotePort());

    if (!sendAesInitializer())
    {
        sLogger.failure("WorldSocket::{}: failed to send the connection initializer to {}:{}", Profile::Name, getRemoteIp(), getRemotePort());
        m_aesWorldState = AesWorldState::Disabled;
        disconnect();
    }

    return true;
}

bool WorldSocket::sendAesInitializer()
{
    burstBegin();
    const bool sent = burstSend(reinterpret_cast<const uint8_t*>(Profile::ServerInitializer.data()), static_cast<uint32_t>(Profile::ServerInitializer.size()));
    if (sent)
        burstPush();
    burstEnd();

    return sent;
}

bool WorldSocket::processVersionedRead()
{
    if (m_aesWorldState == AesWorldState::Disabled)
        return false;

    flushAesOutput();

    if (m_aesWorldState == AesWorldState::AwaitClientInitializer && !processAesInitializer())
        return true;

    while (m_aesWorldState != AesWorldState::Disabled && processAesPacket())
    {
    }

    return true;
}

bool WorldSocket::processAesInitializer()
{
    if (readBuffer.GetSize() < Profile::ClientInitializer.size())
        return false;

    std::array<uint8_t, Profile::ClientInitializer.size()> initializer{};
    if (!readBuffer.Read(initializer.data(), static_cast<uint32_t>(initializer.size())))
        return false;

    const std::string_view received(reinterpret_cast<const char*>(initializer.data()), initializer.size());
    if (received != Profile::ClientInitializer)
    {
        sLogger.failure("WorldSocket::{}: invalid connection initializer from {}:{}", Profile::Name, getRemoteIp(), getRemotePort());
        m_aesWorldState = AesWorldState::Disabled;
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
        m_aesWorldState = AesWorldState::Disabled;
        disconnect();
        return false;
    }
    m_aesCompressionStream = std::shared_ptr<void>(stream, destroyCompressionStream);

    // the server challenge and the dos challenge
    SmsgAuthChallenge challenge;
    m_aesServerChallenge.fill(0);
    if (RAND_bytes(m_aesServerChallenge.data(), static_cast<int>(AesWorld::ChallengeSize)) != 1
        || RAND_bytes(challenge.dosChallenge.data(), static_cast<int>(challenge.dosChallenge.size())) != 1)
    {
        sLogger.failure("WorldSocket::{}: RAND_bytes failed while creating the auth challenge", Profile::Name);
        m_aesWorldState = AesWorldState::Disabled;
        disconnect();
        return false;
    }
    challenge.challenge = m_aesServerChallenge;

    m_aesWorldState = AesWorldState::AwaitAuthSession;
    sendManagedPacket(challenge);
    return true;
}

bool WorldSocket::processAesPacket()
{
    if (!m_aesHeaderReady)
    {
        if (readBuffer.GetSize() < AesWorld::HeaderSize)
            return false;

        std::array<uint8_t, AesWorld::HeaderSize> header{};
        if (!readBuffer.Read(header.data(), static_cast<uint32_t>(header.size())))
            return false;

        const uint32_t size = readUInt32LE(header.data());
        std::memcpy(m_aesPacketTag.data(), header.data() + sizeof(uint32_t), m_aesPacketTag.size());

        // the size counts the opcode
        if (size < AesWorld::OpcodeSize || size >= Profile::MaxClientPacketSize)
        {
            sLogger.failure("WorldSocket::{}: {}:{} sent a malformed packet (size {})", Profile::Name, getRemoteIp(), getRemotePort(), size);
            m_aesWorldState = AesWorldState::Disabled;
            disconnect();
            return false;
        }

        m_aesPacketSize = size;
        m_aesHeaderReady = true;
    }

    if (readBuffer.GetSize() < m_aesPacketSize)
        return false;

    std::vector<uint8_t> data(m_aesPacketSize);
    readBuffer.Read(data.data(), m_aesPacketSize);
    m_aesHeaderReady = false;

    if (!m_aesCrypt.decryptReceive(data.data(), data.size(), m_aesPacketTag))
    {
        sLogger.failure("WorldSocket::{}: {}:{} sent a packet that cannot be decrypted (size {})", Profile::Name, getRemoteIp(), getRemotePort(), m_aesPacketSize);
        m_aesWorldState = AesWorldState::Disabled;
        disconnect();
        return false;
    }

    const uint32_t rawOpcode = readOpcode(data.data());
    const uint32_t payloadSize = m_aesPacketSize - static_cast<uint32_t>(AesWorld::OpcodeSize);

    auto packet = std::make_unique<WorldPacket>(rawOpcode, payloadSize);
    packet->resize(payloadSize);
    if (payloadSize != 0)
        std::memcpy(packet->contents(), data.data() + AesWorld::OpcodeSize, payloadSize);

    const uint32_t opcode = Version::opcodeIdForHex(rawOpcode, m_protocol);
    sWorldPacketLog.logPacket(payloadSize, static_cast<uint16_t>(opcode), payloadSize ? packet->contents() : nullptr, 0, (m_session ? m_session->GetAccountId() : 0), m_protocol.expansion);

    if (opcode == CMSG_AUTH_SESSION)
    {
        if (m_aesWorldState != AesWorldState::AwaitAuthSession)
        {
            sLogger.failure("WorldSocket::{}: {}:{} sent a second CMSG_AUTH_SESSION", Profile::Name, getRemoteIp(), getRemotePort());
            m_aesWorldState = AesWorldState::Disabled;
            disconnect();
            return false;
        }

        handleAesAuthSession(*packet);
        return m_aesWorldState != AesWorldState::Disabled;
    }

    if (opcode == CMSG_AUTH_CONTINUED_SESSION)
    {
        if (m_aesWorldState != AesWorldState::AwaitAuthSession)
        {
            sLogger.failure("WorldSocket::{}: {}:{} sent an unexpected CMSG_AUTH_CONTINUED_SESSION", Profile::Name, getRemoteIp(), getRemotePort());
            m_aesWorldState = AesWorldState::Disabled;
            disconnect();
            return false;
        }

        handleAesAuthContinuedSession(*packet);
        return m_aesWorldState != AesWorldState::Disabled;
    }

    if (opcode == CMSG_ENABLE_ENCRYPTION_ACK)
    {
        if (m_aesWorldState != AesWorldState::AwaitEncryptionAck)
        {
            sLogger.failure("WorldSocket::{}: {}:{} sent an unexpected CMSG_ENTER_ENCRYPTED_MODE_ACK", Profile::Name, getRemoteIp(), getRemotePort());
            m_aesWorldState = AesWorldState::Disabled;
            disconnect();
            return false;
        }

        handleAesEnterEncryptedModeAck();
        return m_aesWorldState != AesWorldState::Disabled;
    }

    dispatchPacket(std::move(packet));
    return true;
}

void WorldSocket::handleAesAuthSession(WorldPacket& packet)
{
    CmsgAuthSession authSession;
    if (!parsePacket(packet, authSession))
    {
        sLogger.failure("WorldSocket::{}: malformed CMSG_AUTH_SESSION from {}:{}: {}", Profile::Name, getRemoteIp(), getRemotePort(), authSession.errorMsg);
        m_aesWorldState = AesWorldState::Disabled;
        disconnect();
        return;
    }

    m_latency = Util::getMSTime() - m_latency;

    // the logonserver announced the Battle.net session with its realm join ticket over LogonComm before the client connected;
    // it is consumed only after the digest matched
    PendingWorldSession pending;
    if (!sLogonCommHandler.getPendingSession(authSession.realmJoinTicket, pending, false))
    {
        sLogger.failure("WorldSocket::{}: realm join ticket from {}:{} has no pending Battle.net session", Profile::Name, getRemoteIp(), getRemotePort());
        m_aesWorldState = AesWorldState::Disabled;
        disconnect();
        return;
    }

    // the seed depends on the client platform, the session does not carry it
    bool verified = false;
    for (const auto& seed : Profile::AuthSeeds)
    {
        std::array<uint8_t, 64 + 16> input{};
        std::memcpy(input.data(), pending.worldAuthKeyData.data(), 64);
        std::memcpy(input.data() + 64, seed.data(), seed.size());

        std::array<uint8_t, AesWorld::Hash::DigestLength> digestKey{};
        AesWorld::Hash::hash(input.data(), input.size(), digestKey.data());

        if (digestMatches(digestKey, authSession.localChallenge.data(), m_aesServerChallenge.data(), authSession.sessionDigest))
        {
            verified = true;
            break;
        }
    }

    if (!verified)
    {
        sLogger.failure("WorldSocket::{}: digest mismatch for game account {} ('{}') from {}:{}", Profile::Name, pending.gameAccountId, pending.gameAccountName, getRemoteIp(), getRemotePort());
        m_aesWorldState = AesWorldState::Disabled;
        disconnect();
        return;
    }

    PendingWorldSession consumed;
    if (!sLogonCommHandler.getPendingSession(authSession.realmJoinTicket, consumed, true))
    {
        sLogger.failure("WorldSocket::{}: pending session of game account {} expired during authentication", Profile::Name, pending.gameAccountId);
        m_aesWorldState = AesWorldState::Disabled;
        disconnect();
        return;
    }

    // world session key: key stream seeded with HMAC(server challenge | local challenge | seed), keyed by the hash of the key data
    std::array<uint8_t, AesWorld::Hash::DigestLength> keyDataHash{};
    AesWorld::Hash::hash(pending.worldAuthKeyData.data(), pending.worldAuthKeyData.size(), keyDataHash.data());

    const ChallengeInput sessionInput = challengeInput(m_aesServerChallenge.data(), authSession.localChallenge.data(), AesWorld::Seeds::SessionKeySeed.data());

    std::array<uint8_t, AesWorld::Hash::DigestLength> sessionSeed{};
    AesWorld::Hash::hmac(keyDataHash.data(), keyDataHash.size(), sessionInput.data(), sessionInput.size(), sessionSeed.data());
    generateSessionKey(sessionSeed.data(), sessionSeed.size(), m_aesSessionKey);

    deriveEncryptKey(m_aesSessionKey, authSession.localChallenge.data(), m_aesServerChallenge.data(), m_aesEncryptKey);

    auto protocol = getClientProtocol();
    protocol.regionId = authSession.regionId;
    protocol.battlegroupId = authSession.battlegroupId;
    protocol.realmId = authSession.realmId;
    setClientProtocol(protocol);

    m_accountName = pending.gameAccountName;
    m_clientBuild = pending.clientBuild;
    m_aesGameAccountId = pending.gameAccountId;
    m_aesGameAccountName = pending.gameAccountName;
    m_aesSession = consumed;

    sLogger.debug("WorldSocket::{}: game account {} ('{}') authenticated from {}:{}", Profile::Name, pending.gameAccountId, pending.gameAccountName, getRemoteIp(), getRemotePort());

    if (!sendAesEnterEncryptedMode())
    {
        sLogger.failure("WorldSocket::{}: cannot sign SMSG_ENTER_ENCRYPTED_MODE for game account {}", Profile::Name, m_aesGameAccountId);
        m_aesWorldState = AesWorldState::Disabled;
        disconnect();
    }
}

bool WorldSocket::sendAesEnterEncryptedMode()
{
    // signature of HMAC(encrypt key, enabled | seed), then the enabled bit
    std::array<uint8_t, 1 + AesWorld::SeedSize> message{};
    message[0] = 1;
    std::memcpy(message.data() + 1, AesWorld::Seeds::EnableEncryptionSeed.data(), AesWorld::SeedSize);

    std::array<uint8_t, AesWorld::Hash::DigestLength> digest{};
    AesWorld::Hash::hmac(m_aesEncryptKey.data(), AesWorld::EncryptKeySize, message.data(), message.size(), digest.data());

    // 8.x: RSA signature, 9.x and later: Ed25519 signature with a context
    std::array<uint8_t, AesWorld::SignatureSize> signature{};
    size_t signatureSize = AesWorld::SignatureSize;
    if constexpr (Profile::Expansion == WoW::Expansion::_Shadowlands || Profile::Expansion == WoW::Expansion::_Dragonflight || Profile::Expansion == WoW::Expansion::_TheWarWithin || Profile::Expansion == WoW::Expansion::_Midnight)
    {
        signatureSize = Ed25519SignatureSize;
        if (!signDigestEd25519(digest.data(), digest.size(), signature.data()))
            return false;
    }
    else
    {
        static_assert(AesWorld::Hash::DigestLength == Sha256Hash::DigestLength || AesWorld::WideTransport);
        std::array<uint8_t, Sha256Hash::DigestLength> rsaDigest{};
        std::memcpy(rsaDigest.data(), digest.data(), rsaDigest.size());
        if (!signDigest(rsaDigest, signature.data()))
            return false;
    }

    // the client acknowledges before the encryption starts in both directions
    m_aesWorldState = AesWorldState::AwaitEncryptionAck;

    // 11.x: the region group in front of the signature
    WorldPacket enterEncryptedMode(SMSG_ENABLE_ENCRYPTION, static_cast<uint32_t>(signatureSize + 5));
    if constexpr (AesWorld::WideTransport)
        enterEncryptedMode << int32_t(0);
    enterEncryptedMode.append(signature.data(), signatureSize);
    enterEncryptedMode.writeBit(true);
    enterEncryptedMode.flushBits();
    sendPacket(&enterEncryptedMode);
    return true;
}

void WorldSocket::handleAesEnterEncryptedModeAck()
{
    if (!m_aesCrypt.init(m_aesEncryptKey.data(), AesWorld::EncryptKeySize))
    {
        sLogger.failure("WorldSocket::{}: cannot initialize the packet encryption for game account {}", Profile::Name, m_aesGameAccountId);
        m_aesWorldState = AesWorldState::Disabled;
        disconnect();
        return;
    }

    if (m_aesInstanceConnection)
    {
        attachAesInstanceConnection();
        return;
    }

    completeAesAuthentication();
}

void WorldSocket::completeAesAuthentication()
{
    m_aesWorldState = AesWorldState::Authenticated;

    // the account data the logonserver announced with the session; permissions of the realm (account_permissions) take precedence like on the classic path
    std::string gmFlags = sLogonCommHandler.getPermissionStringForAccountId(m_aesGameAccountId);
    if (gmFlags.empty())
        gmFlags = m_aesSession.gmFlags;

    completeAuthentication(m_aesGameAccountId, m_aesGameAccountName, gmFlags, m_aesSession.accountFlags, m_aesSession.forcedLanguage.empty() ? std::string("enUS") : m_aesSession.forcedLanguage, m_aesSession.muted);
}

void WorldSocket::attachAesInstanceConnection()
{
    // the connection joins the session that asked for it
    WorldSession* session = sWorld.getSessionByAccountId(m_aesGameAccountId);
    if (session == nullptr || session->getInstanceConnectKey() != m_aesConnectToKey)
    {
        sLogger.failure("WorldSocket::{}: the session of game account {} is gone, second connection closed", Profile::Name, m_aesGameAccountId);
        m_aesWorldState = AesWorldState::Disabled;
        disconnect();
        return;
    }

    m_aesWorldState = AesWorldState::Authenticated;
    m_session = session;
    session->setInstanceSocket(this);

    // the session continues the login in its own update
    session->QueuePacket(std::make_unique<WorldPacket>(Version::opcodeHexFor(CMSG_AUTH_CONTINUED_SESSION, m_protocol), 0));
}

void WorldSocket::handleAesAuthContinuedSession(WorldPacket& packet)
{
    const auto reject = [this](const char* reason)
    {
        sLogger.failure("WorldSocket::{}: continued session from {}:{} rejected: {}", Profile::Name, getRemoteIp(), getRemotePort(), reason);
        m_aesWorldState = AesWorldState::Disabled;
        disconnect();
    };

    // dos response, key, local challenge, digest; 11.x: dos response, local challenge, digest, key, native realm
    // address, third key
    uint64_t dosResponse = 0;
    uint64_t key = 0;
    std::array<uint8_t, AesWorld::ChallengeSize> localChallenge{};
    std::array<uint8_t, 24> digest{};

    constexpr size_t minimumSize = sizeof(dosResponse) + sizeof(key) + localChallenge.size() + digest.size() + (AesWorld::WideTransport ? 8 : 0);
    if (packet.size() < minimumSize)
    {
        reject("malformed CMSG_AUTH_CONTINUED_SESSION");
        return;
    }

    if constexpr (AesWorld::WideTransport)
    {
        packet >> dosResponse;
        packet.read(localChallenge.data(), localChallenge.size());
        packet.read(digest.data(), digest.size());
        packet >> key;
        packet.read<uint32_t>();                        // native realm address
        packet.read<uint32_t>();                        // third key
    }
    else
    {
        packet >> dosResponse >> key;
        packet.read(localChallenge.data(), localChallenge.size());
        packet.read(digest.data(), digest.size());
    }

    // key: account (32 bit), connection type (1 bit), random part (31 bit)
    const uint32_t accountId = static_cast<uint32_t>(key & 0xFFFFFFFF);
    const uint8_t connectionType = static_cast<uint8_t>((key >> 32) & 1);
    if (connectionType != AesWorld::ConnectionTypeInstance)
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

    // HMAC keyed by the session key over key, local challenge, server challenge and the seed
    std::array<uint8_t, 8 + AesWorld::ChallengeSize * 2 + AesWorld::SeedSize> input{};
    std::memcpy(input.data(), &key, 8);
    std::memcpy(input.data() + 8, localChallenge.data(), AesWorld::ChallengeSize);
    std::memcpy(input.data() + 8 + AesWorld::ChallengeSize, m_aesServerChallenge.data(), AesWorld::ChallengeSize);
    std::memcpy(input.data() + 8 + AesWorld::ChallengeSize * 2, AesWorld::Seeds::ContinuedSessionSeed.data(), AesWorld::SeedSize);

    std::array<uint8_t, AesWorld::Hash::DigestLength> expected{};
    AesWorld::Hash::hmac(realmSocket->m_aesSessionKey.data(), realmSocket->m_aesSessionKey.size(), input.data(), input.size(), expected.data());

    if (CRYPTO_memcmp(expected.data(), digest.data(), digest.size()) != 0)
    {
        reject("digest mismatch");
        return;
    }

    setClientProtocol(realmSocket->getClientProtocol());
    m_aesSessionKey = realmSocket->m_aesSessionKey;
    m_aesGameAccountId = accountId;
    m_aesGameAccountName = realmSocket->m_aesGameAccountName;
    m_aesInstanceConnection = true;
    m_aesConnectToKey = key;

    // the key of this connection comes from its own challenge
    deriveEncryptKey(m_aesSessionKey, localChallenge.data(), m_aesServerChallenge.data(), m_aesEncryptKey);

    sLogger.debug("WorldSocket::{}: second connection of game account {} authenticated from {}:{}", Profile::Name, accountId, getRemoteIp(), getRemotePort());

    if (!sendAesEnterEncryptedMode())
        reject("cannot sign SMSG_ENTER_ENCRYPTED_MODE");
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

    const uint16_t port = static_cast<uint16_t>(worldConfig.listen.listenPort);

    if constexpr (Profile::Expansion == WoW::Expansion::_Midnight)
    {
        // 12.x: no signature; count, serial, connection type, key, native realm address, third key, then every
        // payload with address type, address, port and a bleep token (token, proxy id and address strings with
        // their bit lengths, then the token lifespan in nanoseconds)
        WorldPacket connectTo(SMSG_CONNECT_TO, 4 + 4 + 1 + 8 + 4 + 4 + 1 + 4 + 2 + 5 + 8);
        connectTo << uint32_t(1);                       // payloads
        connectTo << uint32_t(serial);
        connectTo << uint8_t(AesWorld::ConnectionTypeInstance);
        connectTo << uint64_t(key);
        connectTo << uint32_t(m_protocol.getVirtualRealmAddress());
        connectTo << uint32_t(0);                       // third key
        connectTo << uint8_t(AesWorld::AddressTypeIPv4);
        connectTo.append(reinterpret_cast<const uint8_t*>(&local.sin_addr), 4);
        connectTo << uint16_t(port);
        connectTo.writeBits(0, 5);                      // token length
        connectTo.writeBits(0, 24);                     // proxy id length
        connectTo.writeBits(0, 6);                      // address length
        connectTo.flushBits();
        connectTo << int64_t(0);                        // token lifespan
        sendPacket(&connectTo);

        sLogger.debug("WorldSocket::{}: sent SMSG_CONNECT_TO (serial {}) to game account {}", Profile::Name, serial, m_aesGameAccountId);
        return true;
    }
    else
    {
        // where: type and address; signed together with the type as uint32 and the port
        std::vector<uint8_t> where;
        where.push_back(AesWorld::AddressTypeIPv4);
        where.insert(where.end(), reinterpret_cast<const uint8_t*>(&local.sin_addr), reinterpret_cast<const uint8_t*>(&local.sin_addr) + 4);

        std::vector<uint8_t> signed_;
        signed_.insert(signed_.end(), where.begin(), where.end());
        appendUInt32LE(signed_, AesWorld::AddressTypeIPv4);
        appendUInt16LE(signed_, port);

        std::array<uint8_t, AesWorld::SignatureSize> signature{};
        if (!signMessage(signed_.data(), signed_.size(), signature.data()))
        {
            sLogger.failure("WorldSocket::{}: cannot sign SMSG_CONNECT_TO", Profile::Name);
            return false;
        }

        // signature, where, port, serial, connection type, key; 11.x: the native realm address and the third key
        WorldPacket connectTo(SMSG_CONNECT_TO, AesWorld::SignatureSize + where.size() + 2 + 4 + 1 + 8 + 8);
        connectTo.append(signature.data(), signature.size());
        connectTo.append(where.data(), where.size());
        connectTo << uint16_t(port);
        connectTo << uint32_t(serial);
        connectTo << uint8_t(AesWorld::ConnectionTypeInstance);
        connectTo << uint64_t(key);
        if constexpr (AesWorld::WideTransport)
        {
            connectTo << uint32_t(m_protocol.getVirtualRealmAddress());
            connectTo << uint32_t(0);                       // third key
        }
        sendPacket(&connectTo);

        sLogger.debug("WorldSocket::{}: sent SMSG_CONNECT_TO (serial {}) to game account {}", Profile::Name, serial, m_aesGameAccountId);
        return true;
    }
}

bool WorldSocket::sendVersionedPacket(WorldPacket* packet)
{
    if (m_aesWorldState == AesWorldState::Disabled)
        return false;

    if (packet == nullptr)
        return true;

    const uint32_t rawOpcode = Version::opcodeHexFor(packet->getOpcode(), m_protocol);
    if (rawOpcode == 0)
    {
        sLogger.debug("WorldSocket::{}: dropped {} without an opcode of this client ({} bytes)", Profile::Name, Version::opcodeNameForId(packet->getOpcode(), m_protocol.expansion), packet->size());
        return true;
    }

    sWorldPacketLog.logPacket(static_cast<uint32_t>(packet->size()), static_cast<uint16_t>(packet->getOpcode()), packet->size() ? packet->contents() : nullptr, 1, (m_session ? m_session->GetAccountId() : 0), m_protocol.expansion);

    if (!writeAesPacket(rawOpcode, packet->size() ? packet->contents() : nullptr, static_cast<uint32_t>(packet->size())))
    {
        sLogger.failure("WorldSocket::{}: cannot send {} to {}:{}", Profile::Name, Version::opcodeNameForId(packet->getOpcode(), m_protocol.expansion), getRemoteIp(), getRemotePort());
        m_aesWorldState = AesWorldState::Disabled;
        disconnect();
    }

    return true;
}

bool WorldSocket::writeAesPacket(uint32_t rawOpcode, const uint8_t* payload, uint32_t payloadSize)
{
    burstBegin();

    const bool encrypted = m_aesCrypt.isInitialized();

    // opcode and payload, compressed when large enough
    std::vector<uint8_t> body;
    body.reserve(AesWorld::OpcodeSize + payloadSize + 16);

    auto* stream = static_cast<z_stream*>(m_aesCompressionStream.get());
    bool compressed = false;
    if (encrypted && payloadSize > AesWorld::MinSizeForCompression && stream != nullptr)
    {
        // uncompressed size with the opcode, checksum of opcode and payload, checksum of the deflate data; the
        // opcode bytes are the low bytes of the little endian value
        const uint32_t uncompressedAdler = static_cast<uint32_t>(adler32(adler32(AesWorld::CompressionAdlerSeed, reinterpret_cast<const Bytef*>(&rawOpcode), static_cast<uInt>(AesWorld::OpcodeSize)), payload, payloadSize));

        std::vector<uint8_t> deflated(deflateBound(stream, payloadSize + static_cast<uLong>(AesWorld::OpcodeSize)));
        stream->next_out = deflated.data();
        stream->avail_out = static_cast<uInt>(deflated.size());
        stream->next_in = reinterpret_cast<Bytef*>(&rawOpcode);
        stream->avail_in = static_cast<uInt>(AesWorld::OpcodeSize);

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
            return false;
        }

        deflated.resize(deflated.size() - stream->avail_out);

        const uint32_t compressedOpcode = Version::opcodeHexFor(SMSG_COMPRESSED_PACKET, m_protocol);
        appendOpcode(body, compressedOpcode);
        appendUInt32LE(body, payloadSize + static_cast<uint32_t>(AesWorld::OpcodeSize));
        appendUInt32LE(body, uncompressedAdler);
        appendUInt32LE(body, static_cast<uint32_t>(adler32(AesWorld::CompressionAdlerSeed, deflated.data(), static_cast<uInt>(deflated.size()))));
        body.insert(body.end(), deflated.begin(), deflated.end());
        compressed = true;
    }

    if (!compressed)
    {
        appendOpcode(body, rawOpcode);
        if (payloadSize != 0)
            body.insert(body.end(), payload, payload + payloadSize);
    }

    // header: size of the body, tag of the encryption; the counters run for unencrypted packets as well
    AesGcmCrypt::Tag tag{};
    if (!m_aesCrypt.encryptSend(body.data(), body.size(), tag))
    {
        burstEnd();
        return false;
    }

    std::vector<uint8_t> frame;
    frame.reserve(AesWorld::HeaderSize + body.size());
    appendUInt32LE(frame, static_cast<uint32_t>(body.size()));
    frame.insert(frame.end(), tag.begin(), tag.end());
    frame.insert(frame.end(), body.begin(), body.end());

    // keep the order of the packet counters: queue behind frames that are still waiting
    bool sent = true;
    if (m_aesPendingOutput.empty() && writeBuffer.GetSpace() >= frame.size())
        sent = burstSend(frame.data(), static_cast<uint32_t>(frame.size()));
    else
        m_aesPendingOutput.insert(m_aesPendingOutput.end(), frame.begin(), frame.end());

    if (sent)
        burstPush();

    burstEnd();

    if (!m_aesPendingOutput.empty())
        flushAesOutput();

    return sent;
}

bool WorldSocket::flushAesOutput()
{
    burstBegin();

    if (m_aesPendingOutput.empty())
    {
        burstEnd();
        return true;
    }

    const size_t amount = std::min(m_aesPendingOutput.size(), writeBuffer.GetSpace());
    bool sent = amount == 0;
    if (amount != 0)
    {
        sent = burstSend(m_aesPendingOutput.data(), static_cast<uint32_t>(amount));
        if (sent)
        {
            m_aesPendingOutput.erase(m_aesPendingOutput.begin(), m_aesPendingOutput.begin() + static_cast<std::ptrdiff_t>(amount));
            burstPush();
        }
    }

    burstEnd();
    return sent;
}

#endif
