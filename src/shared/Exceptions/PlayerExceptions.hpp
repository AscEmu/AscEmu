/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Exceptions.hpp"

namespace AscEmu::Exception
{
    class PlayerNotFoundException : public AscemuException
    {
    public:
        explicit PlayerNotFoundException() : AscemuException("Player not found") { }
    };
}
