/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Combines a base archive with any number of patch archives into one logical
// view, mirroring StormLib's SFileOpenArchive()+SFileOpenPatchArchive(...)
// chain: a file present in a later-added archive shadows the same-named file
// in every earlier one. This is the actual functionality AscEmu's Cata/Mop
// tools depend on StormLib for - libmpq (and, before this, mpqlib::MpqArchive)
// only ever understood a single, standalone archive.

#pragma once

#include "mpqlib/MpqArchive.hpp"

#include <string>
#include <vector>

namespace mpqlib
{
    class MpqPatchChain
    {
    public:
        explicit MpqPatchChain(const std::string& basePath);

        bool isOpen() const { return !m_archives.empty() && m_archives.front().isOpen(); }

        // Adds a patch archive on top of everything opened so far. Returns
        // false (chain is left unmodified) if the archive can't be opened.
        bool addPatch(const std::string& patchPath);

        bool hasFile(std::string_view fileName) const;
        bool readFile(std::string_view fileName, std::vector<uint8_t>& out) const;

        // Union of every archive's (listfile), highest-priority copy wins,
        // de-duplicated and unordered.
        std::vector<std::string> listFiles() const;

    private:
        std::vector<MpqArchive> m_archives;
    };
}
