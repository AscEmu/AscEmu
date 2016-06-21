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

#include "Common.h"

///////////////////////////////////////////////////////////////////////////////
// String Functions ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::vector<std::string> StrSplit(const std::string & src, const std::string & sep);

// This HAS to be called outside the threads __try / __except block!
void SetThreadName(const char* format, ...);
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
    SERVER_DECL float round(float f);
    SERVER_DECL double round(double d);
    SERVER_DECL long double round(long double ld);

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


#ifndef _FLAG96
#define _FLAG96

#ifndef PAIR64_HIPART
#define PAIR64_HIPART(x)   (uint32)((uint64(x) >> 32) & uint64(0x00000000FFFFFFFF))
#define PAIR64_LOPART(x)   (uint32)(uint64(x)         & uint64(0x00000000FFFFFFFF))
#endif

class flag96
{
    private:

        uint32 part[3];

    public:

        flag96(uint32 p1 = 0, uint32 p2 = 0, uint32 p3 = 0)
        {
            part[0] = p1;
            part[1] = p2;
            part[2] = p3;
        }

        inline bool IsEqual(uint32 p1 = 0, uint32 p2 = 0, uint32 p3 = 0) const
        {
            return (part[0] == p1 && part[1] == p2 && part[2] == p3);
        }

        inline bool HasFlag(uint32 p1 = 0, uint32 p2 = 0, uint32 p3 = 0) const
        {
            return (part[0] & p1 || part[1] & p2 || part[2] & p3);
        }

        inline void Set(uint32 p1 = 0, uint32 p2 = 0, uint32 p3 = 0)
        {
            part[0] = p1;
            part[1] = p2;
            part[2] = p3;
        }

        inline bool operator <(const flag96 &right) const
        {
            for (uint8 i = 3; i > 0; --i)
            {
                if (part[i - 1] < right.part[i - 1])
                    return true;
                else if (part[i - 1] > right.part[i - 1])
                    return false;
            }
            return false;
        }

        inline bool operator ==(const flag96 &right) const
        {
            return
                (
                part[0] == right.part[0] &&
                part[1] == right.part[1] &&
                part[2] == right.part[2]
                );
        }

        inline bool operator !=(const flag96 &right) const
        {
            return !this->operator ==(right);
        }

        inline flag96 & operator =(const flag96 &right)
        {
            part[0] = right.part[0];
            part[1] = right.part[1];
            part[2] = right.part[2];
            return *this;
        }

        inline flag96 operator &(const flag96 &right) const
        {
            return flag96(part[0] & right.part[0], part[1] & right.part[1],
                part[2] & right.part[2]);
        }

        inline flag96 & operator &=(const flag96 &right)
        {
            part[0] &= right.part[0];
            part[1] &= right.part[1];
            part[2] &= right.part[2];
            return *this;
        }

        inline flag96 operator |(const flag96 &right) const
        {
            return flag96(part[0] | right.part[0], part[1] | right.part[1],
                part[2] | right.part[2]);
        }

        inline flag96 & operator |=(const flag96 &right)
        {
            part[0] |= right.part[0];
            part[1] |= right.part[1];
            part[2] |= right.part[2];
            return *this;
        }

        inline flag96 operator ~() const
        {
            return flag96(~part[0], ~part[1], ~part[2]);
        }

        inline flag96 operator ^(const flag96 &right) const
        {
            return flag96(part[0] ^ right.part[0], part[1] ^ right.part[1],
                part[2] ^ right.part[2]);
        }

        inline flag96 & operator ^=(const flag96 &right)
        {
            part[0] ^= right.part[0];
            part[1] ^= right.part[1];
            part[2] ^= right.part[2];
            return *this;
        }

        inline operator bool() const
        {
            return (part[0] != 0 || part[1] != 0 || part[2] != 0);
        }

        inline bool operator !() const
        {
            return !this->operator bool();
        }

        inline uint32 & operator [](uint8 el)
        {
            return part[el];
        }

        inline const uint32 & operator [](uint8 el) const
        {
            return part[el];
        }
};
#endif

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
