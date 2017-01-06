/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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

#ifndef _UTIL_H
#define _UTIL_H

#include "Common.hpp"

///////////////////////////////////////////////////////////////////////////////
// String Functions ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::vector<std::string> StrSplit(const std::string & src, const std::string & sep);


time_t convTimePeriod(uint32 dLength, char dType);

inline uint32 secsToTimeBitFields(time_t secs)
{
    tm* lt = localtime(&secs);
    return (lt->tm_year - 100) << 24 | lt->tm_mon  << 20 | (lt->tm_mday - 1) << 14 | lt->tm_wday << 11 | lt->tm_hour << 6 | lt->tm_min;
}


extern SERVER_DECL const char* _StringToUTF8(const char* pASCIIBuf);
extern SERVER_DECL const char* _StringToANSI(const char* pUtf8Buf);
extern SERVER_DECL bool _IsStringUTF8(const char* str);

volatile long Sync_Add(volatile long* value);

volatile long Sync_Sub(volatile long* value);

#ifdef WIN32

typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // must be 0x1000
    LPCSTR szName; // pointer to name (in user addr space)
    DWORD dwThreadID; // thread ID (-1=caller thread)
    DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

#endif

namespace Arcemu
{
    /////////////////////////////////////////////////////////////////////////
    //void Sleep( unsigned long timems );
    //  Puts the calling thread to sleep for the specified miliseconds
    //
    //Parameter(s)
    //  unsigned long timemes  -  time interval to put the thread to sleep for
    //
    //Return Value
    //  None
    //
    //
    /////////////////////////////////////////////////////////////////////////
    void Sleep(unsigned long timems);
}

template <class T>
inline T CalculatePctF(T base, float pct)
{
    return T(base * pct / 100.0f);
}

template <class T>
inline T CalculatePctN(T base, int32 pct)
{
    return T(base * float(pct) / 100.0f);
}

template <class T>
inline T CalculatePctU(T base, uint32 pct)
{
    return T(base * float(pct) / 100.0f);
}

template <class T>
inline T AddPctF(T& base, float pct)
{
    return base += CalculatePctF(base, pct);
}

template <class T>
inline T AddPctN(T& base, int32 pct)
{
    return base += CalculatePctN(base, pct);
}

template <class T>
inline T AddPctU(T& base, uint32 pct)
{
    return base += CalculatePctU(base, pct);
}

template <class T>
inline T ApplyPctF(T& base, float pct)
{
    return base = CalculatePctF(base, pct);
}

template <class T>
inline T ApplyPctN(T& base, int32 pct)
{
    return base = CalculatePctN(base, pct);
}

template <class T>
inline T ApplyPctU(T& base, uint32 pct)
{
    return base = CalculatePctU(base, pct);
}

template <class T>
inline T RoundToInterval(T& num, T floor, T ceil)
{
    return num = std::min(std::max(num, floor), ceil);
}
#endif
