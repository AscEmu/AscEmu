/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SysInfo.hpp"

#include <chrono>
#include <fstream>
#include <string>

#if defined(_WIN32)

    #include <Windows.h>
    #include <psapi.h>
    #pragma comment(lib, "Psapi.lib")

#elif defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)

    #include <sys/resource.h>
    #include <unistd.h>

    #if defined(__FreeBSD__) || defined(__APPLE__)
        #include <sys/types.h>
        #include <sys/sysctl.h>
    #endif

    #ifdef __APPLE__
        #include <mach/mach.h>
        #include <mach/task.h>
    #endif

#endif

namespace AscEmu
{
    namespace
    {
        constexpr uint64_t MicrosecondsPerSecond = 1'000'000ULL;
        constexpr uint64_t KilobytesToBytes = 1024ULL;
    }

#if defined(_WIN32)

    std::uint32_t SysInfo::getCPUCount()
    {
        SYSTEM_INFO systemInfo{};
        GetSystemInfo(&systemInfo);

        return systemInfo.dwNumberOfProcessors;
    }

    uint64_t SysInfo::getCPUUsage()
    {
        FILETIME creationTime{};
        FILETIME exitTime{};
        FILETIME kernelTime{};
        FILETIME userTime{};

        if (!GetProcessTimes(
                GetCurrentProcess(),
                &creationTime,
                &exitTime,
                &kernelTime,
                &userTime))
        {
            return 0;
        }

        ULARGE_INTEGER kernel{};
        kernel.HighPart = kernelTime.dwHighDateTime;
        kernel.LowPart = kernelTime.dwLowDateTime;

        ULARGE_INTEGER user{};
        user.HighPart = userTime.dwHighDateTime;
        user.LowPart = userTime.dwLowDateTime;

        // FILETIME = 100ns -> microseconds
        return (kernel.QuadPart + user.QuadPart) / 10ULL;
    }

    uint64_t SysInfo::getRAMUsage()
    {
        PROCESS_MEMORY_COUNTERS_EX memoryInfo{};

        if (!GetProcessMemoryInfo(
                GetCurrentProcess(),
                reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memoryInfo),
                sizeof(memoryInfo)))
        {
            return 0;
        }

        return memoryInfo.WorkingSetSize;
    }

    uint64_t SysInfo::getTickCount()
    {
        return ::GetTickCount64();
    }

#elif defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)

    std::uint32_t SysInfo::getCPUCount()
    {
        return static_cast<std::uint32_t>(sysconf(_SC_NPROCESSORS_ONLN));

    }

    uint64_t SysInfo::getCPUUsage()
    {
        rusage usage{};

        if (getrusage(RUSAGE_SELF, &usage) != 0)
            return 0;

        const uint64_t userTime =
            static_cast<uint64_t>(usage.ru_utime.tv_sec) *
            MicrosecondsPerSecond +
            static_cast<uint64_t>(usage.ru_utime.tv_usec);

        const uint64_t systemTime =
            static_cast<uint64_t>(usage.ru_stime.tv_sec) *
            MicrosecondsPerSecond +
            static_cast<uint64_t>(usage.ru_stime.tv_usec);

        return userTime + systemTime;
    }

    uint64_t SysInfo::getRAMUsage()
    {
    #if defined(__linux__)

        std::ifstream file("/proc/self/status");

        if (!file.is_open())
            return 0;

        std::string line;

        while (std::getline(file, line))
        {
            if (line.starts_with("VmRSS:"))
            {
                const auto firstDigit = line.find_first_of("0123456789");

                if (firstDigit == std::string::npos)
                    return 0;

                const auto kilobytes =
                    std::stoull(line.substr(firstDigit));

                return kilobytes * KilobytesToBytes;
            }
        }

        return 0;

    #elif defined(__APPLE__)

        task_basic_info info{};
        mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;

        if (task_info(
                mach_task_self(),
                TASK_BASIC_INFO,
                reinterpret_cast<task_info_t>(&info),
                &count) != KERN_SUCCESS)
        {
            return 0;
        }

        return info.resident_size;

    #else
        return 0;
    #endif
    }

    uint64_t SysInfo::getTickCount()
    {
        using namespace std::chrono;

        return duration_cast<milliseconds>(
            steady_clock::now().time_since_epoch()
        ).count();
    }

#endif
}
