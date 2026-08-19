/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

#include "Management/WorldStatesHandler.hpp"

namespace AscEmu::Packets
{
    class SmsgInitWorldStates : public ManagedPacket
    {
    public:
        uint32_t mapId;
        uint32_t zone;
        uint32_t area;
        bool clear {false};

        uint32_t arenaSeason {0};
        uint32_t arenaProgress {0};

        std::unordered_map<uint32_t, uint32_t> zoneWorldStates;

        SmsgInitWorldStates() : SmsgInitWorldStates(0, 0, 0)
        {
        }

        SmsgInitWorldStates(uint32_t mapId, uint32_t zone, uint32_t area) :
            ManagedPacket(SMSG_INIT_WORLD_STATES, 4 + 4 + 4 + 4 + 4 + 4 + 4),
            mapId(mapId),
            zone(zone),
            area(area)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 4 + 4 + 4 + 4 + 4 + 4 + (zoneWorldStates.size() * (4 + 4)); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
            {
                packet << uint32_t(mapId);
                packet << uint32_t(zone);
                packet << uint32_t(area);

                if (clear)
                {
                    packet << uint16_t(0);
                }
                else
                {
                    packet << uint16_t(1);
                    packet << uint32_t(3191);
                    packet << uint32_t(arenaSeason);
                }

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << uint32_t(mapId);
                packet << uint32_t(zone);
                packet << uint32_t(area);

                if (clear)
                {
                    packet << uint16_t(0);
                }
                else
                {
                    if (zoneWorldStates.size())
                    {
                        packet << uint16_t(2 + zoneWorldStates.size());

                        for (auto worldState : zoneWorldStates)
                        {
                            packet << uint32_t(worldState.first);
                            packet << uint32_t(worldState.second);
                        }
                    }
                    else
                    {
                        packet << uint16_t(2);
                    }

                    packet << uint32_t(3191);
                    packet << uint32_t(arenaSeason);
                    packet << uint32_t(3901);
                    packet << uint32_t(arenaProgress);
                }

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet << uint32_t(mapId);
                packet << uint32_t(zone);
                packet << uint32_t(area);

                if (clear)
                {
                    packet.writeBits(0, 21);
                }
                else
                {
                    if (zoneWorldStates.size())
                    {
                        packet.writeBits(2 + zoneWorldStates.size(), 21);
                        packet.flushBits();

                        for (auto worldState : zoneWorldStates)
                        {
                            packet << uint32_t(worldState.first);
                            packet << uint32_t(worldState.second);
                        }
                    }
                    else
                    {
                        packet.writeBits(2, 21);
                        packet.flushBits();
                    }

                    packet << uint32_t(3191);
                    packet << uint32_t(arenaSeason);
                    packet << uint32_t(3901);
                    packet << uint32_t(arenaProgress);
                }

                return true;
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
