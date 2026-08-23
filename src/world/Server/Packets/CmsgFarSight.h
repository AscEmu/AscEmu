/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgFarSight : public ManagedPacket
    {
    public:
        bool apply = false;

        CmsgFarSight() : CmsgFarSight(false)
        {
        }

        CmsgFarSight(bool apply) :
            ManagedPacket(CMSG_FAR_SIGHT, 1),
            apply(apply)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint8_t applyByte = 0;
                packet >> applyByte;
                apply = applyByte != 0;
                return true;
            }
            else if (m_protocol.isMop())
            {
                apply = packet.readBit();
                return true;
            }

            return false;
        }
    };
}
