/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Minimal manual smoke test: opens a base archive plus any number of patch
// archives, then tries to read one file from the resulting chain.
//
// Usage: mpqlib_test <base.MPQ> [patch1.MPQ patch2.MPQ ...] -- <fileInArchive>

#include "mpqlib/MpqPatchChain.hpp"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        std::printf("usage: %s <base.MPQ> [patch.MPQ ...] -- <fileInArchive>\n", argv[0]);
        return 1;
    }

    std::vector<std::string> args(argv + 1, argv + argc);

    const auto separator = std::find(args.begin(), args.end(), "--");
    if (separator == args.end() || (separator + 1) == args.end())
    {
        std::printf("missing '--' before the target file name\n");
        return 1;
    }

    const std::string fileName = *(separator + 1);
    const std::vector<std::string> archivePaths(args.begin(), separator);

    if (archivePaths.empty())
    {
        std::printf("no archives given\n");
        return 1;
    }

    std::printf("opening base archive: %s\n", archivePaths.front().c_str());
    mpqlib::MpqPatchChain chain(archivePaths.front());

    if (!chain.isOpen())
    {
        std::printf("FAILED to open base archive\n");
        return 1;
    }

    for (size_t i = 1; i < archivePaths.size(); ++i)
    {
        std::printf("adding patch archive: %s ... ", archivePaths[i].c_str());
        if (chain.addPatch(archivePaths[i]))
            std::printf("ok\n");
        else
            std::printf("FAILED (skipped)\n");
    }

    std::printf("looking up '(listfile)' ... ");
    if (chain.hasFile("(listfile)"))
    {
        std::printf("found\n");
        const auto listing = chain.listFiles();
        std::printf("(listfile) contains %zu entries\n", listing.size());
        for (size_t i = 0; i < listing.size() && i < 10; ++i)
            std::printf("  %s\n", listing[i].c_str());
    }
    else
    {
        std::printf("NOT FOUND (hash lookup itself may be broken)\n");
    }

    std::printf("looking up '%s' ... ", fileName.c_str());
    if (!chain.hasFile(fileName))
    {
        std::printf("NOT FOUND\n");
        return 1;
    }
    std::printf("found\n");

    std::vector<uint8_t> data;
    if (!chain.readFile(fileName, data))
    {
        std::printf("FAILED to read file\n");
        return 1;
    }

    std::printf("read %zu bytes successfully\n", data.size());

    return 0;
}
