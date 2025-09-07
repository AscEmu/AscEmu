/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <iostream>
#include <string>
#include <stdexcept>
#include <limits>

namespace Util
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Narrowing
    uint32_t MAKE_PAIR32(uint16_t l, uint16_t h);
    int32_t float2int32(float value);

    // Fastest Method of long2int32
    int32_t long2int32(const double value);

    template <typename T>
    T int32abs(int value)
    {
        return static_cast<T>(std::abs(value));
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Narrowing std::string and cstring to specific types

    // General template for unsigned types
    template <typename T>
    T safeStringToUnsigned(const std::string& _str, bool _silencedError)
    {
        try
        {
            unsigned long long value = std::stoull(_str);  // Convert to unsigned long long
            if (value > std::numeric_limits<T>::max())
            {
                throw std::out_of_range("Overflow for type");
            }
            return static_cast<T>(value);
        }
        catch (const std::invalid_argument& e)
        {
            if (!_silencedError)
                std::cerr << "Invalid argument: " << e.what() << std::endl;
            return 0; // Return 0 or handle error as needed
        }
        catch (const std::out_of_range& e)
        {
            if (!_silencedError)
                std::cerr << "Overflow error: " << e.what() << std::endl;
            return std::numeric_limits<T>::max(); // Return max value for overflow
        }
    }

    // General template for signed types
    template <typename T>
    T safeStringToSigned(const std::string& _str)
    {
        long long value = std::stoll(_str);  // Convert to long long
        try
        {
            if (value > std::numeric_limits<T>::max() || value < std::numeric_limits<T>::min())
            {
                throw std::out_of_range("Overflow/underflow for type");
            }
            return static_cast<T>(value);
        }
        catch (const std::invalid_argument& e)
        {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
            return 0; // Return 0 or handle error as needed
        }
        catch (const std::out_of_range& e)
        {
            std::cerr << "Overflow/underflow error: " << e.what() << std::endl;
            if (value < std::numeric_limits<T>::min())
            {
                return std::numeric_limits<T>::min(); // Handle underflow
            }
            return std::numeric_limits<T>::max(); // Handle overflow
        }
    }

    // Specialization for float with error handling
    template <>
    inline float safeStringToSigned<float>(const std::string& str)
    {
        try
        {
            return std::stof(str);  // Convert to float
        }
        catch (const std::invalid_argument& e)
        {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
            return 0.0f;  // Return 0.0 for invalid input
        }
        catch (const std::out_of_range& e)
        {
            std::cerr << "Overflow error: " << e.what() << std::endl;
            return std::numeric_limits<float>::max();  // Return max float for overflow
        }
    }

    // Specialization for double with error handling
    template <>
    inline double safeStringToSigned<double>(const std::string& str)
    {
        try
        {
            return std::stod(str);  // Convert to double
        }
        catch (const std::invalid_argument& e)
        {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
            return 0.0;  // Return 0.0 for invalid input
        }
        catch (const std::out_of_range& e)
        {
            std::cerr << "Overflow error: " << e.what() << std::endl;
            return std::numeric_limits<double>::max();  // Return max double for overflow
        }
    }

    // Specialization for bool (any value > 0 = true, <= 0 = false)
    template <>
    inline bool safeStringToSigned<bool>(const std::string& str)
    {
        try
        {
            int value = std::stoi(str);  // Convert to int
            return value > 0;  // true if greater than 0, false otherwise
        }
        catch (const std::invalid_argument& e)
        {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
            return false;  // Return false for invalid input
        }
    }

    // Overload for char* input
    template <typename T>
    T safeStringToUnsigned(const char* _cstr, bool _silencedError)
    {
        // Check for empty C-string and return 0
        if (!_cstr || *_cstr == '\0')
        {
            if (!_silencedError)
                std::cerr << "Empty string provided. Returning 0." << std::endl;

            return 0; // Return 0 for empty strings
        }
        std::string str(_cstr);  // Convert C-string to std::string
        return safeStringToUnsigned<T>(str, _silencedError);
    }

    // Overload for char* input
    template <typename T>
    T safeStringToSigned(const char* _cstr)
    {
        if (!_cstr || *_cstr == '\0')
        {
            std::cerr << "Empty string provided. Returning 0." << std::endl;
        }
        std::string str(_cstr);  // Convert C-string to std::string
        return safeStringToSigned<T>(str);
    }

    // Convenience functions for each specific type
    // bool conversion
    bool stringToBool(const std::string& _str);
    bool stringToBool(const char* _cstr);

    // uint8_t conversion
    uint8_t stringToUint8(const std::string& _str, bool _silencedError);
    uint8_t stringToUint8(const char* _cstr, bool _silencedError);

    // int8_t conversion
    int8_t stringToInt8(const std::string& _str);
    int8_t stringToInt8(const char* _cstr);

    // uint16_t conversion
    uint16_t stringToUint16(const std::string& _str, bool _silencedError);
    uint16_t stringToUint16(const char* _cstr, bool _silencedError);

    // int16_t conversion
    int16_t stringToInt16(const std::string& _str);
    int16_t stringToInt16(const char* _cstr);

    // uint32_t conversion
    uint32_t stringToUint32(const std::string& _str, bool _silencedError);
    uint32_t stringToUint32(const char* _cstr, bool _silencedError);

    // int32_t conversion
    int32_t stringToInt32(const std::string& _str);
    int32_t stringToInt32(const char* _cstr);

    // uint64_t conversion
    uint64_t stringToUint64(const std::string& _str, bool _silencedError);
    uint64_t stringToUint64(const char* _cstr, bool _silencedError);

    // int64_t conversion
    int64_t stringToInt64(const std::string& _str);
    int64_t stringToInt64(const char* cstr);

    // float conversion
    float stringToFloat(const std::string& _str);
    float stringToFloat(const char* _cstr);

    // double conversion
    double stringToDouble(const std::string& _str);
    double stringToDouble(const char* _cstr);
}
