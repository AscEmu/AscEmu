/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BNetSrpV1.hpp"

#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string_view>
#include <vector>

namespace AscEmu::Cryptography::BNetSrpV1
{
    namespace
    {
        constexpr char ModulusHex[] =
            "86A7F6DEEB306CE519770FE37D556F29944132554DED0BD68205E27F3231FEF5"
            "A10108238A3150C59CAF7B0B6478691C13A6ACF5E1B5ADAFD4A943D4A21A142B"
            "800E8A55F8BFBAC700EB77A7235EE5A609E350EA9FC19F10D921C2FA832E4461"
            "B7125D38D254A0BE873DFC27858ACB3F8B9F258461E4373BC3A6C2A9634324AB";

        struct BnDeleter
        {
            void operator()(BIGNUM* value) const noexcept
            {
                BN_clear_free(value);
            }
        };

        struct BnCtxDeleter
        {
            void operator()(BN_CTX* value) const noexcept
            {
                BN_CTX_free(value);
            }
        };

        struct EvpMdCtxDeleter
        {
            void operator()(EVP_MD_CTX* value) const noexcept
            {
                EVP_MD_CTX_free(value);
            }
        };

        using BnPtr = std::unique_ptr<BIGNUM, BnDeleter>;
        using BnCtxPtr = std::unique_ptr<BN_CTX, BnCtxDeleter>;
        using EvpMdCtxPtr = std::unique_ptr<EVP_MD_CTX, EvpMdCtxDeleter>;

        std::string upperAscii(std::string value)
        {
            std::transform(
                value.begin(),
                value.end(),
                value.begin(),
                [](unsigned char character)
                {
                    return static_cast<char>(std::toupper(character));
                }
            );
            return value;
        }

        bool sha256(
            const std::vector<std::pair<const uint8_t*, size_t>>& chunks,
            std::array<uint8_t, 32>& digest)
        {
            EvpMdCtxPtr context(EVP_MD_CTX_new());
            if (!context)
                return false;

            if (EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) != 1)
                return false;

            for (const auto& [data, size] : chunks)
            {
                if (size == 0)
                    continue;

                if (data == nullptr || EVP_DigestUpdate(context.get(), data, size) != 1)
                    return false;
            }

            unsigned int digestSize = 0;
            if (EVP_DigestFinal_ex(context.get(), digest.data(), &digestSize) != 1)
                return false;

            return digestSize == digest.size();
        }

        bool sha256String(std::string_view value, std::array<uint8_t, 32>& digest)
        {
            return sha256(
                {
                    {
                        reinterpret_cast<const uint8_t*>(value.data()),
                        value.size()
                    }
                },
                digest
            );
        }

        std::string bytesToHex(const uint8_t* data, size_t size)
        {
            std::ostringstream stream;
            stream << std::uppercase << std::hex << std::setfill('0');

            for (size_t index = 0; index < size; ++index)
                stream << std::setw(2) << static_cast<unsigned int>(data[index]);

            return stream.str();
        }

        bool hexToBytes(const std::string& hex, std::vector<uint8_t>& bytes)
        {
            if (hex.empty() || (hex.size() % 2) != 0)
                return false;

            bytes.clear();
            bytes.reserve(hex.size() / 2);

            for (size_t offset = 0; offset < hex.size(); offset += 2)
            {
                unsigned int value = 0;
                std::istringstream stream(hex.substr(offset, 2));
                stream >> std::hex >> value;
                if (stream.fail())
                    return false;

                bytes.push_back(static_cast<uint8_t>(value));
            }

            return true;
        }

        BnPtr makeBn()
        {
            return BnPtr(BN_new());
        }

        BnPtr makeBnFromHex(const char* hex)
        {
            BIGNUM* raw = nullptr;
            if (BN_hex2bn(&raw, hex) == 0)
                return {};

            return BnPtr(raw);
        }

        BnPtr makeBnFromBytes(const uint8_t* data, size_t size)
        {
            if (data == nullptr || size == 0)
                return {};

            return BnPtr(BN_bin2bn(data, static_cast<int>(size), nullptr));
        }

        BnPtr makeBnFromLittleEndianBytes(const uint8_t* data, size_t size)
        {
            if (data == nullptr || size == 0)
                return {};

            std::vector<uint8_t> bigEndian(data, data + size);
            std::reverse(bigEndian.begin(), bigEndian.end());

            return BnPtr(BN_bin2bn(
                bigEndian.data(),
                static_cast<int>(bigEndian.size()),
                nullptr));
        }

        std::string bnToFixedHex(const BIGNUM* value, size_t byteCount)
        {
            if (value == nullptr)
                return {};

            std::vector<uint8_t> bytes(byteCount);
            if (BN_bn2binpad(value, bytes.data(), static_cast<int>(bytes.size())) != static_cast<int>(bytes.size()))
                return {};

            return bytesToHex(bytes.data(), bytes.size());
        }

        std::string bnToFixedLittleEndianHex(const BIGNUM* value, size_t byteCount)
        {
            if (value == nullptr)
                return {};

            std::vector<uint8_t> bytes(byteCount);
            if (BN_bn2binpad(
                    value,
                    bytes.data(),
                    static_cast<int>(bytes.size())) != static_cast<int>(bytes.size()))
            {
                return {};
            }

            std::reverse(bytes.begin(), bytes.end());
            return bytesToHex(bytes.data(), bytes.size());
        }

        bool buildCommonValues(BnPtr& modulus, BnPtr& generator, BnPtr& multiplier)
        {
            modulus = makeBnFromHex(ModulusHex);
            generator = makeBn();
            multiplier = makeBn();

            if (!modulus || !generator || !multiplier)
                return false;

            if (BN_set_word(generator.get(), 2) != 1)
                return false;

            std::array<uint8_t, ModulusLength> nBytes{};
            std::array<uint8_t, ModulusLength> gBytes{};
            if (BN_bn2binpad(modulus.get(), nBytes.data(), static_cast<int>(nBytes.size())) != static_cast<int>(nBytes.size()))
                return false;
            if (BN_bn2binpad(generator.get(), gBytes.data(), static_cast<int>(gBytes.size())) != static_cast<int>(gBytes.size()))
                return false;

            std::array<uint8_t, 32> kDigest{};
            if (!sha256(
                    {
                        {nBytes.data(), nBytes.size()},
                        {gBytes.data(), gBytes.size()}
                    },
                    kDigest))
            {
                return false;
            }

            BnPtr k = makeBnFromBytes(kDigest.data(), kDigest.size());
            if (!k)
                return false;

            if (BN_copy(multiplier.get(), k.get()) == nullptr)
                return false;

            return true;
        }

        bool calculateSrpUsername(const std::string& accountName, std::string& output)
        {
            const std::string normalizedAccountName = upperAscii(accountName);
            std::array<uint8_t, 32> accountDigest{};
            if (!sha256String(normalizedAccountName, accountDigest))
                return false;

            output = bytesToHex(accountDigest.data(), accountDigest.size());
            return true;
        }

        bool bnToBrokenEvidenceVector(const BIGNUM* value, std::vector<uint8_t>& bytes)
        {
            if (value == nullptr || BN_is_negative(value))
                return false;

            // This deliberately reproduces Battle.net SRP v1's historical
            // evidence serialization. For exact byte boundaries it emits one
            // leading zero byte: (numBits + 8) >> 3.
            const int bitCount = BN_num_bits(value);
            const size_t byteCount = static_cast<size_t>((bitCount + 8) >> 3);

            bytes.assign(byteCount, 0);
            if (byteCount == 0)
                return true;

            return BN_bn2binpad(
                value,
                bytes.data(),
                static_cast<int>(bytes.size())) == static_cast<int>(bytes.size());
        }

        bool parseFixedHex(
            const std::string& hex,
            size_t expectedBytes,
            std::vector<uint8_t>& bytes)
        {
            // WoW's SRP JSON values are BigNumber hex strings, not guaranteed
            // fixed-width byte strings. A leading zero nibble/byte may therefore
            // be omitted (for example public_A can be 255 hex chars instead of
            // 256). Normalize by left-padding to the expected width.
            if (hex.empty() || hex.size() > expectedBytes * 2)
                return false;

            std::string normalizedHex;
            normalizedHex.reserve(expectedBytes * 2);
            normalizedHex.append(expectedBytes * 2 - hex.size(), '0');
            normalizedHex.append(hex);

            return hexToBytes(normalizedHex, bytes) &&
                bytes.size() == expectedBytes;
        }

        bool sha256VectorsToHex(
            const std::vector<uint8_t>& first,
            const std::vector<uint8_t>& second,
            const std::vector<uint8_t>& third,
            std::string& output)
        {
            std::array<uint8_t, 32> digest{};
            if (!sha256(
                    {
                        {first.data(), first.size()},
                        {second.data(), second.size()},
                        {third.data(), third.size()}
                    },
                    digest))
            {
                return false;
            }

            output = bytesToHex(digest.data(), digest.size());
            return true;
        }

        bool bnToFixedVector(const BIGNUM* value, size_t byteCount, std::vector<uint8_t>& bytes)
        {
            if (value == nullptr)
                return false;

            bytes.assign(byteCount, 0);
            return BN_bn2binpad(
                value,
                bytes.data(),
                static_cast<int>(bytes.size())) == static_cast<int>(bytes.size());
        }
    }

    bool makeRegistrationData(
        const std::string& accountName,
        const std::string& password,
        RegistrationData& output)
    {
        if (accountName.empty() || password.empty())
            return false;

        if (!calculateSrpUsername(accountName, output.srpUsername))
            return false;

        if (RAND_bytes(output.salt.data(), static_cast<int>(output.salt.size())) != 1)
            return false;

        const std::string normalizedPassword = upperAscii(password);
        const std::string credentials = output.srpUsername + ":" + normalizedPassword;

        std::array<uint8_t, 32> credentialsDigest{};
        if (!sha256String(credentials, credentialsDigest))
            return false;

        std::array<uint8_t, 32> xDigest{};
        if (!sha256(
                {
                    {output.salt.data(), output.salt.size()},
                    {credentialsDigest.data(), credentialsDigest.size()}
                },
                xDigest))
        {
            return false;
        }

        BnPtr modulus;
        BnPtr generator;
        BnPtr multiplier;
        if (!buildCommonValues(modulus, generator, multiplier))
            return false;

        // reference implementation BnetSRP6v1Base::CalculateX returns the SHA-256 digest
        // directly as BigNumber. BigNumber's binary constructor defaults to
        // littleEndian=true. k/u/evidence explicitly pass false and are BE.
        BnPtr x = makeBnFromLittleEndianBytes(
            xDigest.data(),
            xDigest.size());
        BnPtr verifier = makeBn();
        BnCtxPtr context(BN_CTX_new());
        if (!x || !verifier || !context)
            return false;

        if (BN_mod_exp(verifier.get(), generator.get(), x.get(), modulus.get(), context.get()) != 1)
            return false;

        output.saltHex = bytesToHex(output.salt.data(), output.salt.size());

        // reference implementation SRP6::CalculateVerifier stores v through ToByteVector(),
        // whose default byte order is little endian.
        output.verifierHex =
            bnToFixedLittleEndianHex(verifier.get(), ModulusLength);

        return !output.verifierHex.empty();
    }

    bool createServerChallenge(
        const std::string& accountName,
        const std::string& saltHex,
        const std::string& verifierHex,
        ServerChallenge& output)
    {
        if (accountName.empty() || saltHex.size() != SaltLength * 2 || verifierHex.size() != ModulusLength * 2)
            return false;

        std::vector<uint8_t> salt;
        if (!hexToBytes(saltHex, salt) || salt.size() != SaltLength)
            return false;

        if (!calculateSrpUsername(accountName, output.username))
            return false;

        BnPtr modulus;
        BnPtr generator;
        BnPtr multiplier;
        if (!buildCommonValues(modulus, generator, multiplier))
            return false;

        std::vector<uint8_t> verifierBytes;
        if (!hexToBytes(verifierHex, verifierBytes) ||
            verifierBytes.size() != ModulusLength)
        {
            return false;
        }

        // bnet_srp_verifier stores the raw reference implementation ToByteVector() form:
        // little-endian bytes represented as hex text in our AscEmu schema.
        BnPtr verifier = makeBnFromLittleEndianBytes(
            verifierBytes.data(),
            verifierBytes.size());
        BnPtr privateB = makeBn();
        BnPtr modulusMinusOne = makeBn();
        BnPtr publicGb = makeBn();
        BnPtr kv = makeBn();
        BnPtr publicB = makeBn();
        BnCtxPtr context(BN_CTX_new());

        if (!verifier || !privateB || !modulusMinusOne || !publicGb || !kv || !publicB || !context)
            return false;

        if (BN_copy(modulusMinusOne.get(), modulus.get()) == nullptr || BN_sub_word(modulusMinusOne.get(), 1) != 1)
            return false;

        do
        {
            if (BN_rand_range(privateB.get(), modulusMinusOne.get()) != 1)
                return false;
        } while (BN_is_zero(privateB.get()));

        if (BN_mod_exp(publicGb.get(), generator.get(), privateB.get(), modulus.get(), context.get()) != 1)
            return false;
        if (BN_mod_mul(kv.get(), multiplier.get(), verifier.get(), modulus.get(), context.get()) != 1)
            return false;
        if (BN_mod_add(publicB.get(), publicGb.get(), kv.get(), modulus.get(), context.get()) != 1)
            return false;

        output.version = 1;
        output.iterations = 1;
        output.modulus = ModulusHex;
        output.generator = "02";
        output.hashFunction = "SHA-256";
        output.salt = bytesToHex(salt.data(), salt.size());
        output.publicB = bnToFixedHex(publicB.get(), ModulusLength);
        output.privateB = bnToFixedHex(privateB.get(), ModulusLength);
        output.verifier = bnToFixedHex(verifier.get(), ModulusLength);

        return !output.publicB.empty() &&
            !output.privateB.empty() &&
            !output.verifier.empty();
    }

    bool verifyClientEvidence(
        const ServerChallenge& challenge,
        const std::string& publicAHex,
        const std::string& clientEvidenceM1Hex,
        std::string& serverEvidenceM2Hex,
        EvidenceDiagnostics* diagnostics)
    {
        serverEvidenceM2Hex.clear();

        if (challenge.publicB.size() != ModulusLength * 2 ||
            challenge.privateB.size() != ModulusLength * 2 ||
            challenge.verifier.size() != ModulusLength * 2)
        {
            return false;
        }

        std::vector<uint8_t> publicABytes;
        std::vector<uint8_t> clientM1Bytes;
        if (!parseFixedHex(publicAHex, ModulusLength, publicABytes) ||
            !parseFixedHex(clientEvidenceM1Hex, 32, clientM1Bytes))
        {
            return false;
        }

        BnPtr modulus;
        BnPtr generator;
        BnPtr multiplier;
        if (!buildCommonValues(modulus, generator, multiplier))
            return false;

        BnPtr publicA = makeBnFromBytes(publicABytes.data(), publicABytes.size());
        BnPtr publicB = makeBnFromHex(challenge.publicB.c_str());
        BnPtr privateB = makeBnFromHex(challenge.privateB.c_str());
        BnPtr verifier = makeBnFromHex(challenge.verifier.c_str());
        BnPtr clientM1 = makeBnFromBytes(clientM1Bytes.data(), clientM1Bytes.size());
        BnCtxPtr context(BN_CTX_new());

        if (!publicA || !publicB || !privateB || !verifier || !clientM1 || !context)
            return false;

        BnPtr aModuloN = makeBn();
        if (!aModuloN || BN_nnmod(aModuloN.get(), publicA.get(), modulus.get(), context.get()) != 1)
            return false;
        if (BN_is_zero(aModuloN.get()))
            return false;

        // u = SHA256(PAD128(A) || PAD128(B))
        std::array<uint8_t, ModulusLength> fixedA{};
        std::array<uint8_t, ModulusLength> fixedB{};
        if (BN_bn2binpad(publicA.get(), fixedA.data(), static_cast<int>(fixedA.size())) != static_cast<int>(fixedA.size()) ||
            BN_bn2binpad(publicB.get(), fixedB.data(), static_cast<int>(fixedB.size())) != static_cast<int>(fixedB.size()))
        {
            return false;
        }

        std::array<uint8_t, 32> uDigest{};
        if (!sha256(
                {
                    {fixedA.data(), fixedA.size()},
                    {fixedB.data(), fixedB.size()}
                },
                uDigest))
        {
            return false;
        }

        BnPtr u = makeBnFromBytes(uDigest.data(), uDigest.size());
        BnPtr uModuloN = makeBn();
        if (!u || !uModuloN ||
            BN_nnmod(uModuloN.get(), u.get(), modulus.get(), context.get()) != 1)
        {
            return false;
        }
        if (BN_is_zero(uModuloN.get()))
            return false;

        // S = (A * v^u)^b mod N
        BnPtr vu = makeBn();
        BnPtr base = makeBn();
        BnPtr sessionKey = makeBn();
        if (!vu || !base || !sessionKey)
            return false;

        if (BN_mod_exp(vu.get(), verifier.get(), u.get(), modulus.get(), context.get()) != 1)
            return false;
        if (BN_mod_mul(base.get(), publicA.get(), vu.get(), modulus.get(), context.get()) != 1)
            return false;
        if (BN_mod_exp(sessionKey.get(), base.get(), privateB.get(), modulus.get(), context.get()) != 1)
            return false;

        // Battle.net's v1 evidence uses its historical "broken" BigNumber
        // serialization rather than the fixed-width values used for u.
        std::vector<uint8_t> brokenA;
        std::vector<uint8_t> brokenB;
        std::vector<uint8_t> brokenS;
        if (!bnToBrokenEvidenceVector(publicA.get(), brokenA) ||
            !bnToBrokenEvidenceVector(publicB.get(), brokenB) ||
            !bnToBrokenEvidenceVector(sessionKey.get(), brokenS))
        {
            return false;
        }

        std::array<uint8_t, 32> expectedM1{};
        if (!sha256(
                {
                    {brokenA.data(), brokenA.size()},
                    {brokenB.data(), brokenB.size()},
                    {brokenS.data(), brokenS.size()}
                },
                expectedM1))
        {
            return false;
        }

        if (diagnostics != nullptr)
        {
            diagnostics->expectedM1 = bytesToHex(expectedM1.data(), expectedM1.size());

            std::vector<uint8_t> fixedEvidenceA;
            std::vector<uint8_t> fixedEvidenceB;
            std::vector<uint8_t> fixedEvidenceS;
            if (bnToFixedVector(publicA.get(), ModulusLength, fixedEvidenceA) &&
                bnToFixedVector(publicB.get(), ModulusLength, fixedEvidenceB) &&
                bnToFixedVector(sessionKey.get(), ModulusLength, fixedEvidenceS))
            {
                sha256VectorsToHex(
                    fixedEvidenceA,
                    fixedEvidenceB,
                    fixedEvidenceS,
                    diagnostics->fixedWidthEvidenceM1);
            }

            std::vector<uint8_t> littleA = brokenA;
            std::vector<uint8_t> littleB = brokenB;
            std::vector<uint8_t> littleS = brokenS;
            std::reverse(littleA.begin(), littleA.end());
            std::reverse(littleB.begin(), littleB.end());
            std::reverse(littleS.begin(), littleS.end());
            sha256VectorsToHex(
                littleA,
                littleB,
                littleS,
                diagnostics->littleEndianEvidenceM1);

            std::array<uint8_t, 32> reversedUDigest = uDigest;
            std::reverse(reversedUDigest.begin(), reversedUDigest.end());

            BnPtr littleEndianU = makeBnFromBytes(
                reversedUDigest.data(),
                reversedUDigest.size());
            BnPtr littleVu = makeBn();
            BnPtr littleBase = makeBn();
            BnPtr littleSessionKey = makeBn();

            if (littleEndianU && littleVu && littleBase && littleSessionKey &&
                BN_mod_exp(
                    littleVu.get(),
                    verifier.get(),
                    littleEndianU.get(),
                    modulus.get(),
                    context.get()) == 1 &&
                BN_mod_mul(
                    littleBase.get(),
                    publicA.get(),
                    littleVu.get(),
                    modulus.get(),
                    context.get()) == 1 &&
                BN_mod_exp(
                    littleSessionKey.get(),
                    littleBase.get(),
                    privateB.get(),
                    modulus.get(),
                    context.get()) == 1)
            {
                std::vector<uint8_t> littleUBrokenS;
                if (bnToBrokenEvidenceVector(
                        littleSessionKey.get(),
                        littleUBrokenS))
                {
                    sha256VectorsToHex(
                        brokenA,
                        brokenB,
                        littleUBrokenS,
                        diagnostics->littleEndianUExpectedM1);
                }
            }
        }

        if (CRYPTO_memcmp(
                expectedM1.data(),
                clientM1Bytes.data(),
                expectedM1.size()) != 0)
        {
            return false;
        }

        BnPtr verifiedM1 = makeBnFromBytes(expectedM1.data(), expectedM1.size());
        std::vector<uint8_t> brokenM1;
        if (!verifiedM1 || !bnToBrokenEvidenceVector(verifiedM1.get(), brokenM1))
            return false;

        std::array<uint8_t, 32> serverM2{};
        if (!sha256(
                {
                    {brokenA.data(), brokenA.size()},
                    {brokenM1.data(), brokenM1.size()},
                    {brokenS.data(), brokenS.size()}
                },
                serverM2))
        {
            return false;
        }

        serverEvidenceM2Hex = bytesToHex(serverM2.data(), serverM2.size());
        return true;
    }
}
