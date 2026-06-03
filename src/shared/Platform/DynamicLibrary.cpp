/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Platform/DynamicLibrary.hpp"

#include <utility>

#if defined(_WIN32)
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace AscEmu::Platform
{
    namespace
    {
#if defined(_WIN32)
        HMODULE toNativeHandle(DynamicLibrary::NativeHandle handle) noexcept
        {
            return static_cast<HMODULE>(handle);
        }

        DynamicLibrary::NativeHandle fromNativeHandle(HMODULE handle) noexcept
        {
            return static_cast<DynamicLibrary::NativeHandle>(handle);
        }

        std::string wideToUtf8(std::wstring_view value)
        {
            if (value.empty())
                return {};

            const auto requiredSize = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);

            if (requiredSize <= 0)
                return {};

            std::string result(static_cast<std::size_t>(requiredSize), '\0');

            WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), requiredSize, nullptr, nullptr);

            return result;
        }
#endif
    }

    DynamicLibrary::DynamicLibrary(std::filesystem::path path)
        : m_path(std::move(path))
    {}

    DynamicLibrary::~DynamicLibrary()
    {
        close();
    }

    DynamicLibrary::DynamicLibrary(DynamicLibrary&& other) noexcept
        : m_path(std::move(other.m_path)), m_handle(std::exchange(other.m_handle, nullptr)), m_lastError(std::move(other.m_lastError))
    {}

    DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other) noexcept
    {
        if (this == &other)
            return *this;

        close();

        m_path = std::move(other.m_path);
        m_handle = std::exchange(other.m_handle, nullptr);
        m_lastError = std::move(other.m_lastError);

        return *this;
    }

    bool DynamicLibrary::load()
    {
        resetError();

        if (isLoaded())
            return true;

        if (m_path.empty())
        {
            setError("DynamicLibrary::load failed: library path is empty");
            return false;
        }

#if defined(_WIN32)
        const auto nativeHandle = LoadLibraryW(m_path.wstring().c_str());
        if (nativeHandle == nullptr)
        {
            setError("LoadLibrary failed for '" + m_path.string() + "': " + getLastNativeError());
            return false;
        }

        m_handle = fromNativeHandle(nativeHandle);
#else
        m_handle = dlopen(m_path.c_str(), RTLD_NOW);
        if (m_handle == nullptr)
        {
            setError("dlopen failed for '" + m_path.string() + "': " + getLastNativeError());
            return false;
        }
#endif

        return true;
    }

    void DynamicLibrary::close() noexcept
    {
        if (m_handle == nullptr)
            return;

        auto* handleToClose = std::exchange(m_handle, nullptr);

#if defined(_WIN32)
        if (FreeLibrary(toNativeHandle(handleToClose)) == 0)
        {
            // Destructor-safe: do not throw from close().
            m_lastError = "FreeLibrary failed for '" + m_path.string() + "': " + getLastNativeError();
        }
#else
        if (dlclose(handleToClose) != 0)
        {
            // Destructor-safe: do not throw from close().
            m_lastError = "dlclose failed for '" + m_path.string() + "': " + getLastNativeError();
        }
#endif
    }

    void* DynamicLibrary::symbolAddress(std::string_view symbolName) noexcept
    {
        resetError();

        if (!isLoaded())
        {
            setError("Cannot resolve symbol '" + std::string(symbolName) + "': library is not loaded");
            return nullptr;
        }

        if (symbolName.empty())
        {
            setError("Cannot resolve symbol: symbol name is empty");
            return nullptr;
        }

#if defined(_WIN32)
        auto* address = reinterpret_cast<void*>(GetProcAddress(toNativeHandle(m_handle), std::string(symbolName).c_str()));

        if (address == nullptr)
        {
            setError("GetProcAddress failed for symbol '" + std::string(symbolName) + "' in '" + m_path.string() + "': " + getLastNativeError());
        }

        return address;
#else
        // POSIX requires clearing dlerror() before dlsym(), because a null symbol
        // address can be valid on some platforms.
        dlerror();

        auto* address = dlsym(m_handle, std::string(symbolName).c_str());

        if (const char* error = dlerror(); error != nullptr)
        {
            setError("dlsym failed for symbol '" + std::string(symbolName) + "' in '" + m_path.string() + "': " + error);
            return nullptr;
        }

        return address;
#endif
    }

    bool DynamicLibrary::isLoaded() const noexcept
    {
        return m_handle != nullptr;
    }

    bool DynamicLibrary::hasError() const noexcept
    {
        return !m_lastError.empty();
    }

    const std::filesystem::path& DynamicLibrary::path() const noexcept
    {
        return m_path;
    }

    const std::string& DynamicLibrary::lastError() const noexcept
    {
        return m_lastError;
    }

    void DynamicLibrary::setPath(std::filesystem::path path)
    {
        if (isLoaded())
            close();

        m_path = std::move(path);
        resetError();
    }

    bool DynamicLibrary::hasLibraryExtension(const std::filesystem::path& path)
    {
        std::string fileExtension;

#if defined(_WIN32)
        fileExtension = ".dll";
#elif defined(__APPLE__)
        fileExtension = ".dylib";
#else
        fileExtension = ".so";
#endif

        return path.extension() == fileExtension;
    }

    bool DynamicLibrary::isLibraryFile(const std::filesystem::directory_entry& entry)
    {
        return entry.is_regular_file() && hasLibraryExtension(entry.path());
    }

    void DynamicLibrary::resetError() noexcept
    {
        m_lastError.clear();
    }

    void DynamicLibrary::setError(std::string message)
    {
        m_lastError = std::move(message);
    }

    std::string DynamicLibrary::getLastNativeError()
    {
#if defined(_WIN32)
        const auto errorCode = GetLastError();
        if (errorCode == 0)
            return "no error information available";

        LPWSTR messageBuffer = nullptr;

        const auto size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&messageBuffer), 0, nullptr
        );

        if (size == 0 || messageBuffer == nullptr)
            return "Windows error code " + std::to_string(errorCode);

        std::wstring message(messageBuffer, size);
        LocalFree(messageBuffer);

        while (!message.empty() && (message.back() == L'\n' || message.back() == L'\r'))
            message.pop_back();

        const auto utf8Message = wideToUtf8(message);
        if (!utf8Message.empty())
            return utf8Message;

        return "Windows error code " + std::to_string(errorCode);
#else
        if (const char* error = dlerror(); error != nullptr)
            return error;

        return "no error information available";
#endif
    }
}
