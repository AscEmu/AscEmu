/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
 */

#ifndef _CRASH_HANDLER_H
#define _CRASH_HANDLER_H

#include "Common.hpp"

bool HookCrashReporter(bool logon);
void OutputCrashLogLine(const char* format, ...);

#ifdef WIN32

#include <DbgHelp.h>
#include "StackWalker.h"
#include "CircularQueue.h"

extern CircularQueue<uint32, 30> last_spells;

class CStackWalker : public StackWalker
{
    public:

        void OnOutput(LPCSTR szText) override;
        void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName) override;
        void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion) override;
        void OnCallstackEntry(CallstackEntryType eType, CallstackEntry & entry) override;
        void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr) override;
};

void StartCrashHandler();
void OnCrash(bool Terminate);

typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
int __cdecl HandleCrash(PEXCEPTION_POINTERS pExceptPtrs);

#define THREAD_TRY_EXECUTION __try{
#define THREAD_HANDLE_CRASH  }__except(HandleCrash(GetExceptionInformation())) {}

#else

#define THREAD_TRY_EXECUTION
#define THREAD_HANDLE_CRASH

#endif

#endif  //_CRASH_HANDLER_H
