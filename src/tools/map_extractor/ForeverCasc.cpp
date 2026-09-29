/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ForeverCasc.hpp"
#include "ForeverDb2Files.hpp"

#include <CascLib.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <limits>
#include <sstream>

namespace MapExtractor::Forever
{
    namespace
    {
        std::vector<std::string> splitPipe(std::string const& line)
        {
            std::vector<std::string> result;
            std::string field;
            std::istringstream in(line);
            while (std::getline(in, field, '|'))
                result.push_back(field);
            return result;
        }

        std::string baseHeaderName(std::string field)
        {
            if (size_t const bang = field.find('!'); bang != std::string::npos)
                field.resize(bang);
            return field;
        }

        uint32_t parseBuild(std::string const& version)
        {
            size_t const dot = version.rfind('.');
            std::string const buildText = dot == std::string::npos ? version : version.substr(dot + 1);
            try
            {
                unsigned long const value = std::stoul(buildText);
                return value <= std::numeric_limits<uint32_t>::max() ? static_cast<uint32_t>(value) : 0;
            }
            catch (...)
            {
                return 0;
            }
        }

        std::optional<std::filesystem::path> findBuildInfo(std::filesystem::path path)
        {
            std::error_code ec;
            if (!std::filesystem::is_directory(path, ec))
                path = path.parent_path();

            for (int i = 0; i < 5 && !path.empty(); ++i)
            {
                auto const buildInfo = path / ".build.info";
                if (std::filesystem::is_regular_file(buildInfo, ec))
                    return buildInfo;
                path = path.parent_path();
            }
            return std::nullopt;
        }
    }

    namespace
    {
        bool readOpenCascFile(HANDLE file, std::vector<uint8_t>& data)
        {
            ULONGLONG fileSize = 0;
            if (!CascGetFileSize64(file, &fileSize) || fileSize > static_cast<ULONGLONG>(std::numeric_limits<size_t>::max()))
                return false;

            data.resize(static_cast<size_t>(fileSize));
            size_t offset = 0;
            while (offset < data.size())
            {
                DWORD const requested = static_cast<DWORD>(std::min<size_t>(data.size() - offset, 0x40000000u));
                DWORD read = 0;
                if (!CascReadFile(file, data.data() + offset, requested, &read) || !read)
                {
                    data.clear();
                    return false;
                }
                offset += read;
            }
            return true;
        }

        std::array<DWORD, 5> cascLocales(uint32_t localeMask)
        {
            return {
                static_cast<DWORD>(localeMask),
                static_cast<DWORD>(CASC_LOCALE_NONE),
                static_cast<DWORD>(CASC_LOCALE_ENUS | CASC_LOCALE_ENGB),
                static_cast<DWORD>(CASC_LOCALE_DEDE),
                static_cast<DWORD>(CASC_LOCALE_ALL_WOW)
            };
        }
    }

    CascStorage::~CascStorage()
    {
        close();
    }

    bool CascStorage::open(ClientBuildInfo const& info, uint32_t localeMask, std::string& error)
    {
        close();

        CASC_OPEN_STORAGE_ARGS args{};
        args.Size = sizeof(args);
        // Match the proven Trinity-style local CASC opening semantics:
        // open from the WoW install root, select the product, and do not
        // pre-filter ROOT entries by locale.
        std::string const path = info.storageRoot.string();
        args.szLocalPath = path.c_str();
        args.szCodeName = info.product.empty() ? nullptr : info.product.c_str();
        args.dwLocaleMask = CASC_LOCALE_NONE;

        if (!CascOpenStorageEx(nullptr, &args, false, &m_handle))
        {
            error = "CascOpenStorageEx failed (error " + std::to_string(GetCascError()) + ")";
            m_handle = nullptr;
            return false;
        }

        CASC_STORAGE_PRODUCT product{};
        size_t needed = 0;
        if (!CascGetStorageInfo(m_handle, CascStorageProduct, &product, sizeof(product), &needed))
        {
            error = "CascGetStorageInfo(CascStorageProduct) failed";
            close();
            return false;
        }

        m_build = product.BuildNumber;
        m_product = product.szCodeName;
        return true;
    }

    void CascStorage::close()
    {
        if (m_handle)
        {
            CascCloseStorage(m_handle);
            m_handle = nullptr;
        }
        m_build = 0;
        m_product.clear();
    }

    bool CascStorage::readFile(uint32_t fileDataId, std::vector<uint8_t>& data, uint32_t localeMask) const
    {
        data.clear();
        if (!m_handle)
            return false;

        HANDLE file = nullptr;
        if (!CascOpenFile(m_handle, CASC_FILE_DATA_ID(fileDataId), CASC_LOCALE_NONE,
            CASC_OPEN_BY_FILEID | CASC_OVERCOME_ENCRYPTED, &file))
            file = nullptr;
        if (!file)
            return false;

        bool const ok = readOpenCascFile(file, data);
        CascCloseFile(file);
        return ok;
    }

    bool CascStorage::extractFile(uint32_t fileDataId, std::filesystem::path const& destination, uint32_t localeMask) const
    {
        std::vector<uint8_t> data;
        if (!readFile(fileDataId, data, localeMask))
            return false;

        std::error_code ec;
        std::filesystem::create_directories(destination.parent_path(), ec);
        std::ofstream out(destination, std::ios::binary | std::ios::trunc);
        if (!out)
            return false;
        out.write(reinterpret_cast<char const*>(data.data()), static_cast<std::streamsize>(data.size()));
        return static_cast<bool>(out);
    }

    std::vector<uint32_t> CascStorage::enumerateFileDataIds() const
    {
        std::vector<uint32_t> result;
        if (!m_handle)
            return result;

        CASC_FIND_DATA findData{};
        HANDLE find = CascFindFirstFile(m_handle, "*", &findData, nullptr);
        if (find == INVALID_HANDLE_VALUE || find == nullptr)
            return result;

        do
        {
            if (findData.dwFileDataId != CASC_INVALID_ID)
                result.push_back(findData.dwFileDataId);
        }
        while (CascFindNextFile(find, &findData));

        CascFindClose(find);

        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }

    bool CascStorage::readDb2Identity(uint32_t fileDataId, uint32_t& tableHash, uint32_t& layoutHash, uint32_t /*localeMask*/) const
    {
        tableHash = 0;
        layoutHash = 0;
        if (!m_handle)
            return false;

        HANDLE file = nullptr;

        // Same semantics as Trinity's proven
        // OpenFile(fileDataId, CASC_LOCALE_NONE, false, true):
        // CASC_OPEN_BY_FILEID | CASC_OVERCOME_ENCRYPTED.
        if (!CascOpenFile(
                m_handle,
                CASC_FILE_DATA_ID(fileDataId),
                CASC_LOCALE_NONE,
                CASC_OPEN_BY_FILEID | CASC_OVERCOME_ENCRYPTED,
                &file))
            return false;

        std::array<uint8_t, 160> header{};
        DWORD read = 0;
        bool const readable = CascReadFile(file, header.data(), static_cast<DWORD>(header.size()), &read);

        bool const isWdc =
            readable
            && read >= 28
            && header[0] == 'W'
            && header[1] == 'D'
            && (header[2] == 'B' || header[2] == 'C')
            && header[3] >= '0'
            && header[3] <= '9';

        if (!isWdc)
        {
            CascCloseFile(file);
            return false;
        }

        auto readU32 = [&header](size_t offset)
        {
            return static_cast<uint32_t>(header[offset])
                | (static_cast<uint32_t>(header[offset + 1]) << 8)
                | (static_cast<uint32_t>(header[offset + 2]) << 16)
                | (static_cast<uint32_t>(header[offset + 3]) << 24);
        };

        // The user's known-good Forever 1.60.x dump uses the AscEmu/
        // WOWSTATIC wrapper before the normal WDC header fields.
        bool const wowStatic =
            read >= 160
            && header[8]  == 'W'
            && header[9]  == 'O'
            && header[10] == 'W'
            && header[11] == 'S'
            && header[12] == 'T'
            && header[13] == 'A'
            && header[14] == 'T'
            && header[15] == 'I'
            && header[16] == 'C';

        if (wowStatic)
        {
            tableHash = readU32(152);
            layoutHash = readU32(156);
        }
        else
        {
            tableHash = readU32(20);
            layoutHash = readU32(24);
        }

        CascCloseFile(file);
        return true;
    }

    std::optional<ClientBuildInfo> detectClient(std::filesystem::path startPath, std::string& error)
    {
        auto buildInfoPath = findBuildInfo(std::move(startPath));
        if (!buildInfoPath)
            return std::nullopt;

        std::ifstream in(*buildInfoPath);
        if (!in)
        {
            error = "Cannot open " + buildInfoPath->string();
            return std::nullopt;
        }

        std::string headerLine;
        if (!std::getline(in, headerLine))
        {
            error = "Empty .build.info";
            return std::nullopt;
        }

        auto const headers = splitPipe(headerLine);
        size_t productCol = std::string::npos;
        size_t versionCol = std::string::npos;
        size_t activeCol = std::string::npos;
        size_t buildKeyCol = std::string::npos;
        for (size_t i = 0; i < headers.size(); ++i)
        {
            std::string const name = baseHeaderName(headers[i]);
            if (name == "Product") productCol = i;
            else if (name == "Version") versionCol = i;
            else if (name == "Active") activeCol = i;
            else if (name == "Build Key") buildKeyCol = i;
        }

        if (productCol == std::string::npos || versionCol == std::string::npos)
        {
            error = ".build.info does not contain Product/Version columns";
            return std::nullopt;
        }

        std::optional<ClientBuildInfo> firstEntry;
        std::optional<ClientBuildInfo> activeEntry;
        std::optional<ClientBuildInfo> supportedEntry;
        std::string line;
        while (std::getline(in, line))
        {
            auto const fields = splitPipe(line);
            if (productCol >= fields.size() || versionCol >= fields.size())
                continue;

            uint32_t const build = parseBuild(fields[versionCol]);
            if (!build)
                continue;

            ClientBuildInfo info;
            info.storageRoot = buildInfoPath->parent_path();
            info.product = fields[productCol];
            if (buildKeyCol < fields.size())
                info.buildKey = fields[buildKeyCol];
            info.build = build;

            if (!firstEntry)
                firstEntry = info;
            if (activeCol < fields.size() && fields[activeCol] == "1" && !activeEntry)
                activeEntry = info;
            if (IsSupportedBuild(build))
                supportedEntry = info;
        }

        if (supportedEntry)
            return supportedEntry;
        if (activeEntry)
            return activeEntry;
        return firstEntry;
    }
}
