/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "WorldConf.h"
#if VERSION_STRING != Cata
#include "ManagedPacket.h"
#include "MovementPacket.h"

namespace AscEmu { namespace Packets
{
    class MsgMoveFallLand : public MovementPacket
    {
    public:
        MsgMoveFallLand() :
            MovementPacket(MSG_MOVE_FALL_LAND, 0)
        {
        }
    };
}}
#endif
