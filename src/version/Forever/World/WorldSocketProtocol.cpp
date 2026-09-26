#include "Utilities/Util.hpp"
/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Forever WorldSocket adapter.
//
// This file deliberately owns the Forever World-V2 bootstrap/auth discovery.
// It has no dependency on another client profile. The current implementation
// sends the verified 69893 auth challenge, parses CMSG_AUTH_SESSION and logs
// the exact material required to verify the build-auth key before encryption
// is enabled.

#include "world/Server/WorldSocket.hpp"
#include "world/Server/OpcodeTable.hpp"
#include "world/Server/Opcodes.hpp"

#include "Logging/Logger.hpp"
#include "version/Forever/BuildProfile.hpp"
#include "version/Forever/Auth.hpp"
#include "version/Forever/World/PostAuthBootstrap.hpp"
#include "version/Forever/World/CharacterSelectBootstrap.hpp"
#include "world/Server/BattleNetCommClient/BattleNetCommClient.hpp"
#include "world/Server/World.h"
#include "world/Server/WorldSession.h"
#include "world/Server/DatabaseDefinition.hpp"


#include <openssl/crypto.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace
{
    namespace ForeverWorldV2
    {
        inline constexpr std::string_view ServerInitializer =
            "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2\n";
        inline constexpr std::string_view ClientInitializer =
            "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2\n";

        inline constexpr uint32_t AuthTagSize = 12;
        inline constexpr uint32_t ServerHeaderSize = 4 + AuthTagSize;
        inline constexpr uint32_t ClientHeaderSize = ServerHeaderSize + 4;
        inline constexpr uint32_t MaxPacketSize = 0x10000;
        inline constexpr uint32_t ClientIvMagic = 0x544E4C43; // "CLNT"
        inline constexpr uint32_t ServerIvMagic = 0x52565253; // "SRVR"
        inline constexpr int32_t EnterEncryptedModeRegionGroup = 8;
        inline constexpr uint32_t AuthChallengePayloadSize = 65;
        inline constexpr uint32_t AuthSessionFixedSize = 77;
    }

    uint32_t foreverWireOpcode(uint32_t internalOpcode)
    {
        return sOpcodeTables.getHexValueForExpansion(internalOpcode, WoW::Expansion::Forever);
    }

    bool isForeverMovementOpcode(uint32_t opcode)
    {
        switch (opcode)
        {
            case CMSG_MOVE_CHANGE_TRANSPORT:
            case MSG_MOVE_JUMP:
            case CMSG_MOVE_DOUBLE_JUMP:
            case MSG_MOVE_FALL_LAND:
            case CMSG_MOVE_FALL_RESET:
            case CMSG_MOVE_UPDATE_FALL_SPEED:
            case MSG_MOVE_HEARTBEAT:
            case CMSG_MOVE_SET_ADV_FLY:
            case MSG_MOVE_SET_WALK_MODE:
            case MSG_MOVE_SET_RUN_MODE:
            case CMSG_MOVE_SET_FLY:
            case MSG_MOVE_SET_PITCH:
            case MSG_MOVE_SET_FACING:
            case CMSG_MOVE_SET_FACING_HEARTBEAT:
            case MSG_MOVE_START_ASCEND:
            case MSG_MOVE_START_BACKWARD:
            case MSG_MOVE_START_DESCEND:
            case MSG_MOVE_START_FORWARD:
            case MSG_MOVE_START_PITCH_DOWN:
            case MSG_MOVE_START_PITCH_UP:
            case MSG_MOVE_START_STRAFE_LEFT:
            case MSG_MOVE_START_STRAFE_RIGHT:
            case MSG_MOVE_START_SWIM:
            case MSG_MOVE_START_TURN_LEFT:
            case MSG_MOVE_START_TURN_RIGHT:
            case MSG_MOVE_STOP:
            case MSG_MOVE_STOP_ASCEND:
            case MSG_MOVE_STOP_PITCH:
            case MSG_MOVE_STOP_STRAFE:
            case MSG_MOVE_STOP_SWIM:
            case MSG_MOVE_STOP_TURN:
                return true;
            default:
                return false;
        }
    }

    bool isForeverMovementWireOpcode(uint32_t rawOpcode)
    {
        return isForeverMovementOpcode(sOpcodeTables.getInternalIdForHex(rawOpcode, WoW::Expansion::Forever));
    }
    struct ForeverPendingInstanceLogin
    {
        WorldSession* session{nullptr};
        WorldSocket* realmSocket{nullptr};
        uint32_t guidLow{0};
        uint32_t clientBuild{0};
        std::array<uint8_t, 40> sessionKey{};
        uint32_t regionId{0};
        uint32_t battlegroupId{0};
        uint32_t realmId{0};
        uint32_t nativeRealmAddress{0};
        uint32_t key3{0};
    };

    std::mutex sForeverPendingInstanceLoginsLock;
    std::unordered_map<uint64_t, ForeverPendingInstanceLogin> sForeverPendingInstanceLogins;

    uint32_t readUInt32LE(const uint8_t* data)
    {
        uint32_t value = 0;
        std::memcpy(&value, data, sizeof(value));
        return value;
    }

    uint64_t readUInt64LE(const uint8_t* data)
    {
        uint64_t value = 0;
        std::memcpy(&value, data, sizeof(value));
        return value;
    }


    ByteBuffer buildForeverAccountDataTimes()
    {
        // Same wire model as the verified modern protocol implementation:
        // packed empty 128-bit ObjectGuid (16-bit zero presence mask),
        // int64 ServerTime, then twenty int64 account-data timestamps.
        ByteBuffer packet;
        packet << uint16_t(0); // packed empty ObjectGuid presence mask
        packet << int64_t(static_cast<int64_t>(std::time(nullptr)));
        for (uint32_t i = 0; i < 20U; ++i)
            packet << int64_t(0);
        return packet;
    }

    bool foreverSha512(const uint8_t* first, size_t firstSize, const uint8_t* second, size_t secondSize, std::array<uint8_t, 64>& digest)
    {
        EVP_MD_CTX* context = EVP_MD_CTX_new();
        if (context == nullptr)
            return false;

        bool success = EVP_DigestInit_ex(context, EVP_sha512(), nullptr) == 1;
        if (success && firstSize != 0)
            success = EVP_DigestUpdate(context, first, firstSize) == 1;
        if (success && secondSize != 0)
            success = EVP_DigestUpdate(context, second, secondSize) == 1;

        unsigned int digestSize = 0;
        if (success)
        {
            success = EVP_DigestFinal_ex(context, digest.data(), &digestSize) == 1 &&
                digestSize == digest.size();
        }

        EVP_MD_CTX_free(context);
        return success;
    }

    bool calculateForeverAuthDigest(const std::array<uint8_t, 64>& worldAuthKeyData, const std::array<uint8_t, 16>& buildAuthKey, const std::array<uint8_t, 32>& localChallenge, const std::array<uint8_t, 32>& serverChallenge, std::array<uint8_t, 64>& result)
    {
        std::array<uint8_t, 64> digestKeyHash{};
        if (!foreverSha512(worldAuthKeyData.data(), worldAuthKeyData.size(), buildAuthKey.data(), buildAuthKey.size(), digestKeyHash))
        {
            return false;
        }

        std::array<uint8_t, 96> hmacInput{};
        size_t offset = 0;

        std::memcpy(hmacInput.data() + offset, localChallenge.data(), localChallenge.size());
        offset += localChallenge.size();

        std::memcpy(hmacInput.data() + offset, serverChallenge.data(), serverChallenge.size());
        offset += serverChallenge.size();

        std::memcpy(hmacInput.data() + offset, AscEmu::Version::Forever::AuthCheckSeed.data(), AscEmu::Version::Forever::AuthCheckSeed.size());

        unsigned int digestSize = 0;
        const unsigned char* calculated = HMAC(EVP_sha512(), digestKeyHash.data(), static_cast<int>(digestKeyHash.size()), hmacInput.data(), hmacInput.size(), result.data(), &digestSize);

        return calculated != nullptr && digestSize == result.size();
    }

    bool foreverDigestMatches(const std::array<uint8_t, 64>& calculated, const std::array<uint8_t, 24>& received)
    {
        return CRYPTO_memcmp(calculated.data(), received.data(), received.size()) == 0;
    }

    bool foreverSha512Parts(const std::vector<std::pair<const uint8_t*, size_t>>& parts, std::array<uint8_t, 64>& digest)
    {
        EVP_MD_CTX* context = EVP_MD_CTX_new();
        if (context == nullptr)
            return false;

        bool success = EVP_DigestInit_ex(context, EVP_sha512(), nullptr) == 1;
        for (const auto& [data, size] : parts)
        {
            if (success && size > 0)
                success = EVP_DigestUpdate(context, data, size) == 1;
        }

        unsigned int digestSize = 0;
        if (success)
            success = EVP_DigestFinal_ex(context, digest.data(), &digestSize) == 1 && digestSize == digest.size();

        EVP_MD_CTX_free(context);
        return success;
    }

    bool foreverHmacSha512(const uint8_t* key, size_t keySize, const std::vector<std::pair<const uint8_t*, size_t>>& parts, std::array<uint8_t, 64>& digest)
    {
        size_t totalSize = 0;
        for (const auto& part : parts)
            totalSize += part.second;

        std::vector<uint8_t> input;
        input.reserve(totalSize);
        for (const auto& [data, size] : parts)
        {
            if (size > 0)
                input.insert(input.end(), data, data + size);
        }

        unsigned int digestSize = 0;
        const unsigned char* result = HMAC(EVP_sha512(), key, static_cast<int>(keySize), input.empty() ? nullptr : input.data(), input.size(), digest.data(), &digestSize);
        return result != nullptr && digestSize == digest.size();
    }

    bool deriveForeverEncryptionKeyFromSession(const std::array<uint8_t, 40>& sessionKey, const std::array<uint8_t, 32>& localChallenge, const std::array<uint8_t, 32>& serverChallenge, std::array<uint8_t, 32>& encryptionKey)
    {
        std::array<uint8_t, 64> digest{};
        if (!foreverHmacSha512(sessionKey.data(), sessionKey.size(), { { localChallenge.data(), localChallenge.size() }, { serverChallenge.data(), serverChallenge.size() }, { AscEmu::Version::Forever::EncryptionKeySeed.data(), AscEmu::Version::Forever::EncryptionKeySeed.size() } }, digest))
            return false;

        std::copy_n(digest.begin(), encryptionKey.size(), encryptionKey.begin());
        return true;
    }

    bool parseIpv4Address(std::string_view address, std::array<uint8_t, 4>& bytes)
    {
        unsigned int a = 0, b = 0, c = 0, d = 0;
        char tail = 0;
        const std::string text(address);
        if (std::sscanf(text.c_str(), "%u.%u.%u.%u%c", &a, &b, &c, &d, &tail) != 4 || a > 255 || b > 255 || c > 255 || d > 255)
            return false;
        bytes = { static_cast<uint8_t>(a), static_cast<uint8_t>(b), static_cast<uint8_t>(c), static_cast<uint8_t>(d) };
        return true;
    }

    bool generateForeverSessionKey(const std::array<uint8_t, 64>& seed, std::array<uint8_t, 40>& sessionKey)
    {
        // Blizzard's SessionKeyGenerator hashes both halves into o1/o2, starts o0
        // at zero, then emits SHA512(o1 || o0 || o2) blocks.
        std::array<uint8_t, 64> o0{};
        std::array<uint8_t, 64> o1{};
        std::array<uint8_t, 64> o2{};

        if (!foreverSha512Parts({ { seed.data(), seed.size() / 2 } }, o1) || !foreverSha512Parts({ { seed.data() + seed.size() / 2, seed.size() / 2 } }, o2))
        {
            return false;
        }

        std::array<uint8_t, 64> block{};
        if (!foreverSha512Parts({ { o1.data(), o1.size() }, { o0.data(), o0.size() }, { o2.data(), o2.size() } }, block))
            return false;

        memcpy(sessionKey.data(), block.data(), sessionKey.size());
        return true;
    }

    bool deriveForeverKeys(const std::array<uint8_t, 64>& worldAuthKeyData, const std::array<uint8_t, 32>& serverChallenge, const std::array<uint8_t, 32>& localChallenge, std::array<uint8_t, 40>& sessionKey, std::array<uint8_t, 32>& encryptionKey)
    {
        std::array<uint8_t, 64> keyDataDigest{};
        if (!foreverSha512Parts({ { worldAuthKeyData.data(), worldAuthKeyData.size() } }, keyDataDigest))
            return false;

        std::array<uint8_t, 64> sessionSeed{};
        if (!foreverHmacSha512(keyDataDigest.data(), keyDataDigest.size(), { { serverChallenge.data(), serverChallenge.size() }, { localChallenge.data(), localChallenge.size() }, { AscEmu::Version::Forever::SessionKeySeed.data(), AscEmu::Version::Forever::SessionKeySeed.size() } }, sessionSeed))
        {
            return false;
        }

        if (!generateForeverSessionKey(sessionSeed, sessionKey))
            return false;

        std::array<uint8_t, 64> encryptionDigest{};
        if (!foreverHmacSha512(sessionKey.data(), sessionKey.size(), { { localChallenge.data(), localChallenge.size() }, { serverChallenge.data(), serverChallenge.size() }, { AscEmu::Version::Forever::EncryptionKeySeed.data(), AscEmu::Version::Forever::EncryptionKeySeed.size() } }, encryptionDigest))
        {
            return false;
        }

        memcpy(encryptionKey.data(), encryptionDigest.data(), encryptionKey.size());
        return true;
    }

    struct Ed25519Point
    {
        BIGNUM* x = BN_new();
        BIGNUM* y = BN_new();
        BIGNUM* z = BN_new();
        BIGNUM* t = BN_new();

        Ed25519Point() = default;
        Ed25519Point(const Ed25519Point&) = delete;
        Ed25519Point& operator=(const Ed25519Point&) = delete;

        ~Ed25519Point()
        {
            BN_free(x);
            BN_free(y);
            BN_free(z);
            BN_free(t);
        }

        bool isValid() const
        {
            return x != nullptr && y != nullptr && z != nullptr && t != nullptr;
        }
    };

    bool copyEd25519Point(Ed25519Point& destination, const Ed25519Point& source)
    {
        return BN_copy(destination.x, source.x) != nullptr &&
            BN_copy(destination.y, source.y) != nullptr &&
            BN_copy(destination.z, source.z) != nullptr &&
            BN_copy(destination.t, source.t) != nullptr;
    }

    BIGNUM* bnFromLittleEndian(const uint8_t* data, size_t size)
    {
        std::vector<uint8_t> bigEndian(data, data + size);
        std::reverse(bigEndian.begin(), bigEndian.end());
        return BN_bin2bn(bigEndian.data(), static_cast<int>(bigEndian.size()), nullptr);
    }

    bool bnToLittleEndian32(const BIGNUM* value, uint8_t* output)
    {
        std::array<uint8_t, 32> bigEndian{};
        if (BN_bn2binpad(value, bigEndian.data(), static_cast<int>(bigEndian.size())) != static_cast<int>(bigEndian.size()))
            return false;

        std::reverse_copy(bigEndian.begin(), bigEndian.end(), output);
        return true;
    }

    bool ed25519PointAdd(const Ed25519Point& left, const Ed25519Point& right, Ed25519Point& result, const BIGNUM* prime, const BIGNUM* twoD, BN_CTX* context)
    {
        BN_CTX_start(context);
        BIGNUM* a = BN_CTX_get(context);
        BIGNUM* b = BN_CTX_get(context);
        BIGNUM* c = BN_CTX_get(context);
        BIGNUM* d = BN_CTX_get(context);
        BIGNUM* e = BN_CTX_get(context);
        BIGNUM* f = BN_CTX_get(context);
        BIGNUM* g = BN_CTX_get(context);
        BIGNUM* h = BN_CTX_get(context);
        BIGNUM* temp1 = BN_CTX_get(context);
        BIGNUM* temp2 = BN_CTX_get(context);

        bool success = temp2 != nullptr;
        if (success)
            success = BN_mod_sub(temp1, left.y, left.x, prime, context) == 1 &&
                BN_mod_sub(temp2, right.y, right.x, prime, context) == 1 &&
                BN_mod_mul(a, temp1, temp2, prime, context) == 1;
        if (success)
            success = BN_mod_add(temp1, left.y, left.x, prime, context) == 1 &&
                BN_mod_add(temp2, right.y, right.x, prime, context) == 1 &&
                BN_mod_mul(b, temp1, temp2, prime, context) == 1;
        if (success)
            success = BN_mod_mul(temp1, left.t, right.t, prime, context) == 1 &&
                BN_mod_mul(c, temp1, twoD, prime, context) == 1;
        if (success)
            success = BN_mod_mul(temp1, left.z, right.z, prime, context) == 1 &&
                BN_mod_add(d, temp1, temp1, prime, context) == 1;
        if (success)
            success = BN_mod_sub(e, b, a, prime, context) == 1 &&
                BN_mod_sub(f, d, c, prime, context) == 1 &&
                BN_mod_add(g, d, c, prime, context) == 1 &&
                BN_mod_add(h, b, a, prime, context) == 1;
        if (success)
            success = BN_mod_mul(temp1, e, f, prime, context) == 1 && BN_copy(result.x, temp1) != nullptr;
        if (success)
            success = BN_mod_mul(temp1, g, h, prime, context) == 1 && BN_copy(result.y, temp1) != nullptr;
        if (success)
            success = BN_mod_mul(temp1, e, h, prime, context) == 1 && BN_copy(result.t, temp1) != nullptr;
        if (success)
            success = BN_mod_mul(temp1, f, g, prime, context) == 1 && BN_copy(result.z, temp1) != nullptr;

        BN_CTX_end(context);
        return success;
    }

    bool ed25519PointDouble(const Ed25519Point& point, Ed25519Point& result, const BIGNUM* prime, BN_CTX* context)
    {
        BN_CTX_start(context);
        BIGNUM* a = BN_CTX_get(context);
        BIGNUM* b = BN_CTX_get(context);
        BIGNUM* c = BN_CTX_get(context);
        BIGNUM* d = BN_CTX_get(context);
        BIGNUM* e = BN_CTX_get(context);
        BIGNUM* f = BN_CTX_get(context);
        BIGNUM* g = BN_CTX_get(context);
        BIGNUM* h = BN_CTX_get(context);
        BIGNUM* temp = BN_CTX_get(context);
        BIGNUM* zero = BN_CTX_get(context);

        bool success = zero != nullptr;
        if (success)
        {
            BN_zero(zero);
            success = BN_mod_sqr(a, point.x, prime, context) == 1 &&
                BN_mod_sqr(b, point.y, prime, context) == 1 &&
                BN_mod_sqr(temp, point.z, prime, context) == 1 &&
                BN_mod_add(c, temp, temp, prime, context) == 1 &&
                BN_mod_sub(d, zero, a, prime, context) == 1;
        }
        if (success)
            success = BN_mod_add(temp, point.x, point.y, prime, context) == 1 &&
                BN_mod_sqr(e, temp, prime, context) == 1 &&
                BN_mod_sub(e, e, a, prime, context) == 1 &&
                BN_mod_sub(e, e, b, prime, context) == 1;
        if (success)
            success = BN_mod_add(g, d, b, prime, context) == 1 &&
                BN_mod_sub(f, g, c, prime, context) == 1 &&
                BN_mod_sub(h, d, b, prime, context) == 1;
        if (success)
            success = BN_mod_mul(temp, e, f, prime, context) == 1 && BN_copy(result.x, temp) != nullptr;
        if (success)
            success = BN_mod_mul(temp, g, h, prime, context) == 1 && BN_copy(result.y, temp) != nullptr;
        if (success)
            success = BN_mod_mul(temp, e, h, prime, context) == 1 && BN_copy(result.t, temp) != nullptr;
        if (success)
            success = BN_mod_mul(temp, f, g, prime, context) == 1 && BN_copy(result.z, temp) != nullptr;

        BN_CTX_end(context);
        return success;
    }

    bool ed25519ScalarMultiplyBase(const BIGNUM* scalar, Ed25519Point& result, const BIGNUM* prime, const BIGNUM* twoD, BN_CTX* context)
    {
        Ed25519Point accumulator;
        Ed25519Point addend;
        Ed25519Point temporary;
        if (!accumulator.isValid() || !addend.isValid() || !temporary.isValid())
            return false;

        BN_zero(accumulator.x);
        BN_zero(accumulator.t);
        if (BN_one(accumulator.y) != 1 || BN_one(accumulator.z) != 1)
            return false;

        if (BN_dec2bn(&addend.x, "15112221349535400772501151409588531511454012693041857206046113283949847762202") == 0 || BN_dec2bn(&addend.y, "46316835694926478169428394003475163141307993866256225615783033603165251855960") == 0 || BN_one(addend.z) != 1 || BN_mod_mul(addend.t, addend.x, addend.y, prime, context) != 1)
        {
            return false;
        }

        for (int bit = 0; bit < 256; ++bit)
        {
            if (BN_is_bit_set(scalar, bit) != 0)
            {
                if (!ed25519PointAdd(accumulator, addend, temporary, prime, twoD, context) || !copyEd25519Point(accumulator, temporary))
                {
                    return false;
                }
            }

            if (!ed25519PointDouble(addend, temporary, prime, context) || !copyEd25519Point(addend, temporary))
            {
                return false;
            }
        }

        return copyEd25519Point(result, accumulator);
    }

    bool ed25519EncodePoint(const Ed25519Point& point, const BIGNUM* prime, BN_CTX* context, std::array<uint8_t, 32>& encoded)
    {
        BN_CTX_start(context);
        BIGNUM* inverseZ = BN_CTX_get(context);
        BIGNUM* x = BN_CTX_get(context);
        BIGNUM* y = BN_CTX_get(context);

        bool success = y != nullptr;
        if (success)
            success = BN_mod_inverse(inverseZ, point.z, prime, context) != nullptr &&
                BN_mod_mul(x, point.x, inverseZ, prime, context) == 1 &&
                BN_mod_mul(y, point.y, inverseZ, prime, context) == 1 &&
                bnToLittleEndian32(y, encoded.data());
        if (success && BN_is_odd(x) != 0)
            encoded.back() |= 0x80U;

        BN_CTX_end(context);
        return success;
    }

    bool ed25519ctxSign(const std::array<uint8_t, 32>& privateSeed, const uint8_t* message, size_t messageSize, const uint8_t* ed25519Context, size_t contextSize, std::array<uint8_t, 64>& signature)
    {
        if (contextSize == 0 || contextSize > 255)
            return false;

        BN_CTX* bnContext = BN_CTX_new();
        BIGNUM* prime = BN_new();
        BIGNUM* order = BN_new();
        BIGNUM* d = BN_new();
        BIGNUM* twoD = BN_new();
        BIGNUM* denominator = BN_new();
        BIGNUM* denominatorInverse = BN_new();
        BIGNUM* scalarA = nullptr;
        BIGNUM* scalarR = nullptr;
        BIGNUM* scalarK = nullptr;
        BIGNUM* scalarS = BN_new();

        bool success = bnContext != nullptr && prime != nullptr && order != nullptr && d != nullptr &&
            twoD != nullptr && denominator != nullptr && denominatorInverse != nullptr && scalarS != nullptr;

        if (success)
        {
            success = BN_set_bit(prime, 255) == 1 && BN_sub_word(prime, 19) == 1 &&
                BN_set_bit(order, 252) == 1;
        }

        BIGNUM* orderTail = nullptr;
        if (success)
        {
            success = BN_dec2bn(&orderTail, "27742317777372353535851937790883648493") != 0 &&
                BN_add(order, order, orderTail) == 1;
        }

        // d = -121665 / 121666 mod p
        if (success)
        {
            success = BN_set_word(denominator, 121666) == 1 &&
                BN_mod_inverse(denominatorInverse, denominator, prime, bnContext) != nullptr &&
                BN_set_word(d, 121665) == 1 &&
                BN_mod_mul(d, d, denominatorInverse, prime, bnContext) == 1 &&
                BN_sub(d, prime, d) == 1 &&
                BN_mod_add(twoD, d, d, prime, bnContext) == 1;
        }

        std::array<uint8_t, 64> privateDigest{};
        if (success)
            success = foreverSha512Parts({ { privateSeed.data(), privateSeed.size() } }, privateDigest);

        std::array<uint8_t, 32> scalarABytes{};
        if (success)
        {
            memcpy(scalarABytes.data(), privateDigest.data(), scalarABytes.size());
            scalarABytes[0] &= 248U;
            scalarABytes[31] &= 63U;
            scalarABytes[31] |= 64U;
            scalarA = bnFromLittleEndian(scalarABytes.data(), scalarABytes.size());
            success = scalarA != nullptr;
        }

        Ed25519Point publicPoint;
        std::array<uint8_t, 32> publicKey{};
        if (success)
            success = publicPoint.isValid() &&
                ed25519ScalarMultiplyBase(scalarA, publicPoint, prime, twoD, bnContext) &&
                ed25519EncodePoint(publicPoint, prime, bnContext, publicKey);

        constexpr std::array<uint8_t, 32> dom2Prefix =
        {
            'S','i','g','E','d','2','5','5','1','9',' ','n','o',' ','E','d',
            '2','5','5','1','9',' ','c','o','l','l','i','s','i','o','n','s'
        };
        const uint8_t dom2Flag = 0;
        const uint8_t contextLength = static_cast<uint8_t>(contextSize);

        std::array<uint8_t, 64> rDigest{};
        if (success)
        {
            success = foreverSha512Parts({ { dom2Prefix.data(), dom2Prefix.size() }, { &dom2Flag, sizeof(dom2Flag) }, { &contextLength, sizeof(contextLength) }, { ed25519Context, contextSize }, { privateDigest.data() + 32, 32 }, { message, messageSize } }, rDigest);
        }

        if (success)
        {
            scalarR = bnFromLittleEndian(rDigest.data(), rDigest.size());
            success = scalarR != nullptr && BN_mod(scalarR, scalarR, order, bnContext) == 1;
        }

        Ed25519Point rPoint;
        std::array<uint8_t, 32> encodedR{};
        if (success)
            success = rPoint.isValid() &&
                ed25519ScalarMultiplyBase(scalarR, rPoint, prime, twoD, bnContext) &&
                ed25519EncodePoint(rPoint, prime, bnContext, encodedR);

        std::array<uint8_t, 64> kDigest{};
        if (success)
        {
            success = foreverSha512Parts({ { dom2Prefix.data(), dom2Prefix.size() }, { &dom2Flag, sizeof(dom2Flag) }, { &contextLength, sizeof(contextLength) }, { ed25519Context, contextSize }, { encodedR.data(), encodedR.size() }, { publicKey.data(), publicKey.size() }, { message, messageSize } }, kDigest);
        }

        if (success)
        {
            scalarK = bnFromLittleEndian(kDigest.data(), kDigest.size());
            success = scalarK != nullptr && BN_mod(scalarK, scalarK, order, bnContext) == 1 &&
                BN_mod_mul(scalarS, scalarK, scalarA, order, bnContext) == 1 &&
                BN_mod_add(scalarS, scalarS, scalarR, order, bnContext) == 1;
        }

        if (success)
        {
            memcpy(signature.data(), encodedR.data(), encodedR.size());
            success = bnToLittleEndian32(scalarS, signature.data() + encodedR.size());
        }

        BN_clear_free(scalarA);
        BN_clear_free(scalarR);
        BN_clear_free(scalarK);
        BN_free(orderTail);
        BN_free(scalarS);
        BN_free(denominatorInverse);
        BN_free(denominator);
        BN_free(twoD);
        BN_free(d);
        BN_free(order);
        BN_free(prime);
        BN_CTX_free(bnContext);
        return success;
    }

    std::string foreverBytesToHex(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size == 0)
            return "<empty>";

        std::ostringstream out;
        out << std::hex << std::uppercase << std::setfill('0');

        for (size_t i = 0; i < size; ++i)
        {
            if (i != 0)
                out << ' ';
            out << std::setw(2) << static_cast<unsigned>(data[i]);
        }

        return out.str();
    }

    bool createForeverEnterEncryptedModeSignature(const std::array<uint8_t, 32>& encryptionKey, bool enabled, std::array<uint8_t, 64>& signature)
    {
        const uint8_t enabledByte = enabled ? 1U : 0U;
        std::array<uint8_t, 64> toSign{};
        if (!foreverHmacSha512(encryptionKey.data(), encryptionKey.size(), { { &enabledByte, sizeof(enabledByte) }, { AscEmu::Version::Forever::EnableEncryptionSeed.data(), AscEmu::Version::Forever::EnableEncryptionSeed.size() } }, toSign))
        {
            return false;
        }

        // AscEmu currently ships OpenSSL 3.0, whose Ed25519 EVP implementation
        // only supports plain Ed25519 and silently ignores the Ed25519ctx
        // parameters used by newer OpenSSL versions. Implement RFC 8032
        // Ed25519ctx explicitly so the WoW client verifies the signature with
        // its embedded EnterEncryptedMode public key.
        return ed25519ctxSign(AscEmu::Version::Forever::EnableEncryptionPrivateKey, toSign.data(), toSign.size(), AscEmu::Version::Forever::EnableEncryptionContext.data(), AscEmu::Version::Forever::EnableEncryptionContext.size(), signature);
    }
}

bool WorldSocket::initializeVersionedConnection()
{
    m_protocolSetByLogonComm = false;

    WoW::ClientProtocol protocol;
    protocol.expansion = WoW::Expansion::Forever;
    protocol.realmId = worldConfig.battleNetComm.realmId;
    setClientProtocol(protocol);

    m_foreverClientBuild = AscEmu::Version::Forever::Build;
    m_foreverBattleNetAccountId = 0;
    m_foreverGameAccountId = 0;
    m_foreverGameAccountName.clear();
    m_foreverWorldState = ForeverWorldState::AwaitClientInitializer;
    m_foreverPacketOpcode = 0;
    m_foreverPacketRemaining = 0;
    m_foreverRecvCounter = 0;
    m_foreverSendCounter = 0;
    m_foreverCryptoSendCounter = 0;
    m_foreverCryptoRecvCounter = 0;
    m_foreverCharacterEnumRequests = 0;
    m_foreverRegionId = 0;
    m_foreverBattlegroupId = 0;
    m_foreverRealmId = 0;
    m_foreverHotfixBootstrapSent = false;
    m_foreverBufferedEnumRequest = false;
    m_foreverDbQueryBulkCount = 0;
    m_foreverPostDbEnumRefreshPending = false;
    m_foreverPostDbEnumRefreshSent = false;
    m_foreverPostCreateEnumRefreshPending = false;
    m_foreverPostCreateEnumRefreshArmed = false;
    m_foreverEncryptedHeaderReady = false;
    m_foreverEncryptedPacketSize = 0;
    m_foreverEncryptedPacketTag.fill(0);
    m_foreverEncryptedOpcode.fill(0);
    m_foreverSessionKey.fill(0);
    m_foreverEncryptionKey.fill(0);

    sLogger.info("WorldSocket::Forever: World V2 connection from {}:{}; build={}.", getRemoteIp(), getRemotePort(), m_foreverClientBuild);

    burstBegin();
    const bool sent = burstSend(reinterpret_cast<const uint8_t*>(ForeverWorldV2::ServerInitializer.data()), static_cast<uint32_t>(ForeverWorldV2::ServerInitializer.size()));
    if (sent)
        burstPush();
    burstEnd();

    if (!sent)
    {
        sLogger.failure("WorldSocket::Forever: failed to send World V2 server initializer to {}:{}.", getRemoteIp(), getRemotePort());
        disconnect();
    }

    return true;
}

bool WorldSocket::processVersionedRead()
{
    if (m_foreverWorldState == ForeverWorldState::Disabled)
        return false;

    if (m_foreverWorldState == ForeverWorldState::AwaitClientInitializer)
    {
        constexpr auto expected = ForeverWorldV2::ClientInitializer;
        if (readBuffer.GetSize() < expected.size())
            return true;

        std::array<char, 64> initializer{};
        static_assert(expected.size() < initializer.size());

        if (!readBuffer.Read(reinterpret_cast<uint8_t*>(initializer.data()), static_cast<uint32_t>(expected.size())))
        {
            return true;
        }

        const std::string_view received(initializer.data(), expected.size());
        if (received != expected)
        {
            sLogger.failure("WorldSocket::Forever: invalid World V2 client initializer from {}:{}.", getRemoteIp(), getRemotePort());
            disconnect();
            return true;
        }

        m_foreverWorldState = ForeverWorldState::AwaitAuthSession;

        if (!sendForeverAuthChallenge())
        {
            sLogger.failure("WorldSocket::Forever: failed to send SMSG_AUTH_CHALLENGE to {}:{}.", getRemoteIp(), getRemotePort());
            disconnect();
            return true;
        }

    }

    if (m_foreverWorldState == ForeverWorldState::AwaitAuthSession ||
        m_foreverWorldState == ForeverWorldState::AuthSessionObserved ||
        m_foreverWorldState == ForeverWorldState::AwaitEncryptionAck)
    {
        while (m_foreverWorldState != ForeverWorldState::Encrypted)
        {
            const size_t before = readBuffer.GetSize();
            if (!processForeverAuthPacket())
                break;

            if (readBuffer.GetSize() == before)
                break;
        }

        if (m_foreverWorldState != ForeverWorldState::Encrypted)
            return true;
    }

    if (m_foreverWorldState == ForeverWorldState::Encrypted)
    {
        while (processForeverEncryptedPacket())
        {
        }
    }

    return true;
}

bool WorldSocket::sendForeverWorldPacket(uint32_t opcode, const uint8_t* payload, uint32_t payloadSize)
{
    using namespace AscEmu::Version::Forever;

    if (!isConnected() || opcode == 0 || payloadSize > ForeverWorldV2::MaxPacketSize - sizeof(uint32_t))
    {
        return false;
    }

    const uint32_t packetSize = static_cast<uint32_t>(sizeof(uint32_t)) + payloadSize;

    std::array<uint8_t, ForeverWorldV2::ServerHeaderSize> header{};
    std::memcpy(header.data(), &packetSize, sizeof(packetSize));

    if (m_foreverWorldState == ForeverWorldState::Encrypted)
    {
        std::vector<uint8_t> encrypted(packetSize);
        std::memcpy(encrypted.data(), &opcode, sizeof(opcode));
        if (payloadSize != 0)
            std::memcpy(encrypted.data() + sizeof(opcode), payload, payloadSize);

        std::array<uint8_t, ForeverWorldV2::AuthTagSize> tag{};
        if (!encryptForeverPayload(encrypted, tag))
        {
            sLogger.failure("WorldSocket::Forever: AES-256-GCM encryption failed for opcode=0x{:08X}, counter={}.", opcode, m_foreverCryptoSendCounter);
            return false;
        }

        std::memcpy(header.data() + sizeof(packetSize), tag.data(), tag.size());

        burstBegin();
        bool sent = burstSend(header.data(), static_cast<uint32_t>(header.size()));
        if (sent)
            sent = burstSend(encrypted.data(), static_cast<uint32_t>(encrypted.size()));
        if (sent)
            burstPush();
        burstEnd();

        if (sent)
            ++m_foreverSendCounter;
        return sent;
    }

    // Remaining 12 bytes are the zero auth tag before encrypted mode.
    burstBegin();

    bool sent = burstSend(header.data(), static_cast<uint32_t>(header.size()));
    if (sent)
    {
        sent = burstSend(reinterpret_cast<const uint8_t*>(&opcode), static_cast<uint32_t>(sizeof(opcode)));
    }

    if (sent && payloadSize != 0)
        sent = burstSend(payload, payloadSize);

    if (sent)
        burstPush();

    burstEnd();

    if (sent)
        ++m_foreverSendCounter;

    return sent;
}

bool WorldSocket::sendForeverAuthChallenge()
{
    using namespace AscEmu::Version::Forever;

    if (RAND_bytes(m_foreverServerChallenge.data(), static_cast<int>(m_foreverServerChallenge.size())) != 1 || RAND_bytes(m_foreverDosChallenge.data(), static_cast<int>(m_foreverDosChallenge.size())) != 1)
    {
        sLogger.failure("WorldSocket::Forever: RAND_bytes failed while creating auth challenge.");
        return false;
    }

    std::array<uint8_t, ForeverWorldV2::AuthChallengePayloadSize> payload{};

    std::memcpy(payload.data(), m_foreverDosChallenge.data(), m_foreverDosChallenge.size());

    std::memcpy(payload.data() + m_foreverDosChallenge.size(), m_foreverServerChallenge.data(), m_foreverServerChallenge.size());

    payload.back() = 1; // DosZeroBits

    return sendForeverPacket(SMSG_AUTH_CHALLENGE, payload.data(), static_cast<uint32_t>(payload.size()));
}

bool WorldSocket::processForeverAuthPacket()
{
    using namespace AscEmu::Version::Forever;

    if (m_foreverPacketRemaining == 0)
    {
        if (readBuffer.GetSize() < ForeverWorldV2::ClientHeaderSize)
            return false;

        std::array<uint8_t, ForeverWorldV2::ClientHeaderSize> header{};
        if (!readBuffer.Read(header.data(), static_cast<uint32_t>(header.size())))
            return false;

        const uint32_t packetSize = readUInt32LE(header.data());
        if (packetSize < sizeof(uint32_t) || packetSize > ForeverWorldV2::MaxPacketSize)
        {
            sLogger.failure("WorldSocket::Forever: invalid World V2 packet size {} from {}:{}.", packetSize, getRemoteIp(), getRemotePort());
            disconnect();
            m_foreverWorldState = ForeverWorldState::Disabled;
            return false;
        }

        bool nonZeroAuthTag = false;
        for (uint32_t i = sizeof(uint32_t); i < ForeverWorldV2::ServerHeaderSize; ++i)
        {
            if (header[i] != 0)
            {
                nonZeroAuthTag = true;
                break;
            }
        }

        m_foreverPacketOpcode =
            readUInt32LE(header.data() + ForeverWorldV2::ServerHeaderSize);
        m_foreverPacketRemaining =
            packetSize - static_cast<uint32_t>(sizeof(uint32_t));

        if (isForeverMovementWireOpcode(m_foreverPacketOpcode))
        {
        }
        else if (m_foreverPacketOpcode == foreverWireOpcode(CMSG_PING))
        {
            sLogger.debugOpcode("WorldSocket::Forever: RX header opcode=0x{:08X}, payload={} byte(s), auth-tag={}.", m_foreverPacketOpcode, m_foreverPacketRemaining, nonZeroAuthTag ? "non-zero" : "zero");
        }
    }

    if (readBuffer.GetSize() < m_foreverPacketRemaining)
        return false;

    const uint32_t opcode = m_foreverPacketOpcode;
    const uint32_t payloadSize = m_foreverPacketRemaining;

    std::vector<uint8_t> payload(payloadSize);
    if (payloadSize != 0 &&
        !readBuffer.Read(payload.data(), payloadSize))
    {
        return false;
    }

    m_foreverPacketOpcode = 0;
    m_foreverPacketRemaining = 0;
    ++m_foreverRecvCounter;

    if (opcode == foreverWireOpcode(CMSG_AUTH_SESSION))
    {
        return processForeverAuthSession(opcode, payload);
    }

    if (opcode == foreverWireOpcode(CMSG_AUTH_CONTINUED_SESSION))
    {
        return processForeverAuthContinuedSession(opcode, payload);
    }

    if (opcode == foreverWireOpcode(CMSG_ENTER_ENCRYPTED_MODE_ACK))
        return processForeverEnterEncryptedModeAck(opcode, payload);

    if (isForeverMovementWireOpcode(opcode))
    {
    }
    else if (opcode == foreverWireOpcode(CMSG_PING))
    {
        sLogger.debugOpcode("WorldSocket::Forever: RX opcode=0x{:08X}, payload={} byte(s), hex=[{}]", opcode, payload.size(), Util::ByteArrayToHexString(payload.data(), static_cast<uint32_t>(payload.size())));
    }

    if (opcode == foreverWireOpcode(CMSG_PING))
    {
        if (payload.size() != sizeof(uint64_t))
        {
            sLogger.warning("WorldSocket::Forever: malformed CMSG_PING payload={} byte(s); expected 8.", payload.size());
            return true;
        }

        const uint32_t serial = readUInt32LE(payload.data());
        const uint32_t latency = readUInt32LE(payload.data() + sizeof(uint32_t));
        const uint64_t rawPing = readUInt64LE(payload.data());

        std::array<uint8_t, sizeof(uint32_t)> pong{};
        std::memcpy(pong.data(), &serial, sizeof(serial));

        sLogger.debugOpcode("WorldSocket::Forever: CMSG_PING serial={} latency={} raw={} -> SMSG_PONG.", serial, latency, rawPing);

        if (!sendForeverPacket(SMSG_PONG, pong.data(), static_cast<uint32_t>(pong.size())))
        {
            sLogger.failure("WorldSocket::Forever: failed to send SMSG_PONG for serial {}.", serial);
            disconnect();
            m_foreverWorldState = ForeverWorldState::Disabled;
            return false;
        }

        return true;
    }

    sLogger.info("WorldSocket::Forever: unhandled pre-auth opcode=0x{:08X}; state={}, build={}.", opcode, static_cast<uint32_t>(m_foreverWorldState), m_foreverClientBuild);

    return true;
}

bool WorldSocket::beginForeverInstanceLogin(uint32_t guidLow)
{
    using namespace AscEmu::Version::Forever;

    if (m_session == nullptr || m_foreverWorldState != ForeverWorldState::Encrypted)
        return false;
    if (m_foreverPendingLoginGuid != 0)
    {
        sLogger.warning("WorldSocket::Forever: ignoring duplicate CMSG_PLAYER_LOGIN while instance handoff for guidLow={} is pending.", m_foreverPendingLoginGuid);
        return true;
    }

    uint32_t randomKey = 0;
    if (RAND_bytes(reinterpret_cast<unsigned char*>(&randomKey), sizeof(randomKey)) != 1)
        return false;
    randomKey &= 0x7FFFFFFFU;

    const uint64_t connectToKey = static_cast<uint64_t>(m_session->GetAccountId()) | (uint64_t(1) << 32U) | (static_cast<uint64_t>(randomKey) << 33U);
    const uint32_t nativeRealmAddress = (m_foreverRegionId << 24U) | ((m_foreverBattlegroupId & 0xFFU) << 16U) | (m_foreverRealmId & 0xFFFFU);
    constexpr uint32_t key3 = 0;

    std::array<uint8_t, 4> address{};
    std::string connectHost = worldConfig.listen.listenHost;
    if (connectHost.empty() || connectHost == "0.0.0.0")
    {
        if (getRemoteIp().rfind("127.", 0) == 0)
            connectHost = "127.0.0.1";
        else
        {
            sLogger.failure("WorldSocket::Forever: cannot build SMSG_CONNECT_TO because Listen.Host='{}' is not a client-reachable IPv4 address.", worldConfig.listen.listenHost);
            return false;
        }
    }

    if (!parseIpv4Address(connectHost, address))
    {
        sLogger.failure("WorldSocket::Forever: SMSG_CONNECT_TO currently requires an IPv4 Listen.Host, got '{}'.", connectHost);
        return false;
    }

    ByteBuffer payload;
    payload << uint32_t(1); // payload count
    payload << uint32_t(17); // WorldAttempt1
    payload << uint8_t(1); // instance connection
    payload << connectToKey;
    payload << nativeRealmAddress;
    payload << key3;
    payload << uint8_t(1); // IPv4
    payload.append(address.data(), address.size());
    payload << uint16_t(worldConfig.listen.listenPort);
    payload << uint8_t(0) << uint8_t(0) << uint8_t(0) << uint8_t(0) << uint8_t(0); // empty BleepToken bit fields
    payload << uint64_t(0); // token lifespan

    {
        std::lock_guard lock(sForeverPendingInstanceLoginsLock);
        sForeverPendingInstanceLogins[connectToKey] = { m_session, this, guidLow, m_foreverClientBuild, m_foreverSessionKey, m_foreverRegionId, m_foreverBattlegroupId, m_foreverRealmId, nativeRealmAddress, key3 };
    }

    m_foreverConnectToKey = connectToKey;
    m_foreverPendingLoginGuid = guidLow;

    if (!sendForeverPacket(SMSG_CONNECT_TO, payload.contents(), static_cast<uint32_t>(payload.size())))
    {
        std::lock_guard lock(sForeverPendingInstanceLoginsLock);
        sForeverPendingInstanceLogins.erase(connectToKey);
        return false;
    }

    return true;
}

bool WorldSocket::processForeverAuthSession(uint32_t opcode, const std::vector<uint8_t>& payload)
{
    using namespace AscEmu::Version::Forever;

    if (payload.size() < ForeverWorldV2::AuthSessionFixedSize + sizeof(uint32_t))
    {
        sLogger.failure("WorldSocket::Forever: malformed CMSG_AUTH_SESSION 0x{:08X}; " "only {} payload byte(s).", opcode, payload.size());
        disconnect();
        m_foreverWorldState = ForeverWorldState::Disabled;
        return false;
    }

    size_t offset = 0;

    (void)readUInt64LE(payload.data() + offset);
    offset += sizeof(uint64_t);

    const uint32_t regionId = readUInt32LE(payload.data() + offset);
    offset += sizeof(uint32_t);

    const uint32_t battlegroupId = readUInt32LE(payload.data() + offset);
    offset += sizeof(uint32_t);

    const uint32_t realmId = readUInt32LE(payload.data() + offset);
    offset += sizeof(uint32_t);

    m_foreverRegionId = regionId;
    m_foreverBattlegroupId = battlegroupId;
    m_foreverRealmId = realmId;

    std::array<uint8_t, 32> localChallenge{};
    std::memcpy(localChallenge.data(), payload.data() + offset, localChallenge.size());
    offset += localChallenge.size();

    std::array<uint8_t, 24> digest{};
    std::memcpy(digest.data(), payload.data() + offset, digest.size());
    offset += digest.size();

    ++offset; // useIPv6 flag; currently not needed server-side.

    const uint32_t ticketSize = readUInt32LE(payload.data() + offset);
    offset += sizeof(uint32_t);

    if (ticketSize == 0 || ticketSize > payload.size() - offset)
    {
        sLogger.failure("WorldSocket::Forever: malformed CMSG_AUTH_SESSION ticket size {} " "(remaining {}).", ticketSize, payload.size() - offset);
        disconnect();
        m_foreverWorldState = ForeverWorldState::Disabled;
        return false;
    }

    const std::string realmJoinTicket(reinterpret_cast<const char*>(payload.data() + offset), ticketSize);

    offset += ticketSize;


    AscEmu::BattlenetComm::PendingWorldSession pending;
    if (!AscEmu::BattlenetComm::sBattleNetCommClient.getPendingSession(realmJoinTicket, pending, false))
    {
        sLogger.failure("WorldSocket::Forever: RealmJoinTicket has no pending " "BattleNetComm world session.");
        disconnect();
        m_foreverWorldState = ForeverWorldState::Disabled;
        return false;
    }

    m_foreverClientBuild = pending.clientBuild;
    m_clientBuild = pending.clientBuild;
    m_accountName = pending.gameAccountName;
    m_foreverBattleNetAccountId = pending.accountId;
    m_foreverGameAccountId = pending.gameAccountId;
    m_foreverGameAccountName = pending.gameAccountName;

    // Build 70009 is protocol-supported, but no verified build-auth key is
    // registered for it yet. Stop after observing CMSG_AUTH_SESSION rather than
    // attempting authentication with a guessed key. Do not consume the pending
    // one-shot session in this state.
    if (pending.clientBuild == 70009U)
    {
        sLogger.info("WorldSocket::Forever: AUTHKEY_SCAN build={} account={} game_account={} pending_region={} pending_realm={} packet_region={} packet_realm={}", pending.clientBuild, pending.accountId, pending.gameAccountId, pending.region, pending.realmId, regionId, realmId);
        sLogger.info("WorldSocket::Forever: AUTHKEY_SCAN world_auth_key_data={}", foreverBytesToHex(pending.worldAuthKeyData.data(), pending.worldAuthKeyData.size()));
        sLogger.info("WorldSocket::Forever: AUTHKEY_SCAN local_challenge={}", foreverBytesToHex(localChallenge.data(), localChallenge.size()));
        sLogger.info("WorldSocket::Forever: AUTHKEY_SCAN server_challenge={}", foreverBytesToHex(m_foreverServerChallenge.data(), m_foreverServerChallenge.size()));
        sLogger.info("WorldSocket::Forever: AUTHKEY_SCAN client_digest={}", foreverBytesToHex(digest.data(), digest.size()));
        sLogger.info("WorldSocket::Forever: AUTHKEY_SCAN realm_join_ticket_bytes={} join_secret={}", realmJoinTicket.size(), foreverBytesToHex(pending.joinSecret.data(), pending.joinSecret.size()));
        
        m_foreverWorldState = ForeverWorldState::AuthSessionObserved;
        sLogger.info("WorldSocket::Forever: CMSG_AUTH_SESSION observed for build {}; authentication is intentionally stopped because no verified build-auth key is registered.", pending.clientBuild);
        return true;
    }

    const auto buildAuthKey = AscEmu::Version::Forever::getBuildAuthKey(pending.clientBuild);
    if (!buildAuthKey.has_value())
    {
        sLogger.failure("WorldSocket::Forever: no build-auth key registered for build {}.", pending.clientBuild);
        disconnect();
        m_foreverWorldState = ForeverWorldState::Disabled;
        return false;
    }

    std::array<uint8_t, 64> calculatedDigest{};
    if (!calculateForeverAuthDigest(pending.worldAuthKeyData, buildAuthKey.value(), localChallenge, m_foreverServerChallenge, calculatedDigest))
    {
        sLogger.failure("WorldSocket::Forever: failed to calculate auth digest for build {}.", pending.clientBuild);
        disconnect();
        m_foreverWorldState = ForeverWorldState::Disabled;
        return false;
    }

    if (!foreverDigestMatches(calculatedDigest, digest))
    {
        sLogger.failure("WorldSocket::Forever: CMSG_AUTH_SESSION digest mismatch for account={} build={}.", pending.accountId, pending.clientBuild);
        disconnect();
        m_foreverWorldState = ForeverWorldState::Disabled;
        return false;
    }


    // Authentication succeeded. Keep session secrets out of normal logs.

    // The pending ticket is one-shot. Consume it only after the digest was
    // verified successfully so failed auth attempts cannot burn valid state.
    AscEmu::BattlenetComm::PendingWorldSession consumed;
    if (!AscEmu::BattlenetComm::sBattleNetCommClient.getPendingSession(realmJoinTicket, consumed, true))
    {
        sLogger.failure("WorldSocket::Forever: verified pending session disappeared before consume.");
        disconnect();
        m_foreverWorldState = ForeverWorldState::Disabled;
        return false;
    }

    m_foreverWorldState = ForeverWorldState::AuthSessionObserved;

    if (!deriveForeverKeys(pending.worldAuthKeyData, m_foreverServerChallenge, localChallenge, m_foreverSessionKey, m_foreverEncryptionKey))
    {
        sLogger.failure("WorldSocket::Forever: failed to derive World V2 session/encryption keys.");
        disconnect();
        m_foreverWorldState = ForeverWorldState::Disabled;
        return false;
    }

    if (!sendForeverEnterEncryptedMode())
    {
        sLogger.failure("WorldSocket::Forever: failed to send SMSG_ENTER_ENCRYPTED_MODE.");
        disconnect();
        m_foreverWorldState = ForeverWorldState::Disabled;
        return false;
    }

    m_foreverWorldState = ForeverWorldState::AwaitEncryptionAck;

    return true;
}

bool WorldSocket::processForeverAuthContinuedSession(uint32_t opcode, const std::vector<uint8_t>& payload)
{
    using namespace AscEmu::Version::Forever;

    constexpr size_t ExpectedSize = sizeof(uint64_t) + 32U + 24U + sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint32_t);
    if (opcode != foreverWireOpcode(CMSG_AUTH_CONTINUED_SESSION) || payload.size() != ExpectedSize)
    {
        sLogger.failure("WorldSocket::Forever: malformed CMSG_AUTH_CONTINUED_SESSION opcode=0x{:08X}, payload={} byte(s).", opcode, payload.size());
        return false;
    }

    size_t offset = 0;
    (void)readUInt64LE(payload.data() + offset);
    offset += sizeof(uint64_t);

    std::array<uint8_t, 32> localChallenge{};
    std::memcpy(localChallenge.data(), payload.data() + offset, localChallenge.size());
    offset += localChallenge.size();

    std::array<uint8_t, 24> receivedDigest{};
    std::memcpy(receivedDigest.data(), payload.data() + offset, receivedDigest.size());
    offset += receivedDigest.size();

    const uint64_t connectToKey = readUInt64LE(payload.data() + offset);
    offset += sizeof(uint64_t);
    const uint32_t nativeRealmAddress = readUInt32LE(payload.data() + offset);
    offset += sizeof(uint32_t);
    const uint32_t key3 = readUInt32LE(payload.data() + offset);

    ForeverPendingInstanceLogin pending;
    {
        std::lock_guard lock(sForeverPendingInstanceLoginsLock);
        const auto itr = sForeverPendingInstanceLogins.find(connectToKey);
        if (itr == sForeverPendingInstanceLogins.end())
        {
            sLogger.failure("WorldSocket::Forever: CMSG_AUTH_CONTINUED_SESSION rejected unknown key=0x{:016X}.", connectToKey);
            return false;
        }
        pending = itr->second;
    }

    if (pending.session == nullptr || pending.nativeRealmAddress != nativeRealmAddress || pending.key3 != key3)
    {
        sLogger.failure("WorldSocket::Forever: CMSG_AUTH_CONTINUED_SESSION key metadata mismatch key=0x{:016X} realm=0x{:08X} key3={}.", connectToKey, nativeRealmAddress, key3);
        return false;
    }

    std::array<uint8_t, 64> calculatedDigest{};
    if (!foreverHmacSha512(pending.sessionKey.data(), pending.sessionKey.size(), { { reinterpret_cast<const uint8_t*>(&connectToKey), sizeof(connectToKey) }, { localChallenge.data(), localChallenge.size() }, { m_foreverServerChallenge.data(), m_foreverServerChallenge.size() }, { ContinuedSessionSeed.data(), ContinuedSessionSeed.size() } }, calculatedDigest) || CRYPTO_memcmp(calculatedDigest.data(), receivedDigest.data(), receivedDigest.size()) != 0)
    {
        sLogger.failure("WorldSocket::Forever: CMSG_AUTH_CONTINUED_SESSION digest mismatch key=0x{:016X} account={}.", connectToKey, pending.session->GetAccountId());
        return false;
    }

    m_foreverClientBuild = pending.clientBuild;
    m_clientBuild = pending.clientBuild;
    m_foreverRegionId = pending.regionId;
    m_foreverBattlegroupId = pending.battlegroupId;
    m_foreverRealmId = pending.realmId;
    m_foreverSessionKey = pending.sessionKey;
    if (!deriveForeverEncryptionKeyFromSession(m_foreverSessionKey, localChallenge, m_foreverServerChallenge, m_foreverEncryptionKey))
        return false;

    m_foreverContinuedSession = true;
    m_foreverConnectToKey = connectToKey;
    m_foreverPendingLoginGuid = pending.guidLow;
    m_foreverContinuedWorldSession = pending.session;
    m_foreverGameAccountId = pending.session->GetAccountId();
    m_foreverWorldState = ForeverWorldState::AuthSessionObserved;

    if (!sendForeverEnterEncryptedMode())
        return false;

    m_foreverWorldState = ForeverWorldState::AwaitEncryptionAck;
    return true;
}

bool WorldSocket::sendForeverEnterEncryptedMode()
{
    using namespace AscEmu::Version::Forever;

    if (!AscEmu::Version::Forever::supportsBuild(m_foreverClientBuild))
        return false;

    std::array<uint8_t, 64> signature{};
    if (!createForeverEnterEncryptedModeSignature(m_foreverEncryptionKey, true, signature))
    {
        sLogger.failure("WorldSocket::Forever: Ed25519ctx signing failed for SMSG_ENTER_ENCRYPTED_MODE.");
        return false;
    }

    std::array<uint8_t, 69> payload{};
    const uint32_t regionGroup =
        static_cast<uint32_t>(ForeverWorldV2::EnterEncryptedModeRegionGroup);
    std::memcpy(payload.data(), &regionGroup, sizeof(regionGroup));
    std::memcpy(payload.data() + sizeof(regionGroup), signature.data(), signature.size());
    payload.back() = 0x80; // Enabled=true, MSB-first bit writer flush.

    const bool sent = sendForeverPacket(SMSG_ENTER_ENCRYPTED_MODE, payload.data(), static_cast<uint32_t>(payload.size()));

    if (sent)
    {
    }

    return sent;
}

bool WorldSocket::processForeverEnterEncryptedModeAck(uint32_t opcode, const std::vector<uint8_t>& payload)
{
    using namespace AscEmu::Version::Forever;

    if (m_foreverWorldState != ForeverWorldState::AwaitEncryptionAck)
    {
        sLogger.failure("WorldSocket::Forever: unexpected encrypted-mode ACK 0x{:08X} while state={}.", opcode, static_cast<uint32_t>(m_foreverWorldState));
        return false;
    }

    if (opcode != foreverWireOpcode(CMSG_ENTER_ENCRYPTED_MODE_ACK) || !payload.empty())
    {
        sLogger.failure("WorldSocket::Forever: malformed encrypted-mode ACK opcode=0x{:08X}, payload={}.", opcode, payload.size());
        return false;
    }

    m_foreverCryptoSendCounter = m_foreverSendCounter;
    m_foreverCryptoRecvCounter = m_foreverRecvCounter;
    m_foreverEncryptedHeaderReady = false;
    m_foreverEncryptedPacketSize = 0;
    m_foreverEncryptedPacketTag.fill(0);
    m_foreverEncryptedOpcode.fill(0);
    m_foreverWorldState = ForeverWorldState::Encrypted;


    if (m_foreverContinuedSession)
    {
        if (m_foreverContinuedWorldSession == nullptr || m_foreverPendingLoginGuid == 0)
            return false;

        m_session = m_foreverContinuedWorldSession;
        m_session->SetForeverInstanceSocket(this);
        isAuthenticated = true;

        ForeverPendingInstanceLogin pending;
        {
            std::lock_guard lock(sForeverPendingInstanceLoginsLock);
            const auto itr = sForeverPendingInstanceLogins.find(m_foreverConnectToKey);
            if (itr != sForeverPendingInstanceLogins.end())
                pending = itr->second;
        }

        {
            std::lock_guard lock(sForeverPendingInstanceLoginsLock);
            sForeverPendingInstanceLogins.erase(m_foreverConnectToKey);
        }

        if (pending.realmSocket != nullptr)
        {
            pending.realmSocket->m_foreverPendingLoginGuid = 0;
            pending.realmSocket->m_foreverConnectToKey = 0;
        }

        if (!sendForeverPacket(SMSG_RESUME_COMMS, nullptr, 0))
            return false;

        sLogger.info("WorldSocket::Forever: instance connection attached for account {}.", m_session->GetAccountId());
        m_session->beginForeverPlayerLogin(m_foreverPendingLoginGuid);
        return true;
    }

    if (m_foreverGameAccountId == 0 || m_foreverGameAccountName.empty())
    {
        sLogger.failure("WorldSocket::Forever: cannot create WorldSession after encryption ACK: battlenet_account={}, game_account={}, name='{}'.", m_foreverBattleNetAccountId, m_foreverGameAccountId, m_foreverGameAccountName);
        disconnect();
        m_foreverWorldState = ForeverWorldState::Disabled;
        return false;
    }

    if (m_session != nullptr)
    {
        sLogger.failure("WorldSocket::Forever: socket already owns a WorldSession before Forever handoff.");
        disconnect();
        m_foreverWorldState = ForeverWorldState::Disabled;
        return false;
    }

    if (WorldSession* existing = sWorld.getSessionByAccountId(m_foreverGameAccountId))
    {
        existing->Disconnect();
    }

    auto sessionHolder = std::make_unique<WorldSession>(m_foreverGameAccountId, m_foreverGameAccountName, this);

    m_session = sessionHolder.get();
    m_session->SetClientBuild(m_foreverClientBuild);
    m_session->LoadSecurity("");
    m_session->SetAccountFlags(AF_FULL_FOREVER);
    m_session->m_lastPing = static_cast<uint32_t>(UNIXTIME);
    m_session->_latency = m_latency;
    m_session->SetGlobalUpdateOwner();

    if (!sendForeverPostAuthBootstrap())
    {
        sLogger.failure("WorldSocket::Forever: failed to send post-auth bootstrap after encrypted-mode ACK.");
        m_session = nullptr;
        disconnect();
        m_foreverWorldState = ForeverWorldState::Disabled;
        return false;
    }

    sWorld.addGlobalSession(m_session);
    sWorld.addSession(std::move(sessionHolder));
    isAuthenticated = true;

    sLogger.info("WorldSocket::Forever: session ready for game account {}.", m_foreverGameAccountId);

    return true;
}

bool WorldSocket::sendForeverPostAuthBootstrap()
{
    using namespace AscEmu::Version::Forever;

    struct PacketView
    {
        uint32_t opcode;
        const uint8_t* data;
        uint32_t size;
        const char* name;
    };

    // AuthResponse contains a Unix timestamp at bytes 48..51. The official
    // 69893 queue capture and the later official 69913 no-queue capture both
    // show this field tracking the current server time, so do not replay the
    // stale timestamp from the capture.
    auto authResponse = PostAuthBootstrap::AuthResponse460001NoQueue;
    const auto authResponseUnixTime = static_cast<uint32_t>(std::time(nullptr));
    std::memcpy(authResponse.data() + 48, &authResponseUnixTime, sizeof(authResponseUnixTime));

    // Keep the already-working auth/session prefix unchanged. The glue-state
    // packets below are sent separately because AccountDataTimes is dynamic.
    const std::array<PacketView, 4> authPackets =
    {{
        { SMSG_POST_AUTH_650007,
          PostAuthBootstrap::Packet650007.data(),
          static_cast<uint32_t>(PostAuthBootstrap::Packet650007.size()),
          "SMSG_POST_AUTH_650007" },
        { SMSG_POST_AUTH_4602CE,
          PostAuthBootstrap::Packet4602CE.data(),
          static_cast<uint32_t>(PostAuthBootstrap::Packet4602CE.size()),
          "SMSG_POST_AUTH_4602CE" },
        { SMSG_AUTH_RESPONSE,
          authResponse.data(),
          static_cast<uint32_t>(authResponse.size()),
          "SMSG_AUTH_RESPONSE" },
        { SMSG_SET_TIME_ZONE_INFORMATION,
          PostAuthBootstrap::TimeZone460123.data(),
          static_cast<uint32_t>(PostAuthBootstrap::TimeZone460123.size()),
          "SMSG_SET_TIME_ZONE_INFORMATION" }
    }};

    for (const PacketView& packet : authPackets)
    {
        if (!sendForeverPacket(packet.opcode, packet.data, packet.size))
        {
            sLogger.failure("WorldSocket::Forever: failed to send {} opcode=0x{:08X}, payload={} byte(s).", packet.name, foreverWireOpcode(packet.opcode), packet.size);
            return false;
        }

    }

    // Match the official 69893 pre-enum glue sequence, while using the same
    // semantic AccountDataTimes serializer as the verified modern protocol path:
    //   460003(empty), 4602CB(00 02 00 00), 4601B5(AccountDataTimes),
    //   460064(FeatureStatus), 460371(Config), 460268(TutorialFlags).
    const std::array<uint8_t, 4> glueState4602CB = { 0x00, 0x02, 0x00, 0x00 };
    const std::array<uint8_t, 32> tutorialFlags460268 = {};

    auto sendGluePacket = [this](uint32_t internalOpcode, const uint8_t* data, uint32_t size, const char* name) -> bool
    {
        if (!sendForeverPacket(internalOpcode, data, size))
        {
            sLogger.failure("WorldSocket::Forever: failed to send {} opcode=0x{:08X}, payload={} byte(s).", name, foreverWireOpcode(internalOpcode), size);
            return false;
        }

        return true;
    };

    if (!sendGluePacket(SMSG_GLUE_BOOTSTRAP_EMPTY, nullptr, 0, "SMSG_GLUE_BOOTSTRAP_EMPTY"))
        return false;

    if (!sendGluePacket(SMSG_GLUE_BOOTSTRAP_STATE, glueState4602CB.data(), static_cast<uint32_t>(glueState4602CB.size()), "SMSG_GLUE_BOOTSTRAP_STATE"))
        return false;

    ByteBuffer accountDataTimes = buildForeverAccountDataTimes();
    if (!sendGluePacket(SMSG_ACCOUNT_DATA_TIMES, accountDataTimes.contents(), static_cast<uint32_t>(accountDataTimes.size()), "SMSG_ACCOUNT_DATA_TIMES"))
        return false;

    if (!sendGluePacket(SMSG_FEATURE_SYSTEM_STATUS_GLUE_SCREEN, PostAuthBootstrap::FeatureStatus460064.data(), static_cast<uint32_t>(PostAuthBootstrap::FeatureStatus460064.size()), "SMSG_FEATURE_SYSTEM_STATUS_GLUE_SCREEN"))
        return false;

    if (!sendGluePacket(SMSG_POST_AUTH_CONFIG, PostAuthBootstrap::Config460371.data(), static_cast<uint32_t>(PostAuthBootstrap::Config460371.size()), "SMSG_POST_AUTH_CONFIG"))
        return false;

    if (!sendGluePacket(SMSG_TUTORIAL_FLAGS, tutorialFlags460268.data(), static_cast<uint32_t>(tutorialFlags460268.size()), "SMSG_TUTORIAL_FLAGS"))
        return false;

    return true;
}

bool WorldSocket::encryptForeverPayload(std::vector<uint8_t>& data, std::array<uint8_t, 12>& tag)
{
    using namespace AscEmu::Version::Forever;

    if (data.empty())
        return false;

    std::array<uint8_t, 12> iv{};
    std::memcpy(iv.data(), &m_foreverCryptoSendCounter, sizeof(m_foreverCryptoSendCounter));
    std::memcpy(iv.data() + sizeof(m_foreverCryptoSendCounter), &ForeverWorldV2::ServerIvMagic, sizeof(ForeverWorldV2::ServerIvMagic));

    EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
    if (context == nullptr)
        return false;

    bool success = EVP_EncryptInit_ex(context, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1;
    if (success)
        success = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(iv.size()), nullptr) == 1;
    if (success)
        success = EVP_EncryptInit_ex(context, nullptr, nullptr, m_foreverEncryptionKey.data(), iv.data()) == 1;

    int outputLength = 0;
    if (success)
        success = EVP_EncryptUpdate(context, data.data(), &outputLength, data.data(), static_cast<int>(data.size())) == 1 &&
            outputLength == static_cast<int>(data.size());

    int finalLength = 0;
    if (success)
        success = EVP_EncryptFinal_ex(context, data.data() + outputLength, &finalLength) == 1 && finalLength == 0;
    if (success)
        success = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_GET_TAG, static_cast<int>(tag.size()), tag.data()) == 1;

    EVP_CIPHER_CTX_free(context);
    if (success)
        ++m_foreverCryptoSendCounter;
    return success;
}

bool WorldSocket::decryptForeverPayload(std::vector<uint8_t>& data, const std::array<uint8_t, 12>& tag)
{
    using namespace AscEmu::Version::Forever;

    if (data.empty())
        return false;

    std::array<uint8_t, 12> iv{};
    std::memcpy(iv.data(), &m_foreverCryptoRecvCounter, sizeof(m_foreverCryptoRecvCounter));
    std::memcpy(iv.data() + sizeof(m_foreverCryptoRecvCounter), &ForeverWorldV2::ClientIvMagic, sizeof(ForeverWorldV2::ClientIvMagic));

    EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
    if (context == nullptr)
        return false;

    bool success = EVP_DecryptInit_ex(context, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1;
    if (success)
        success = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(iv.size()), nullptr) == 1;
    if (success)
        success = EVP_DecryptInit_ex(context, nullptr, nullptr, m_foreverEncryptionKey.data(), iv.data()) == 1;

    int outputLength = 0;
    if (success)
        success = EVP_DecryptUpdate(context, data.data(), &outputLength, data.data(), static_cast<int>(data.size())) == 1 &&
            outputLength == static_cast<int>(data.size());

    std::array<uint8_t, ForeverWorldV2::AuthTagSize> mutableTag = tag;
    if (success)
        success = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_TAG, static_cast<int>(mutableTag.size()), mutableTag.data()) == 1;

    int finalLength = 0;
    if (success)
        success = EVP_DecryptFinal_ex(context, data.data() + outputLength, &finalLength) == 1 && finalLength == 0;

    EVP_CIPHER_CTX_free(context);
    if (success)
        ++m_foreverCryptoRecvCounter;
    return success;
}

bool WorldSocket::processForeverEncryptedPacket()
{
    using namespace AscEmu::Version::Forever;

    if (m_foreverWorldState != ForeverWorldState::Encrypted)
        return false;

    if (!m_foreverEncryptedHeaderReady)
    {
        if (readBuffer.GetSize() < ForeverWorldV2::ClientHeaderSize)
            return false;

        std::array<uint8_t, ForeverWorldV2::ClientHeaderSize> header{};
        if (!readBuffer.Read(header.data(), static_cast<uint32_t>(header.size())))
            return false;

        const uint32_t packetSize = readUInt32LE(header.data());
        if (packetSize < sizeof(uint32_t) || packetSize > ForeverWorldV2::MaxPacketSize)
        {
            sLogger.failure("WorldSocket::Forever: invalid encrypted packet size {}.", packetSize);
            disconnect();
            m_foreverWorldState = ForeverWorldState::Disabled;
            return false;
        }

        m_foreverEncryptedPacketSize = packetSize;
        std::memcpy(m_foreverEncryptedPacketTag.data(), header.data() + sizeof(uint32_t), m_foreverEncryptedPacketTag.size());
        std::memcpy(m_foreverEncryptedOpcode.data(), header.data() + ForeverWorldV2::ServerHeaderSize, m_foreverEncryptedOpcode.size());
        m_foreverPacketRemaining = packetSize - static_cast<uint32_t>(sizeof(uint32_t));
        m_foreverEncryptedHeaderReady = true;
    }

    if (readBuffer.GetSize() < m_foreverPacketRemaining)
        return false;

    std::vector<uint8_t> encrypted(m_foreverEncryptedPacketSize);
    std::memcpy(encrypted.data(), m_foreverEncryptedOpcode.data(), m_foreverEncryptedOpcode.size());

    if (m_foreverPacketRemaining != 0 &&
        !readBuffer.Read(encrypted.data() + sizeof(uint32_t), m_foreverPacketRemaining))
    {
        return false;
    }

    const uint64_t counter = m_foreverCryptoRecvCounter;
    const auto tag = m_foreverEncryptedPacketTag;

    m_foreverEncryptedHeaderReady = false;
    m_foreverEncryptedPacketSize = 0;
    m_foreverPacketRemaining = 0;
    m_foreverEncryptedPacketTag.fill(0);
    m_foreverEncryptedOpcode.fill(0);

    if (!decryptForeverPayload(encrypted, tag))
    {
        sLogger.failure("WorldSocket::Forever: AES-256-GCM authentication failed at recv counter {}.", counter);
        disconnect();
        m_foreverWorldState = ForeverWorldState::Disabled;
        return false;
    }

    ++m_foreverRecvCounter;

    const uint32_t opcode = readUInt32LE(encrypted.data());
    const size_t payloadSize = encrypted.size() - sizeof(uint32_t);
    const uint8_t* payload = payloadSize != 0 ? encrypted.data() + sizeof(uint32_t) : nullptr;


    return dispatchForeverOpcode(opcode, payload, payloadSize);
}

bool WorldSocket::sendVersionedPacket(WorldPacket* packet)
{
    if (m_foreverWorldState == ForeverWorldState::Disabled)
        return false;

    if (packet == nullptr)
        return true;

    if (m_foreverWorldState == ForeverWorldState::Encrypted)
    {
        const uint32_t rawOpcode = sOpcodeTables.getHexValueForExpansion(packet->getOpcode(), WoW::Expansion::Forever);
        if (rawOpcode == 0)
        {
            sLogger.debug("WorldSocket::Forever: blocked unmapped managed packet opcode={} payload={}.", packet->getOpcode(), packet->size());
            return true;
        }

        return sendForeverWorldPacket(rawOpcode, packet->contents(), static_cast<uint32_t>(packet->size()));
    }

    sLogger.debug("WorldSocket::Forever: blocked managed packet opcode={} while Forever socket state is not encrypted.", packet->getOpcode());
    return true;
}
