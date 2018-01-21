/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Exceptions.hpp"

namespace AscEmu
{
    namespace Exception
    {
        class PlayerNotFoundException : public AscemuException
        {
        public:
            explicit PlayerNotFoundException() : AscemuException("Player not found") { }
        };
    }
}
