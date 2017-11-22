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

        // packets are uint16_t on WotLK NOT uint32_t
#if VERSION_STRING <= WotLK
            uint16 m_opcode;
#else
            uint32 m_opcode;
#endif

        public:

            WorldPacket data;
            ManagedPacket() {};
#if VERSION_STRING <= WotLK
            ManagedPacket(uint16 pOpcode, uint32 pSize) : m_opcode(pOpcode), data(pOpcode, pSize) {}
#else
            ManagedPacket(uint32 pOpcode, uint32 pSize) : m_opcode(pOpcode), data(pOpcode, pSize) {}
#endif
    };
}
