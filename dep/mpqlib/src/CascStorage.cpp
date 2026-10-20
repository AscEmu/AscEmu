/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/CascStorage.hpp"

#include <CascLib.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <type_traits>

namespace mpqlib
{
    namespace
    {
        // CascLib wants the file data id smuggled through the name pointer.
        LPCSTR fileDataIdAsName(uint32_t fileDataId) noexcept
        {
            return CASC_FILE_DATA_ID(fileDataId);
        }

        void* openFile(void* storage, LPCSTR nameOrId, uint32_t localeMask, DWORD openFlags)
        {
            HANDLE file = nullptr;
            // missing encryption keys zero-fill the affected blocks instead of failing the whole file
            if (!CascOpenFile(storage, nameOrId, localeMask, openFlags | CASC_OVERCOME_ENCRYPTED, &file))
            {
                if (file != nullptr)
                    CascCloseFile(file);
                return nullptr;
            }
            return file;
        }
    }

    CascStorage::CascStorage(void* handle) noexcept : m_handle(handle)
    {
    }

    CascStorage::~CascStorage()
    {
        if (m_handle != nullptr)
            CascCloseStorage(m_handle);
    }

    std::unique_ptr<CascStorage> CascStorage::open(std::filesystem::path const& clientRoot, std::string_view product, uint32_t localeMask)
    {
        // CascLib's string type follows the UNICODE setting of the build
        using CascChar = std::remove_const_t<std::remove_pointer_t<decltype(CASC_OPEN_STORAGE_ARGS::szLocalPath)>>;
        std::string const narrowPath = clientRoot.string();
        std::basic_string<CascChar> const path(narrowPath.begin(), narrowPath.end());
        std::basic_string<CascChar> const codeName(product.begin(), product.end());

        CASC_OPEN_STORAGE_ARGS args{};
        args.Size = sizeof(CASC_OPEN_STORAGE_ARGS);
        args.szLocalPath = path.c_str();
        args.szCodeName = codeName.empty() ? nullptr : codeName.c_str();
        args.dwLocaleMask = localeMask;

        HANDLE handle = nullptr;
        if (!CascOpenStorageEx(nullptr, &args, false, &handle))
        {
            uint32_t const error = GetCascError();
            if (handle != nullptr)
                CascCloseStorage(handle);
            printf("Error opening CASC storage '%s' (product %.*s): %.*s\n", narrowPath.c_str(), static_cast<int>(product.size()), product.data(),
                static_cast<int>(errorText(error).size()), errorText(error).data());
            return nullptr;
        }

        return std::unique_ptr<CascStorage>(new CascStorage(handle));
    }

    uint32_t CascStorage::buildNumber() const
    {
        CASC_STORAGE_PRODUCT product{};
        size_t needed = 0;
        if (!CascGetStorageInfo(m_handle, CascStorageProduct, &product, sizeof(product), &needed))
            return 0;
        return product.BuildNumber;
    }

    std::optional<std::vector<std::byte>> CascStorage::readOpenedFile(void* fileHandle) const
    {
        ULONGLONG size = 0;
        if (!CascGetFileSize64(fileHandle, &size) || size == CASC_INVALID_SIZE64)
            return std::nullopt;

        std::vector<std::byte> data(static_cast<size_t>(size));
        size_t position = 0;
        while (position < data.size())
        {
            // CascReadFile counts in DWORDs, large files go in slices
            DWORD const slice = static_cast<DWORD>(std::min<size_t>(data.size() - position, 0x7FFFFFFF));
            DWORD read = 0;
            if (!CascReadFile(fileHandle, data.data() + position, slice, &read) || read == 0)
                return std::nullopt;
            position += read;
        }
        return data;
    }

    std::optional<std::vector<std::byte>> CascStorage::readFile(std::string_view name, uint32_t localeMask) const
    {
        std::string const fileName(name);
        std::lock_guard<std::mutex> lock(m_mutex);
        void* file = openFile(m_handle, fileName.c_str(), localeMask, CASC_OPEN_BY_NAME);
        if (file == nullptr)
            return std::nullopt;

        auto data = readOpenedFile(file);
        CascCloseFile(file);
        return data;
    }

    std::optional<std::vector<std::byte>> CascStorage::readFile(uint32_t fileDataId, uint32_t localeMask) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        void* file = openFile(m_handle, fileDataIdAsName(fileDataId), localeMask, CASC_OPEN_BY_FILEID);
        if (file == nullptr)
            return std::nullopt;

        auto data = readOpenedFile(file);
        CascCloseFile(file);
        return data;
    }

    std::optional<std::vector<std::byte>> CascStorage::readFilePrefix(uint32_t fileDataId, size_t count, uint32_t localeMask) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        void* file = openFile(m_handle, fileDataIdAsName(fileDataId), localeMask, CASC_OPEN_BY_FILEID);
        if (file == nullptr)
            return std::nullopt;

        std::vector<std::byte> data(count);
        DWORD read = 0;
        bool const ok = CascReadFile(file, data.data(), static_cast<DWORD>(count), &read);
        CascCloseFile(file);
        if (!ok)
            return std::nullopt;

        data.resize(read);
        return data;
    }

    bool CascStorage::hasFile(std::string_view name, uint32_t localeMask) const
    {
        std::string const fileName(name);
        std::lock_guard<std::mutex> lock(m_mutex);
        void* file = openFile(m_handle, fileName.c_str(), localeMask, CASC_OPEN_BY_NAME);
        if (file == nullptr)
            return false;
        CascCloseFile(file);
        return true;
    }

    bool CascStorage::hasFile(uint32_t fileDataId, uint32_t localeMask) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        void* file = openFile(m_handle, fileDataIdAsName(fileDataId), localeMask, CASC_OPEN_BY_FILEID);
        if (file == nullptr)
            return false;
        CascCloseFile(file);
        return true;
    }

    uint32_t CascStorage::lastError()
    {
        return GetCascError();
    }

    std::string_view CascStorage::errorText(uint32_t cascError)
    {
        switch (cascError)
        {
            case ERROR_SUCCESS:             return "success";
            case ERROR_FILE_CORRUPT:        return "file corrupt";
            case ERROR_CAN_NOT_COMPLETE:    return "can not complete";
            case ERROR_HANDLE_EOF:          return "end of file";
            case ERROR_NO_MORE_FILES:       return "no more files";
            case ERROR_BAD_FORMAT:          return "bad format";
            case ERROR_INSUFFICIENT_BUFFER: return "insufficient buffer";
            case ERROR_ALREADY_EXISTS:      return "already exists";
            case ERROR_DISK_FULL:           return "disk full";
            case ERROR_INVALID_PARAMETER:   return "invalid parameter";
            case ERROR_NOT_SUPPORTED:       return "not supported";
            case ERROR_NOT_ENOUGH_MEMORY:   return "not enough memory";
            case ERROR_INVALID_HANDLE:      return "invalid handle";
            case ERROR_ACCESS_DENIED:       return "access denied";
            case ERROR_FILE_NOT_FOUND:      return "file not found";
            case ERROR_FILE_ENCRYPTED:      return "file encrypted";
            case ERROR_FILE_OFFLINE:        return "file offline";
            default:                        return "unknown error";
        }
    }
}
