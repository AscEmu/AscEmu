/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include "Common.hpp"
#include "CommonTypes.hpp"

class Field
{
public:
    bool isSet() const { return mValue ? true : false; }
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
            return_value = sscanf(mValue, I64FMTD, (unsigned long long int*)&value);

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
