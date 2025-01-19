/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _MMAP_COMMON_H
#define _MMAP_COMMON_H

#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <iostream>

enum NavTerrain
{
    NAV_EMPTY   = 0x00,
    NAV_GROUND  = 0x01,
    NAV_MAGMA   = 0x02,
    NAV_SLIME   = 0x04,
    NAV_WATER   = 0x08,
    NAV_UNUSED1 = 0x10,
    NAV_UNUSED2 = 0x20,
    NAV_UNUSED3 = 0x40,
    NAV_UNUSED4 = 0x80
    // we only have 8 bits
};

namespace MMAP
{
    inline bool matchWildcardFilter(const std::string& filter, const std::string& str)
    {
        // Convert the wildcard pattern to a regex pattern
        std::string regexPattern = "^" + std::regex_replace(filter, std::regex(R"(\*)"), ".*") + "$";
        try
        {
            std::regex wildcardRegex(regexPattern);
            return std::regex_match(str, wildcardRegex);
        }
        catch (const std::regex_error& e)
        {
            std::cerr << "Regex error: " << e.what() << std::endl;
            return false;
        }
    }

    enum ListFilesResult
    {
        LISTFILE_DIRECTORY_NOT_FOUND = 0,
        LISTFILE_OK = 1
    };

    inline ListFilesResult getDirContents(std::vector<std::string> &fileList, const std::string& dirpath = ".", const std::string& filter = "*")
    {
        std::filesystem::path dir(dirpath);

        if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir))
            return LISTFILE_DIRECTORY_NOT_FOUND;

        try
        {
            for (const auto& entry : std::filesystem::directory_iterator(dir))
            {
                if (entry.is_regular_file())
                {
                    const std::string filename = entry.path().filename().string();
                    if (matchWildcardFilter(filter, filename))
                        fileList.push_back(filename);
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
            return LISTFILE_DIRECTORY_NOT_FOUND;
        }

        return LISTFILE_OK;
    }
}

#endif
