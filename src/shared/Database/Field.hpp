/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <sstream>
#include <cstdint>

class Field
{
public:
    bool isSet() const { return mValue ? true : false; }
    void SetValue(char* value) { mValue = value; }

    const char* GetString() { return mValue; }
    float GetFloat() { return mValue ? static_cast<float>(atof(mValue)) : 0; }
    bool GetBool() { return mValue ? atoi(mValue) > 0 : false; }

    uint8_t GetUInt8() { return mValue ? static_cast<uint8_t>(atol(mValue)) : 0U; }
    int8_t GetInt8() { return mValue ? static_cast<int8_t>(atol(mValue)) : 0; }
    uint16_t GetUInt16() { return mValue ? static_cast<uint16_t>(atol(mValue)) : 0U; }
    int16_t GetInt16() { return mValue ? static_cast<int16_t>(atol(mValue)) : 0; }
    uint32_t GetUInt32() { return mValue ? static_cast<uint32_t>(atol(mValue)) : 0U; }
    int32_t GetInt32() { return mValue ? static_cast<int32_t>(atol(mValue)) : 0; }

    uint64_t GetUInt64()
    {
        if (mValue)
        {
            uint64_t value;
            std::istringstream iss(mValue);
            iss >> value;
            
            return value;
        }
        
        return 0;
    }

private:
    char* mValue;
};
