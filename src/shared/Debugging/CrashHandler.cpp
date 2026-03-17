/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "git_version.hpp"

#include "Debugging/CrashHandler.h"
#include "CommonFilesystem.hpp"
#include "Logging/Logger.hpp"
#include <cstdarg>
#include <atomic>

#ifdef _WIN32
#include <Windows.h>
#include <crtdbg.h>
#endif

#include "Threading/Mutex.hpp"

#ifdef _WIN32
Mutex m_crashLock;

/* *
   @file CrashHandler.h
   Handles crashes/exceptions on a win32 based platform, writes a dump file,
   for later bug fixing.
*/

namespace {
    std::atomic<bool> hasDied{false}; // Thread-safe crash flag
    bool isDebuggerAttached{false};
}

void startCrashHandler()
{
    isDebuggerAttached = IsDebuggerPresent();

    if (!isDebuggerAttached)
    {
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

        SetUnhandledExceptionFilter(HandleCrash);
    }
}

void echo(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    std::string s = AscEmu::Logging::getFormattedFileName("logs", "CrashLog", false);
    FILE* m_file = fopen(s.c_str(), "a");
    if(!m_file)
    {
        va_end(ap);
        return;
    }

    vfprintf(m_file, format, ap);
    fclose(m_file);
    va_end(ap);
}

void CStackWalker::OnSymInit(LPCSTR /*szSearchPath*/, DWORD /*symOptions*/, LPCSTR /*szUserName*/)
{
}

void CStackWalker::OnLoadModule(LPCSTR /*img*/, LPCSTR /*mod*/, DWORD64 /*baseAddr*/, DWORD /*size*/, DWORD /*result*/, LPCSTR /*symType*/, LPCSTR /*pdbName*/, ULONGLONG /*fileVersion*/)
{
}

void CStackWalker::OnDbgHelpErr(LPCSTR /*szFuncName*/, DWORD /*gle*/, DWORD64 /*addr*/)
{
}

void CStackWalker::OnCallstackEntry(CallstackEntryType eType, CallstackEntry & entry)
{
    CHAR buffer[STACKWALK_MAX_NAMELEN];
    if((eType != lastEntry) && (entry.offset != 0))
    {
        if(entry.name[0] == 0)
            strcpy(entry.name, "(function-name not available)");
        if(entry.undName[0] != 0)
            strcpy(entry.name, entry.undName);
        if(entry.undFullName[0] != 0)
            strcpy(entry.name, entry.undFullName);

        char* p = strrchr(entry.loadedImageName, '\\');
        if(!p)
            p = entry.loadedImageName;
        else
            ++p;

        if(entry.lineFileName[0] == 0)
        {
            if(entry.name[0] == 0)
                sprintf(entry.name, "%lld", entry.offset);

            sprintf(buffer, "%s!%s Line %u\n", p, entry.name, entry.lineNumber);
        }
        else
            sprintf(buffer, "%s!%s Line %u\n", p, entry.name, entry.lineNumber);

        OnOutput(buffer);
    }
}

void CStackWalker::OnOutput(LPCSTR szText)
{
    std::string s = AscEmu::Logging::getFormattedFileName("logs", "CrashLog", false);
    FILE* m_file = fopen(s.c_str(), "a");
    if(!m_file) return;

    sLogger.failure("   {}", szText);
    fprintf(m_file, "   %s", szText);
    fclose(m_file);
}

LONG WINAPI HandleCrash(PEXCEPTION_POINTERS exceptionPointers)
{
    if(exceptionPointers == nullptr)
        return EXCEPTION_CONTINUE_SEARCH;

    // Only allow one thread to crash at a time
    if (!m_crashLock.attemptAcquire())
    {
        TerminateThread(GetCurrentThread(), static_cast<DWORD>(-1));
    }

    // Atomic check to prevent double faults
    bool expected = false;
    if (!hasDied.compare_exchange_strong(expected, true))
    {
        TerminateProcess(GetCurrentProcess(), static_cast<UINT>(-1));
    }

    // Create the date/time string
    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);

    char moduleName[MAX_PATH];
    ZeroMemory(moduleName, sizeof(moduleName));
    if (GetModuleFileNameA(nullptr, moduleName, MAX_PATH) == 0)
    {
        strcpy_s(moduleName, "UNKNOWN");
    }

    char* shortName = strrchr(moduleName, '\\');
    if (shortName)
        shortName++; // skip the backslash
    else
        shortName = moduleName;

    char filename[MAX_PATH];
    snprintf(filename, sizeof(filename),
        "CrashDumps\\dump-%s-%s-%04u-%02u-%02u-%02u-%02u-%02u-%u.dmp",
        shortName, AE_BUILD_HASH,
        systemTime.wYear, systemTime.wMonth, systemTime.wDay,
        systemTime.wHour, systemTime.wMinute, systemTime.wSecond,
        GetCurrentThreadId());


    CreateDirectoryA("CrashDumps", nullptr);
    HANDLE dumpFile = CreateFileA(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, nullptr);

    if (dumpFile != INVALID_HANDLE_VALUE)
    {
        sLogger.failure("Server has crashed. Creating crash dump file {}", filename);

        MINIDUMP_EXCEPTION_INFORMATION info;
        info.ClientPointers = FALSE;
        info.ExceptionPointers = exceptionPointers;
        info.ThreadId = GetCurrentThreadId();

        // Write the dump 
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
            dumpFile, MiniDumpWithIndirectlyReferencedMemory, &info, nullptr, nullptr);

        CloseHandle(dumpFile);
    }
    else
    {
        sLogger.failure("Could not open crash dump file.");
    }

    SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
    OnCrash(!isDebuggerAttached);
    sLogger.finalize();

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif
