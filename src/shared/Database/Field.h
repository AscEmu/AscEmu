/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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

#ifndef _FIELD_H
#define _FIELD_H

#include <stdlib.h>
#include <stdio.h>
#include "Common.hpp"
#include "CommonTypes.hpp"

class Field
{
    public:

    //MIT start
        bool isSet() const { return mValue ? true : false; }
    //MIT end
        inline void SetValue(char* value) { mValue = value; }

        inline const char* GetString() { return mValue; }
        inline float GetFloat() { return mValue ? static_cast<float>(atof(mValue)) : 0; }
        inline bool GetBool() { return mValue ? atoi(mValue) > 0 : false; }

        inline uint8_t GetUInt8() { return mValue ? static_cast<uint8_t>(atol(mValue)) : 0; }
        inline int8_t GetInt8() { return mValue ? static_cast<int8_t>(atol(mValue)) : 0; }
        inline uint16_t GetUInt16() { return mValue ? static_cast<uint16_t>(atol(mValue)) : 0; }
        inline int16_t GetInt16() { return mValue ? static_cast<int16_t>(atol(mValue)) : 0; }
        inline uint32_t GetUInt32() { return mValue ? static_cast<uint32_t>(atol(mValue)) : 0; }
        inline int32_t GetInt32() { return mValue ? static_cast<int32_t>(atol(mValue)) : 0; }

        uint64_t GetUInt64()
        {
            if (mValue)
            {
                uint64_t value;
                int return_value;
#ifndef WIN32    // Make GCC happy.
                return_value = sscanf(mValue, I64FMTD, (long long unsigned int*)&value);
#else
                return_value = sscanf(mValue, I64FMTD, &value);
#endif
                if (return_value != 1)
                    return 0;
                else
                    return value;
            }
            else
                return 0;
        }

    private:

        char* mValue;
};

#endif      //_FIELD_H
