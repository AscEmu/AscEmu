/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace AscEmu::Cryptography::BNetSrpV1
{
    constexpr size_t SaltLength = 32;
    constexpr size_t ModulusLength = 128;

    struct RegistrationData
    {
        std::string srpUsername;
        std::array<uint8_t, SaltLength> salt{};
        std::string saltHex;
        std::string verifierHex;
    };

    struct ServerChallenge
    {
        uint32_t version = 1;
        uint32_t iterations = 1;
        std::string modulus;
        std::string generator;
        std::string hashFunction = "SHA-256";
        std::string username;
        std::string salt;
        std::string publicB;

        // Server-only ephemeral state. Never send or log these values.
        std::string privateB;
        std::string verifier;
    };

    struct EvidenceDiagnostics
    {
        std::string expectedM1;
        std::string fixedWidthEvidenceM1;
        std::string littleEndianEvidenceM1;
        std::string littleEndianUExpectedM1;
    };

    bool makeRegistrationData(
        const std::string& accountName,
        const std::string& password,
        RegistrationData& output);

    bool createServerChallenge(
        const std::string& accountName,
        const std::string& saltHex,
        const std::string& verifierHex,
        ServerChallenge& output);

    bool verifyClientEvidence(
        const ServerChallenge& challenge,
        const std::string& publicAHex,
        const std::string& clientEvidenceM1Hex,
        std::string& serverEvidenceM2Hex,
        EvidenceDiagnostics* diagnostics = nullptr);
}
