/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Patcher.hpp"
#include "BinaryTypes.hpp"
#include "PatternsWindows.hpp"
#include "PatternsMac.hpp"
#include "Patches.hpp"

#include <iostream>
#include <filesystem>

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

int main(int argc, char** argv)
{
    if (argc != 2)
        return 0;

    try
    {
        auto patcher = cp::Patcher{ std::filesystem::path{ argv[1] } };

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
