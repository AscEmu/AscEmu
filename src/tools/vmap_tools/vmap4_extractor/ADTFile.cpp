/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "vmapexport.h"
#include "ADTFile.hpp"

#include "mpqlib/ChunkReader.hpp"

#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

#ifdef WIN32
#define snprintf _snprintf
#endif

char const* getPlainName(char const* fileName)
{
    if (const char* temp = strrchr(fileName, '\\'))
        fileName = temp + 1;
    return fileName;
}

char* getPlainName(char* fileName)
{
    if (char* temp = strrchr(fileName, '\\'))
        fileName = temp + 1;
    return fileName;
}

namespace
{
    // Classic/TBC/WotLK: assumes a fixed 3-character extension.
    void fixNameCaseLegacy(char* name, size_t len)
    {
        for (size_t i = 0; i < len - 3; i++)
        {
            if (i > 0 && name[i] >= 'A' && name[i] <= 'Z' && isalpha(name[i - 1]))
                name[i] |= 0x20;
            else if ((i == 0 || !isalpha(name[i - 1])) && name[i] >= 'a' && name[i] <= 'z')
                name[i] &= ~0x20;
        }

        // extension in lowercase
        for (size_t i = len - 3; i < len; i++)
            name[i] |= 0x20;
    }

    // Cata/Mop: scans backward from the end for the extension's '.' instead
    // of assuming a fixed length. Falls back to the legacy algorithm if the
    // name has no '.' at all, rather than walking off the front of the
    // buffer (a latent bug in the original Cata/Mop version).
    void fixNameCaseModern(char* name, size_t len)
    {
        char* ptr = name + len - 1;
        char* const begin = name;

        for (; ptr >= begin && *ptr != '.'; --ptr)
            *ptr |= 0x20;

        if (ptr < begin)
        {
            fixNameCaseLegacy(name, len);
            return;
        }

        for (; ptr >= begin; --ptr)
        {
            if (ptr > begin && *ptr >= 'A' && *ptr <= 'Z' && isalpha(*(ptr - 1)))
                *ptr |= 0x20;
            else if ((ptr == begin || !isalpha(*(ptr - 1))) && *ptr >= 'a' && *ptr <= 'z')
                *ptr &= ~0x20;
        }
    }
}

void fixNameCase(char* name, size_t len)
{
    // file data id names are spelled by the tool itself, they stay as they are
    if (FileDataIdFromName(std::string_view(name, len)))
        return;

    if (IsLegacyVmapArchiveLayout())
        fixNameCaseLegacy(name, len);
    else
        fixNameCaseModern(name, len);
}

void fixNameSpaces(char* name, size_t len)
{
    for (size_t i = 0; i < len - 3; i++)
    {
        if (name[i] == ' ')
            name[i] = '_';
    }
}

char* getExtension(char* fileName)
{
    if (char* temp = strrchr(fileName, '.'))
        return temp;
    return nullptr;
}

ADTFile::ADTFile(std::string const& filename) :
    m_wmoCount(0), m_modelCount(0), m_wmoInstanceNames(nullptr), m_modelInstanceNames(nullptr),
    m_adt(OpenWorldFile(filename, IsLegacyVmapArchiveLayout())), m_adtFilename(filename)
{
}

namespace
{
    // MMDX/MWMO carry a run of back-to-back, null-terminated names; copy the
    // chunk payload out once so fixNameCase/fixNameSpaces can mutate it in place.
    std::vector<char> copyPayload(std::span<const std::byte> payload)
    {
        std::vector<char> buf(payload.size());
        std::memcpy(buf.data(), payload.data(), buf.size());
        return buf;
    }
}

bool ADTFile::init(uint32_t mapNum, uint32_t tileX, uint32_t tileY)
{
    if (m_adt->isEof())
        return false;

    // The tile coordinates used to be clipped off the file name here; the
    // name is not needed afterwards, and a file data id name has none.
    if (size_t const adtPos = m_adtFilename.find(".adt"); adtPos != std::string::npos)
    {
        m_adtFilename.erase(adtPos, 4);
        std::string tempMapNumber = m_adtFilename.substr(m_adtFilename.length() - 6, 6);
        std::string xMap = tempMapNumber.substr(tempMapNumber.find('_') + 1, (tempMapNumber.find_last_of('_') - 1) - (tempMapNumber.find('_')));
        std::string yMap = tempMapNumber.substr(tempMapNumber.find_last_of('_') + 1, tempMapNumber.length() - tempMapNumber.find_last_of('_'));
        m_adtFilename.erase((m_adtFilename.length() - xMap.length() - yMap.length() - 2), (xMap.length() + yMap.length() + 2));
    }

    std::string dirName = std::string(szWorkDirWmo) + "/dir_bin";
    FILE* dirFile = fopen(dirName.c_str(), "ab");
    if (!dirFile)
    {
        printf("Can't open dirfile!'%s'\n", dirName.c_str());
        return false;
    }

    mpqlib::ChunkReader chunks(m_adt->reader());
    mpqlib::Chunk chunk;
    while (chunks.next(chunk))
    {
        if (chunk.tag == "MMDX")
        {
            if (chunk.payload.empty())
                continue;

            std::vector<char> buf = copyPayload(chunk.payload);
            char* p = buf.data();
            int t = 0;
            m_modelInstanceNames = new std::string[buf.size()];
            while (p < buf.data() + buf.size())
            {
                // Legacy fixes case on the whole path (dir included) before
                // capturing it, so ExtractSingleModel opens the normalized
                // path. Modern captures the raw MMDX string first and only
                // case-fixes the plain-name portion, so ExtractSingleModel
                // opens the byte-for-byte original path (MPQ lookups are
                // case-insensitive either way, so this is believed benign,
                // but it's a real difference worth preserving exactly).
                if (IsLegacyVmapArchiveLayout())
                {
                    fixNameCase(p, strlen(p));
                    char* s = getPlainName(p);
                    fixNameSpaces(s, strlen(s));

                    m_modelInstanceNames[t++] = s;

                    std::string path(p);
                    ExtractSingleModel(path);
                }
                else
                {
                    std::string path(p);

                    char* s = getPlainName(p);
                    fixNameCase(s, strlen(s));
                    fixNameSpaces(s, strlen(s));

                    m_modelInstanceNames[t++] = s;

                    ExtractSingleModel(path);
                }

                p = p + strlen(p) + 1;
            }
        }
        else if (chunk.tag == "MWMO")
        {
            if (chunk.payload.empty())
                continue;

            std::vector<char> buf = copyPayload(chunk.payload);
            char* p = buf.data();
            int q = 0;
            m_wmoInstanceNames = new std::string[buf.size()];
            while (p < buf.data() + buf.size())
            {
                // The MPQ clients extracted every WMO of the archive up front;
                // the CASC clients have no file list, so the map objects are
                // extracted as they are met. Done for all clients - an
                // already extracted WMO is just skipped.
                std::string path(p);
                char* s = getPlainName(p);
                fixNameCase(s, strlen(s));
                fixNameSpaces(s, strlen(s));
                p += strlen(p) + 1;
                m_wmoInstanceNames[q++] = s;
                ExtractSingleWmo(path);
            }
        }
        else if (chunk.tag == "MDDF")
        {
            if (chunk.payload.empty())
                continue;

            auto const payloadOffset = static_cast<size_t>(chunk.payload.data() - m_adt->reader().bytes().data());
            m_adt->seek(static_cast<int>(payloadOffset));

            m_modelCount = static_cast<int>(chunk.payload.size()) / static_cast<int>(sizeof(AdtDoodadDef));
            for (int i = 0; i < m_modelCount; ++i)
            {
                AdtDoodadDef def{};
                m_adt->read(&def, sizeof(def));

                std::string name;
                if (def.flags & kDoodadDefFileDataId)
                {
                    // 8.1 and later: the model comes by file data id
                    name = FileDataIdName(def.id);
                    std::string path = name;
                    if (!ExtractSingleModel(path))
                        continue;
                }
                else if (m_modelInstanceNames != nullptr)
                    name = m_modelInstanceNames[def.id];
                else
                    continue;

                ModelInstance inst(def, name.c_str(), mapNum, tileX, tileY, dirFile);
            }
            delete[] m_modelInstanceNames;
            m_modelInstanceNames = nullptr;
        }
        else if (chunk.tag == "MODF")
        {
            if (chunk.payload.empty())
                continue;

            auto const payloadOffset = static_cast<size_t>(chunk.payload.data() - m_adt->reader().bytes().data());
            m_adt->seek(static_cast<int>(payloadOffset));

            m_wmoCount = static_cast<int>(chunk.payload.size()) / static_cast<int>(sizeof(AdtMapObjDef));
            for (int i = 0; i < m_wmoCount; ++i)
            {
                AdtMapObjDef def{};
                m_adt->read(&def, sizeof(def));

                std::string name;
                if (def.flags & kMapObjDefFileDataId)
                {
                    name = FileDataIdName(def.id);
                    std::string path = name;
                    ExtractSingleWmo(path);
                }
                else if (m_wmoInstanceNames != nullptr)
                    name = m_wmoInstanceNames[def.id];
                else
                    continue;

                WMOInstance inst(def, name.c_str(), mapNum, tileX, tileY, dirFile);
            }
            delete[] m_wmoInstanceNames;
            m_wmoInstanceNames = nullptr;
        }
    }

    m_adt->close();
    fclose(dirFile);
    return true;
}

ADTFile::~ADTFile()
{
    m_adt->close();
}
