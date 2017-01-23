/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _MANAGED_PACKET_HPP
#define _MANAGED_PACKET_HPP

#include "WorldPacket.h"

namespace Packets
{
    class ManagedPacket
    {
        protected:

            uint32 m_opcode;

        public:

            WorldPacket data;
            ManagedPacket() {};
            ManagedPacket(uint32 pOpcode, uint32 pSize) : m_opcode(pOpcode), data(pOpcode, pSize) {}
    };
}

#endif // _MANAGED_PACKET_HPP
