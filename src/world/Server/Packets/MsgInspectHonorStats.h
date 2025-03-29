/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "AEVersion.hpp"
#include "ManagedPacket.h"

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
#if VERSION_STRING == Classic
        size_t expectedSize() const override { return static_cast<size_t>(8 + 1 + 4); }
#elif VERSION_STRING < Cata
        size_t expectedSize() const override { return static_cast<size_t>(8 + 1 + 4 + 4 + 4 + 4); }
#else
        size_t expectedSize() const override { return static_cast<size_t>(8 + 1 + 4 + 4); }
#endif

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << honnorCurrency;
#if VERSION_STRING != Classic
            packet << kills;
#if VERSION_STRING < Cata
            packet << todayContrib;
            packet << yesterdayContrib;
#endif
#endif
            packet << lifetimeHonorKills;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.Init(unpackedGuid);
            return true;
        }
    };
}
