/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Object.h"
#include "Units/Unit.h"

namespace Packets
{
    namespace Movement
    {
        void SendMoveToPacket(Unit* pUnit);
    }
}
