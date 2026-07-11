/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class MsgInspectHonorStats : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t honnorCurrency;
        uint32_t kills;
        uint32_t todayContrib;
        uint32_t yesterdayContrib;
        uint32_t lifetimeHonorKills;

        MsgInspectHonorStats() : MsgInspectHonorStats(0, 0, 0, 0, 0, 0)
        {
        }

        MsgInspectHonorStats(uint64_t guid, uint8_t honnorCurrency, uint32_t kills, uint32_t todayContrib, uint32_t yesterdayContrib, uint32_t lifetimeHonorKills) :
            ManagedPacket(MSG_INSPECT_HONOR_STATS, 8),
            guid(guid),
            honnorCurrency(honnorCurrency),
            kills(kills),
            todayContrib(todayContrib),
            yesterdayContrib(yesterdayContrib),
            lifetimeHonorKills(lifetimeHonorKills)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.isClassic())
                return static_cast<size_t>(8 + 1 + 4);
            if (m_protocol.expansion < WoW::Expansion::_Cata)
                return static_cast<size_t>(8 + 1 + 4 + 4 + 4 + 4);
            return static_cast<size_t>(8 + 1 + 4 + 4);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << honnorCurrency;
            if (!m_protocol.isClassic())
            {
                packet << kills;
                if (m_protocol.expansion < WoW::Expansion::_Cata)
                {
                    packet << todayContrib;
                    packet << yesterdayContrib;
                }
            }
            packet << lifetimeHonorKills;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.init(unpackedGuid);
            return true;
        }
    };
}
