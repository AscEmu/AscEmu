/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <cstdint>

// byte sequences of the 6.2.4 (21742), 7.3.5 (26972) and 8.3.7 (35662) clients the patches are applied at
namespace cp::patterns::bnet
{
    // "<Version>" of the embedded manifest, the build number is the last component of the version behind it
    inline constexpr std::array<std::uint8_t, 9> BinaryVersion{ { '<', 'V', 'e', 'r', 's', 'i', 'o', 'n', '>' } };

    // suffix appended to the portal cvar, removed so "SET portal" names the Battle.net host directly
    inline constexpr std::array<std::uint8_t, 19> Portal{ { '.', 'a', 'c', 't', 'u', 'a', 'l', '.', 'b', 'a', 't', 't', 'l', 'e', '.', 'n', 'e', 't', 0x00 } };

    // start of the RSA modulus the client verifies SMSG_CONNECT_TO with
    inline constexpr std::array<std::uint8_t, 8> ConnectToModulus{ { 0x91, 0xD5, 0x9B, 0xB7, 0xD4, 0xE1, 0x83, 0xA5 } };

    // host of the versions file the client checks for updates
    inline constexpr std::array<std::uint8_t, 36> VersionsFile{ { '%', 's', '.', 'p', 'a', 't', 'c', 'h', '.', 'b', 'a', 't', 't', 'l', 'e', '.', 'n', 'e', 't', ':', '1', '1', '1', '9', '/', '%', 's', '/', 'v', 'e', 'r', 's', 'i', 'o', 'n', 's' } };

    // 6.2.4: file name of the certificate bundle inside the CASC storage
    inline constexpr std::array<std::uint8_t, 21> CertBundleFileName{ { 'c', 'a', '_', 'b', 'u', 'n', 'd', 'l', 'e', '.', 't', 'x', 't', '.', 's', 'i', 'g', 'n', 'e', 'd', 0x00 } };

    // 7.3.5: address the client refreshes the certificate bundle from
    inline constexpr std::array<std::uint8_t, 68> CertBundleUrl{ { 'h', 't', 't', 'p', ':', '/', '/', 'n', 'y', 'd', 'u', 's', '-', 'q', 'a', '.', 'w', 'e', 'b', '.',
        'b', 'l', 'i', 'z', 'z', 'a', 'r', 'd', '.', 'n', 'e', 't', '/', 'B', 'n', 'e', 't', '/', 'z', 'x', 'x', '/', 'c', 'l', 'i', 'e', 'n', 't', '/', 'b', 'g', 's', '-',
        'k', 'e', 'y', '-', 'f', 'i', 'n', 'g', 'e', 'r', 'p', 'r', 'i', 'n', 't' } };

    // 7.3.5: start of the RSA modulus the certificate bundle signature is verified with
    inline constexpr std::array<std::uint8_t, 8> CertSignatureModulus{ { 0x85, 0xF3, 0x7B, 0x14, 0x5A, 0x9C, 0x48, 0xF6 } };

    // 8.3.7: address the client refreshes the certificate bundle from
    inline constexpr char CertBundleUrl837[] = "http://nydus.battle.net/Bnet/zxx/client/bgs-key-fingerprint";

    // 8.3.7: the bundle signing modulus lies in front of its public exponent and the reversed "SIGN" tag,
    // which precede the signature salt
    inline constexpr std::array<std::uint8_t, 8> CertSignatureTrailer837{ { 0x01, 0x00, 0x01, 0x00, 'N', 'G', 'I', 'S' } };

    // 7.3.5: registry key the launcher stores its login parameters under (with the terminating zero)
    inline constexpr char LauncherLoginParametersLocation[] = R"(Software\Blizzard Entertainment\Battle.net\Launch Options\)";

    namespace windows
    {
        // 6.2.4: the bundle is read from the CASC storage and its signature is checked
        namespace x86
        {
            inline constexpr std::array<std::uint8_t, 11> CertBundleCascLocalFile{ { 0x6A, 0x00, 0x8D, 0x45, 0xFC, 0x50, 0x8D, 0x45, 0xF8, 0x50, 0x68 } };
            inline constexpr std::array<std::uint8_t, 10> CertBundleSignatureCheck{ { 0x59, 0x59, 0x84, 0xC0, 0x75, 0x08, 0x47, 0x83, 0xFF, 0x02 } };
        }

        namespace x64
        {
            inline constexpr std::array<std::uint8_t, 7> CertBundleCascLocalFile{ { 0x45, 0x33, 0xC9, 0x48, 0x89, 0x45, 0x90 } };
            inline constexpr std::array<std::uint8_t, 9> CertBundleSignatureCheck{ { 0x75, 0x19, 0x48, 0xFF, 0xC3, 0x48, 0x83, 0xFB, 0x02 } };
        }
    }
}
