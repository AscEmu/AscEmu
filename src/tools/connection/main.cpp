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
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
    // Battle.net clients of the WoD and Legion profiles
    constexpr uint32_t WoDBuild = 21742;
    constexpr uint32_t LegionBuild = 26972;
    constexpr uint32_t BfABuild = 35662;

    // text in a buffer of the pattern size, the rest cleared
    std::vector<uint8_t> paddedText(const char* _text, size_t _size)
    {
        std::vector<uint8_t> bytes(_size, 0);
        std::memcpy(bytes.data(), _text, std::min(std::strlen(_text), _size - 1));
        return bytes;
    }

    bool contains(const std::vector<uint8_t>& _data, std::span<const uint8_t> _bytes)
    {
        return std::search(_data.begin(), _data.end(), _bytes.begin(), _bytes.end()) != _data.end();
    }

    // offset of a text with its closing zero when the binary holds it exactly once
    std::optional<size_t> findUniqueText(const std::vector<uint8_t>& _data, std::string_view _text)
    {
        const auto* begin = reinterpret_cast<const uint8_t*>(_text.data());
        const auto* end = begin + _text.size() + 1;

        const auto first = std::search(_data.begin(), _data.end(), begin, end);
        if (first == _data.end() || std::search(first + 1, _data.end(), begin, end) != _data.end())
            return std::nullopt;

        return static_cast<size_t>(first - _data.begin());
    }

    // start of the zero terminated text that _offset lies in
    size_t textStart(const std::vector<uint8_t>& _data, size_t _offset)
    {
        while (_offset > 0 && _data[_offset - 1] != 0)
            --_offset;
        return _offset;
    }

    bool startsWith(const std::vector<uint8_t>& _data, size_t _offset, std::string_view _text)
    {
        return _offset + _text.size() <= _data.size() && std::memcmp(_data.data() + _offset, _text.data(), _text.size()) == 0;
    }

    // A 7.3.5 client that was patched before no longer holds the original values. These functions find the
    // places through unchanged neighbours instead: the offset to patch, or nothing when the surroundings differ.
    namespace patchedBefore
    {
        // versions address: the text in front of the cdns address, behind its "http://"
        std::optional<size_t> versionsFile(const std::vector<uint8_t>& _data, size_t _size)
        {
            constexpr std::string_view scheme = "http://";

            const auto cdns = findUniqueText(_data, "http://%s.patch.battle.net:1119/%s/cdns");
            if (!cdns)
                return std::nullopt;

            // closing zeros between the two texts
            size_t end = *cdns;
            while (end > 0 && _data[end - 1] == 0)
                --end;

            if (end == 0 || end == *cdns)
                return std::nullopt;

            const size_t start = textStart(_data, end - 1);
            if (!startsWith(_data, start, scheme) || end - start != scheme.size() + _size)
                return std::nullopt;

            return start + scheme.size();
        }

        // 6.2.4 bundle file: the text that ends with the bundle file name
        std::optional<size_t> certBundleFileName(const std::vector<uint8_t>& _data, size_t _size)
        {
            constexpr std::string_view suffix = "_bundle.txt";

            const auto end = findUniqueText(_data, suffix);
            if (!end)
                return std::nullopt;

            // the new name is written with its padding: only zeros may follow the old one in that range
            const size_t start = textStart(_data, *end);
            const size_t oldEnd = *end + suffix.size();
            if (start + _size > _data.size() || oldEnd - start >= _size)
                return std::nullopt;

            for (size_t i = oldEnd; i < start + _size; ++i)
            {
                if (_data[i] != 0)
                    return std::nullopt;
            }

            return start;
        }

        // code patch that is in place already: its bytes followed by the unchanged rest of the pattern
        bool codePatchApplied(const std::vector<uint8_t>& _data, std::span<const uint8_t> _replacement, std::span<const uint8_t> _pattern)
        {
            if (_replacement.size() > _pattern.size())
                return false;

            std::vector<uint8_t> patched(_pattern.begin(), _pattern.end());
            std::copy(_replacement.begin(), _replacement.end(), patched.begin());
            return contains(_data, patched);
        }

        // bundle address: the text that ends with the fingerprint path
        std::optional<size_t> certBundleUrl(const std::vector<uint8_t>& _data, size_t _size)
        {
            constexpr std::string_view path = "/client/bgs-key-fingerprint";

            const auto end = findUniqueText(_data, path);
            if (!end)
                return std::nullopt;

            const size_t start = textStart(_data, *end);
            if (!startsWith(_data, start, "http") || *end + path.size() - start != _size)
                return std::nullopt;

            return start;
        }

        // bundle signing key: the 256 bytes in front of the signature salt, behind two public exponents
        std::optional<size_t> certSignatureModulus(const std::vector<uint8_t>& _data)
        {
            constexpr size_t modulusSize = 256;
            constexpr uint8_t exponents[] = { 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00 };

            const auto salt = findUniqueText(_data, "Blizzard Certificate Bundle");
            if (!salt || *salt < modulusSize + sizeof(exponents))
                return std::nullopt;

            const size_t start = *salt - modulusSize;
            if (std::memcmp(_data.data() + start - sizeof(exponents), exponents, sizeof(exponents)) != 0)
                return std::nullopt;

            return start;
        }

        // 8.3.7 versions address: the text that ends with "/versions" and its zero padding up to the next text
        std::optional<std::pair<size_t, size_t>> versionsField837(const std::vector<uint8_t>& _data)
        {
            const auto end = findUniqueText(_data, "/versions");
            if (!end)
                return std::nullopt;

            const size_t start = textStart(_data, *end);
            if (!startsWith(_data, start, "http://"))
                return std::nullopt;

            size_t next = *end + std::strlen("/versions") + 1;
            while (next < _data.size() && _data[next] == 0)
                ++next;

            return std::make_pair(start, next - start);
        }

        // 8.3.7 bundle signing key: the 256 bytes in front of the public exponent and the tag that precede the salt
        std::optional<size_t> certSignatureModulus837(const std::vector<uint8_t>& _data)
        {
            constexpr size_t modulusSize = 256;
            const auto& trailer = cp::patterns::bnet::CertSignatureTrailer837;

            const auto salt = findUniqueText(_data, "Blizzard Certificate Bundle");
            if (!salt || *salt < modulusSize + trailer.size())
                return std::nullopt;

            if (std::memcmp(_data.data() + *salt - trailer.size(), trailer.data(), trailer.size()) != 0)
                return std::nullopt;

            return *salt - trailer.size() - modulusSize;
        }

        // registry key of the launcher parameters: the text that ends with the launch options key
        std::optional<size_t> launcherLoginParameters(const std::vector<uint8_t>& _data, size_t _size)
        {
            constexpr std::string_view key = R"(\Battle.net\Launch Options\)";

            const auto end = findUniqueText(_data, key);
            if (!end)
                return std::nullopt;

            const size_t start = textStart(_data, *end);
            if (!startsWith(_data, start, R"(Software\)") || *end + key.size() + 1 - start != _size)
                return std::nullopt;

            return start;
        }
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

// 6.2.4, 7.3.5 and 8.3.7: Battle.net host from the portal cvar, known SMSG_CONNECT_TO key, no self update and a
// certificate bundle that trusts the TLS certificate of our bnetserver
static int patchBattleNetClient(cp::Patcher& _patcher, uint32_t _build, const std::filesystem::path& _serverCertificate)
{
    namespace pattern = cp::patterns::bnet;
    namespace patch = cp::patches::bnet;

    const bool legion = _build == LegionBuild;
    const bool bfa = _build == BfABuild;
    // 7.3.5 and 8.3.7 keep the bundle in the Battle.net cache and verify its signature with a key of the binary
    const bool cachedBundle = legion || bfa;
    std::cout << "AE Connection Patcher - " << (bfa ? "BfA 8.3.7" : legion ? "Legion 7.3.5" : "WoD 6.2.4") << " client (build " << _build << ")\n";

    if (_serverCertificate.empty())
    {
        std::cout << "Usage: connection_patcher <Wow-64.exe> <bnetserver.cert.pem>\n";
        std::cout << "The certificate file is the one of the bnetserver (PEM, with its chain), the client will trust exactly its key.\n";
        return 1;
    }

    if (_patcher.type() != cp::BinaryType::Pe32 && _patcher.type() != cp::BinaryType::Pe64)
    {
        std::cerr << "Error: only Windows clients are supported for this build\n";
        return 1;
    }

    cp::CertificateBundle bundle(_serverCertificate);
    std::cout << "Server certificate key hash: " << bundle.publicKeyHash() << "\n";

    // one signing key per server: 7.x and 8.x clients share the cached bundle in the Battle.net cache
    const std::filesystem::path signingKeyFile = _serverCertificate.parent_path() / "web_cert_bundle.key.pem";
    if (cachedBundle)
        std::cout << "Bundle signing key: " << signingKeyFile.string() << "\n";
    std::cout << "Press Enter to patch...\n";
    std::cin.get();

    // the 6.x patterns carry wildcards, the 7.x and 8.x patterns are matched exactly
    const bool wildcards = !cachedBundle;
    bool complete = true;
    auto apply = [&](const char* _name, std::span<const uint8_t> _replacement, std::span<const uint8_t> _pattern, std::optional<size_t> _knownOffset = std::nullopt)
    {
        size_t count = _patcher.patchAll(_replacement, _pattern, wildcards);
        if (count == 0 && _knownOffset && _patcher.patchAt(*_knownOffset, _replacement))
            count = 1;

        std::cout << "patching " << _name << ": " << count << " place(s)\n";
        if (count == 0)
            complete = false;
    };

    // the original SMSG_CONNECT_TO modulus is gone when another patcher worked on this client before
    const bool wasPatchedBefore = !contains(_patcher.data(), pattern::ConnectToModulus);
    if (wasPatchedBefore)
        std::cout << "This client was patched before, changed values are located through their surroundings\n";

    // a prepared client may carry another suffix there; it is only appended to a portal value without a
    // dot, so a host name or an IP address works unchanged
    if (wasPatchedBefore && !contains(_patcher.data(), pattern::Portal))
        std::cout << "patching portal: already removed\n";
    else
        apply("portal", patch::Portal, pattern::Portal);

    if (wasPatchedBefore && contains(_patcher.data(), patch::ConnectToModulus))
        std::cout << "patching SMSG_CONNECT_TO modulus: already set\n";
    else
        apply("SMSG_CONNECT_TO modulus", patch::ConnectToModulus, pattern::ConnectToModulus);

    if (bfa)
    {
        // 8.3.7: one text with the scheme, located through its path because other patchers rewrite it
        const auto field = patchedBefore::versionsField837(_patcher.data());
        char versions[64] = {};
        std::snprintf(versions, sizeof(versions), patch::VersionsFileFormat837, _build);
        const bool patched = field && field->second > std::strlen(versions) && _patcher.patchAt(field->first, paddedText(versions, field->second));
        std::cout << "patching versions file: " << (patched ? 1 : 0) << " place(s)\n";
        if (!patched)
            complete = false;
    }
    else
    {
        char versions[pattern::VersionsFile.size()] = {};
        std::snprintf(versions, sizeof(versions), patch::VersionsFileFormat, _build);
        apply("versions file", paddedText(versions, pattern::VersionsFile.size()), pattern::VersionsFile,
            wasPatchedBefore ? patchedBefore::versionsFile(_patcher.data(), pattern::VersionsFile.size()) : std::nullopt);
    }

    std::filesystem::path bundlePath;
    if (!cachedBundle)
    {
        apply("certificate bundle file name", paddedText(patch::CertBundleFileName, pattern::CertBundleFileName.size()), pattern::CertBundleFileName,
            wasPatchedBefore ? patchedBefore::certBundleFileName(_patcher.data(), pattern::CertBundleFileName.size()) : std::nullopt);

        auto applyCode = [&](const char* _name, std::span<const uint8_t> _replacement, std::span<const uint8_t> _pattern)
        {
            if (wasPatchedBefore && patchedBefore::codePatchApplied(_patcher.data(), _replacement, _pattern))
                std::cout << "patching " << _name << ": already applied\n";
            else
                apply(_name, _replacement, _pattern);
        };

        if (_patcher.type() == cp::BinaryType::Pe64)
        {
            applyCode("certificate bundle from local file", patch::windows::x64::CertBundleCascLocalFile, pattern::windows::x64::CertBundleCascLocalFile);
            applyCode("certificate bundle signature check", patch::windows::x64::CertBundleSignatureCheck, pattern::windows::x64::CertBundleSignatureCheck);
        }
        else
        {
            applyCode("certificate bundle from local file", patch::windows::x86::CertBundleCascLocalFile, pattern::windows::x86::CertBundleCascLocalFile);
            applyCode("certificate bundle signature check", patch::windows::x86::CertBundleSignatureCheck, pattern::windows::x86::CertBundleSignatureCheck);
        }

        // the client opens the bundle relative to its own directory
        bundlePath = _patcher.binaryPath().parent_path() / patch::CertBundleFileName;
    }
    else if (bfa)
    {
        const size_t urlSize = std::strlen(pattern::CertBundleUrl837);
        const std::span<const uint8_t> urlPattern(reinterpret_cast<const uint8_t*>(pattern::CertBundleUrl837), urlSize);
        apply("certificate bundle address", paddedText(patch::CertBundleUrl, urlSize + 1), urlPattern,
            wasPatchedBefore ? patchedBefore::certBundleUrl(_patcher.data(), urlSize) : std::nullopt);

        // a client that already carries the known SMSG_CONNECT_TO key was prepared by another patcher: such
        // clients do not verify the bundle, and the signing key and launcher patches make them crash or close
        const bool preparedClient = wasPatchedBefore && contains(_patcher.data(), patch::ConnectToModulus);
        if (preparedClient)
        {
            std::cout << "patching certificate bundle signing key: skipped, prepared client\n";
            std::cout << "patching launcher login parameters: skipped, prepared client\n";

            // the cached bundle is still written for the other clients of this server
            bundle.createSigningKey(signingKeyFile);
        }
        else
        {
            // the key is located through its surroundings, the original one is unknown
            const auto signingModulus = bundle.createSigningKey(signingKeyFile);
            const auto modulusOffset = patchedBefore::certSignatureModulus837(_patcher.data());
            const bool patched = modulusOffset && _patcher.patchAt(*modulusOffset, signingModulus);
            std::cout << "patching certificate bundle signing key: " << (patched ? 1 : 0) << " place(s)\n";
            if (!patched)
                complete = false;

            const std::span<const uint8_t> launcherPattern(reinterpret_cast<const uint8_t*>(pattern::LauncherLoginParametersLocation), sizeof(pattern::LauncherLoginParametersLocation));
            apply("launcher login parameters", paddedText(patch::LauncherLoginParametersLocation, sizeof(pattern::LauncherLoginParametersLocation)), launcherPattern,
                wasPatchedBefore ? patchedBefore::launcherLoginParameters(_patcher.data(), sizeof(pattern::LauncherLoginParametersLocation)) : std::nullopt);
        }

        const char* programData = std::getenv("ProgramData");
        bundlePath = std::filesystem::path(programData ? programData : "C:\\ProgramData") / "Blizzard Entertainment" / "Battle.net" / "Cache" / "web_cert_bundle";
    }
    else
    {
        apply("certificate bundle address", paddedText(patch::CertBundleUrl, pattern::CertBundleUrl.size()), pattern::CertBundleUrl,
            wasPatchedBefore ? patchedBefore::certBundleUrl(_patcher.data(), pattern::CertBundleUrl.size()) : std::nullopt);

        const auto signingModulus = bundle.createSigningKey(signingKeyFile);
        apply("certificate bundle signing key", signingModulus, pattern::CertSignatureModulus,
            wasPatchedBefore ? patchedBefore::certSignatureModulus(_patcher.data()) : std::nullopt);

        const std::span<const uint8_t> launcherPattern(reinterpret_cast<const uint8_t*>(pattern::LauncherLoginParametersLocation), sizeof(pattern::LauncherLoginParametersLocation));
        apply("launcher login parameters", paddedText(patch::LauncherLoginParametersLocation, sizeof(pattern::LauncherLoginParametersLocation)), launcherPattern,
            wasPatchedBefore ? patchedBefore::launcherLoginParameters(_patcher.data(), sizeof(pattern::LauncherLoginParametersLocation)) : std::nullopt);

        // the client keeps the bundle in the Battle.net cache shared by all users
        const char* programData = std::getenv("ProgramData");
        bundlePath = std::filesystem::path(programData ? programData : "C:\\ProgramData") / "Blizzard Entertainment" / "Battle.net" / "Cache" / "web_cert_bundle";
    }

    if (!complete)
    {
        std::cerr << "Error: not every patch was found, the client stays unchanged\n";
        return 1;
    }

    if (cachedBundle)
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
        if (build == WoDBuild || build == LegionBuild || build == BfABuild)
            return patchBattleNetClient(patcher, build, argc > 2 ? std::filesystem::path{ argv[2] } : std::filesystem::path{});

        // later Battle.net clients are started through their own launcher instead of a patched binary
        if (build >= WoDBuild)
        {
            std::cerr << "Error: Battle.net client build " << build << " is not supported by this patcher" << std::endl;
            return 1;
        }
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
