/*
Copyright (c) 2015 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <exception>

namespace AscEmu
{
    namespace Exception
    {
        class AscemuException : public std::exception
        {
        public:
            explicit AscemuException() : exception("An unspecified exception occurred in AscEmu") { }
            explicit AscemuException(const char* exceptionString) : exception(exceptionString) { }
        };
    }
}