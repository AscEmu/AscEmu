/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSendUnlearnSpells : public ManagedPacket
    {
    public:
        uint32_t count;
        
        SmsgSendUnlearnSpells() : SmsgSendUnlearnSpells(0)
        {
        }

        SmsgSendUnlearnSpells(uint32_t count) :
            ManagedPacket(SMSG_SEND_UNLEARN_SPELLS, 4),
            count(count)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << count;
            }
            else
            {
                packet.writeBits(count, 22);
                packet.flushBits();
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
