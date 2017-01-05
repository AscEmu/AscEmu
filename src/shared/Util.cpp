/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Util.hpp"

namespace Util
{
    void StringToLowerCase(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    void StringToUpperCase(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    }
}

