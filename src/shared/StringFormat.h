/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <iostream>
#include <memory>

namespace AscEmu
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // printf like formatting for C++ with std::string

    template<typename ... Args>
    std::string string_format(const std::string& sch_format, Args ... args)
    {
        size_t size = 1 + snprintf(nullptr, 0, sch_format.c_str(), args ...);
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, sch_format.c_str(), args ...);
        return std::string(buf.get(), buf.get() + size);
    }
}
