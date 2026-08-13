/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgRaidInstanceMessage : public ManagedPacket
    {
    public:
        uint32_t type;
        uint32_t mapId;
        uint32_t difficulty;
        uint32_t time;
        uint8_t isLocked = 0;
        uint8_t isExtended = 0;

        SmsgRaidInstanceMessage() : SmsgRaidInstanceMessage(0, 0, 0, 0)
        {
        }

        SmsgRaidInstanceMessage(uint32_t type, uint32_t mapId, uint32_t difficulty, uint32_t time) :
            ManagedPacket(SMSG_RAID_INSTANCE_MESSAGE, 4 + 4 + 4 + 4 + 1 + 1),
            type(type),
            mapId(mapId),
            difficulty(difficulty),
            time(time)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 4 + 4 + 4 + 1 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << type << mapId << difficulty << time;
                if (type == 4) // Welcome
                    packet << isLocked << isExtended;

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(isLocked);
                packet.writeBit(isExtended);

                packet.flushBits();

                packet << mapId << type << time << difficulty;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
