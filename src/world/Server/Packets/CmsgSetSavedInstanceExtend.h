/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // the player extends a raid save by one reset or ends the extension
    class CmsgSetSavedInstanceExtend : public ManagedPacket
    {
    public:
        uint32_t mapId = 0;
        uint32_t difficulty = 0;
        bool extend = false;

        CmsgSetSavedInstanceExtend() :
            ManagedPacket(CMSG_SET_SAVED_INSTANCE_EXTEND, 4 + 4 + 1)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                int32_t map;
                packet >> map;
                mapId = static_cast<uint32_t>(map);

                // 12.x narrows the difficulty to 16 bit
                if (m_protocol.expansion >= WoW::Expansion::_Midnight)
                {
                    int16_t shortDifficulty;
                    packet >> shortDifficulty;
                    difficulty = static_cast<uint32_t>(shortDifficulty);
                }
                else
                {
                    packet >> difficulty;
                }

                extend = packet.readBit();
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                uint8_t value;
                packet >> mapId >> difficulty >> value;
                extend = value != 0;
                return true;
            }

            return false;
        }
    };
}
