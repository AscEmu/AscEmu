/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldPacket.h"

namespace Packets
{
    class ManagedPacket
    {
        protected:

            uint16_t m_opcode;

        public:

            WorldPacket data;
            ManagedPacket() {};
            ManagedPacket(uint16 pOpcode, uint32 pSize) : m_opcode(pOpcode), data(pOpcode, pSize) {}
    };
}
