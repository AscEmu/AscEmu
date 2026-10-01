/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Patcher.hpp"
#include "BinaryTypes.hpp"
#include "PatternsWindows.hpp"
#include "PatternsMac.hpp"
#include "Patches.hpp"
#include "PatternsBattleNet.hpp"
#include "PatchesBattleNet.hpp"
#include "CertificateBundle.hpp"
#include "Helper.hpp"

#include "mpqlib/ClientVersion.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
    // Battle.net clients of the WoD and Legion profiles
    constexpr uint32_t WoDBuild = 21742;
    constexpr uint32_t LegionBuild = 26972;

    // text in a buffer of the pattern size, the rest cleared
    std::vector<uint8_t> paddedText(const char* _text, size_t _size)
    {
        std::vector<uint8_t> bytes(_size, 0);
        std::memcpy(bytes.data(), _text, std::min(std::strlen(_text), _size - 1));
        return bytes;
    }
}

static std::filesystem::path patchedNameForExe(const std::filesystem::path& _path)
{
    auto base = _path;
    const auto ext = base.extension().string();
    if (!ext.empty() && (ext == ".exe" || ext == ".EXE"))
    {
        base.replace_extension();
        return base.string() + "_AEPatched.exe";
    }
    return base.string() + "_AEPatched";
}

// 6.2.4 and 7.3.5: Battle.net host from the portal cvar, known SMSG_CONNECT_TO key, no self update and a
// certificate bundle that trusts the TLS certificate of our bnetserver
static int patchBattleNetClient(cp::Patcher& _patcher, uint32_t _build, const std::filesystem::path& _serverCertificate)
{
    namespace pattern = cp::patterns::bnet;
    namespace patch = cp::patches::bnet;

    const bool legion = _build == LegionBuild;
    std::cout << "AE Connection Patcher - " << (legion ? "Legion 7.3.5" : "WoD 6.2.4") << " client (build " << _build << ")\n";

    if (_serverCertificate.empty())
    {
        std::cout << "Usage: connection_patcher <Wow-64.exe> <bnetserver.cert.pem>\n";
        std::cout << "The certificate is the one of the bnetserver (PEM), the client will trust exactly this key.\n";
        return 1;
    }

    if (_patcher.type() != cp::BinaryType::Pe32 && _patcher.type() != cp::BinaryType::Pe64)
    {
        std::cerr << "Error: only Windows clients are supported for this build\n";
        return 1;
    }

    cp::CertificateBundle bundle(_serverCertificate);
    std::cout << "Server certificate key hash: " << bundle.publicKeyHash() << "\n";
    std::cout << "Press Enter to patch...\n";
    std::cin.get();

    // the 6.x patterns carry wildcards, the 7.x patterns are matched exactly
    const bool wildcards = !legion;
    bool complete = true;
    auto apply = [&](const char* _name, std::span<const uint8_t> _replacement, std::span<const uint8_t> _pattern)
    {
        const size_t count = _patcher.patchAll(_replacement, _pattern, wildcards);
        std::cout << "patching " << _name << ": " << count << " place(s)\n";
        if (count == 0)
            complete = false;
    };

    apply("portal", patch::Portal, pattern::Portal);
    apply("SMSG_CONNECT_TO modulus", patch::ConnectToModulus, pattern::ConnectToModulus);

    char versions[pattern::VersionsFile.size()] = {};
    std::snprintf(versions, sizeof(versions), patch::VersionsFileFormat, _build);
    apply("versions file", paddedText(versions, pattern::VersionsFile.size()), pattern::VersionsFile);

    std::filesystem::path bundlePath;
    if (!legion)
    {
        apply("certificate bundle file name", paddedText(patch::CertBundleFileName, pattern::CertBundleFileName.size()), pattern::CertBundleFileName);

        if (_patcher.type() == cp::BinaryType::Pe64)
        {
            apply("certificate bundle from local file", patch::windows::x64::CertBundleCascLocalFile, pattern::windows::x64::CertBundleCascLocalFile);
            apply("certificate bundle signature check", patch::windows::x64::CertBundleSignatureCheck, pattern::windows::x64::CertBundleSignatureCheck);
        }
        else
        {
            apply("certificate bundle from local file", patch::windows::x86::CertBundleCascLocalFile, pattern::windows::x86::CertBundleCascLocalFile);
            apply("certificate bundle signature check", patch::windows::x86::CertBundleSignatureCheck, pattern::windows::x86::CertBundleSignatureCheck);
        }

        // the client opens the bundle relative to its own directory
        bundlePath = _patcher.binaryPath().parent_path() / patch::CertBundleFileName;
    }
    else
    {
        apply("certificate bundle address", paddedText(patch::CertBundleUrl, pattern::CertBundleUrl.size()), pattern::CertBundleUrl);

        const auto signingModulus = bundle.createSigningKey();
        apply("certificate bundle signing key", signingModulus, pattern::CertSignatureModulus);

        const std::span<const uint8_t> launcherPattern(reinterpret_cast<const uint8_t*>(pattern::LauncherLoginParametersLocation), sizeof(pattern::LauncherLoginParametersLocation));
        apply("launcher login parameters", paddedText(patch::LauncherLoginParametersLocation, sizeof(pattern::LauncherLoginParametersLocation)), launcherPattern);

        // the client keeps the bundle in the Battle.net cache shared by all users
        const char* programData = std::getenv("ProgramData");
        bundlePath = std::filesystem::path(programData ? programData : "C:\\ProgramData") / "Blizzard Entertainment" / "Battle.net" / "Cache" / "web_cert_bundle";
    }

    if (!complete)
    {
        std::cerr << "Error: not every patch was found, the client stays unchanged\n";
        return 1;
    }

    if (legion)
    {
        // keep the bundle of the official client once
        std::error_code error;
        const std::filesystem::path backup = bundlePath.string() + ".backup";
        if (std::filesystem::exists(bundlePath, error) && !std::filesystem::exists(backup, error))
            std::filesystem::copy_file(bundlePath, backup, error);

        bundle.writeSigned(bundlePath);
    }
    else
    {
        bundle.writeUnsigned(bundlePath);
    }

    std::cout << "Certificate bundle written to " << bundlePath.string() << "\n";

    _patcher.setBinaryPath(patchedNameForExe(_patcher.binaryPath()));
    _patcher.finish();

    std::cout << "Patching done, writing " << _patcher.binaryPath().string() << "\n";
    std::cout << "Set the Battle.net host in WTF\\Config.wtf: SET portal \"<bnetserver host>\"\n";
    return 0;
}

int main(int argc, char** argv)
{
    if (argc < 2)
        return 0;

    // Battle.net clients are recognized by the build number of the binary
    try
    {
        auto patcher = cp::Patcher{ std::filesystem::path{ argv[1] } };
        const uint32_t build = cp::getBuildNumber(patcher.data());
        if (build == WoDBuild || build == LegionBuild)
            return patchBattleNetClient(patcher, build, argc > 2 ? std::filesystem::path{ argv[2] } : std::filesystem::path{});
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    if (argc != 2)
        return 0;

    // The patch patterns below only match Mist of Pandaria's client binary -
    // older clients (Classic through Cata) don't need this patch at all and
    // would just silently produce an unmodified "patched" copy if we let the
    // no-match-found patch() calls fall through, so detect the version up
    // front and refuse cleanly instead.
    const std::filesystem::path targetBinary{ argv[1] };
    auto clientRoot = targetBinary.parent_path();
    if (clientRoot.empty())
        clientRoot = ".";
    auto detected = mpqlib::detectClientVersion(clientRoot);
    if (!detected || *detected < mpqlib::ClientVersion::MistsOfPandaria)
    {
        std::cout << "Nothing to patch\n";
        return 0;
    }

    try
    {
        auto patcher = cp::Patcher{ targetBinary };

        std::cout << "AE Connection Patcher\n";
        std::cout << "Press Enter to patch...\n";
        std::cin.get();

        switch (patcher.type())
        {
            case cp::BinaryType::Pe32:
            {
                patcher.patch(cp::patches::windows::x86::Send,  cp::patterns::windows::x86::Send);
                patcher.patch(cp::patches::windows::x86::Email, cp::patterns::windows::x86::Email);
                patcher.patch(cp::patches::windows::x86::User,  cp::patterns::windows::x86::User);
                patcher.patch(cp::patches::windows::x86::RaF,   cp::patterns::windows::x86::RaF);
                patcher.patch(cp::patches::windows::x86::Rcv,   cp::patterns::windows::x86::Rcv);

                patcher.setBinaryPath(patchedNameForExe(patcher.binaryPath()));
                patcher.finish();
                break;
            }
            case cp::BinaryType::Pe64:
            {
                patcher.patch(cp::patches::windows::x64::Send,  cp::patterns::windows::x64::Send);
                patcher.patch(cp::patches::windows::x64::Email, cp::patterns::windows::x64::Email);
                patcher.patch(cp::patches::windows::x64::User,  cp::patterns::windows::x64::User);
                patcher.patch(cp::patches::windows::x64::RaF,   cp::patterns::windows::x64::RaF);
                patcher.patch(cp::patches::windows::x64::Rcv,   cp::patterns::windows::x64::Rcv);

                patcher.setBinaryPath(patchedNameForExe(patcher.binaryPath()));
                patcher.finish();
                break;
            }
            case cp::BinaryType::Mach32:
            {
                patcher.patch(cp::patches::mac::x86::Send,  cp::patterns::mac::x86::Send);
                patcher.patch(cp::patches::mac::x86::Email, cp::patterns::mac::x86::Email);
                patcher.patch(cp::patches::mac::x86::User,  cp::patterns::mac::x86::User);
                patcher.patch(cp::patches::mac::x86::RaF,   cp::patterns::mac::x86::RaF);
                patcher.patch(cp::patches::mac::x86::Rcv,   cp::patterns::mac::x86::Rcv);

                patcher.setBinaryPath(patcher.binaryPath().string() + " AEPatched");
                patcher.finish();
                break;
            }
            case cp::BinaryType::Mach64:
            {
                patcher.patch(cp::patches::mac::x64::Send,  cp::patterns::mac::x64::Send);
                patcher.patch(cp::patches::mac::x64::Email, cp::patterns::mac::x64::Email);
                patcher.patch(cp::patches::mac::x64::User,  cp::patterns::mac::x64::User);
                patcher.patch(cp::patches::mac::x64::RaF,   cp::patterns::mac::x64::RaF);
                patcher.patch(cp::patches::mac::x64::Rcv,   cp::patterns::mac::x64::Rcv);

                patcher.setBinaryPath(patcher.binaryPath().string() + " AEPatched");
                patcher.finish();
                break;
            }
            default:
                throw std::runtime_error("Binary type not supported");
        }

        std::cout << "Patching done.\n";
        std::cout << "Writing patched file...\n";
        std::cout << "Successfully created your patched binary.\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
