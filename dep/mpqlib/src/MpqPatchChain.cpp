/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/MpqPatchChain.hpp"

#include <unordered_set>

namespace mpqlib
{
    MpqPatchChain::MpqPatchChain(const std::string& basePath)
    {
        m_archives.emplace_back(basePath);
    }

    bool MpqPatchChain::addPatch(const std::string& patchPath)
    {
        MpqArchive archive(patchPath);
        if (!archive.isOpen())
            return false;

        m_archives.push_back(std::move(archive));
        return true;
    }

    bool MpqPatchChain::hasFile(std::string_view fileName) const
    {
        for (auto it = m_archives.rbegin(); it != m_archives.rend(); ++it)
        {
            if (it->findFile(fileName))
                return true;
        }

        return false;
    }

    bool MpqPatchChain::readFile(std::string_view fileName, std::vector<uint8_t>& out) const
    {
        for (auto it = m_archives.rbegin(); it != m_archives.rend(); ++it)
        {
            if (it->readFile(fileName, out))
                return true;
        }

        return false;
    }

    std::vector<std::string> MpqPatchChain::listFiles() const
    {
        std::unordered_set<std::string> unique;

        for (auto it = m_archives.rbegin(); it != m_archives.rend(); ++it)
            for (auto& name : it->listFiles())
                unique.insert(std::move(name));

        return std::vector<std::string>(unique.begin(), unique.end());
    }
}
