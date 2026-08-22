/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgOptOutOfLoot : public ManagedPacket
    {
    public:
        uint32_t turnedOn;

        CmsgOptOutOfLoot() : CmsgOptOutOfLoot(0)
        {
        }

        CmsgOptOutOfLoot(uint32_t turnedOn) :
            ManagedPacket(CMSG_OPT_OUT_OF_LOOT, 0),
            turnedOn(turnedOn)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                turnedOn = packet.readBit() ? 1 : 0;
                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> turnedOn;
                return true;
            }

            return false;
        }
    };
}
