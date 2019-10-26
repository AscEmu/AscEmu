/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "Management/ArenaTeam.h"

namespace AscEmu::Packets
{
    class SmsgArenaTeamStats : public ManagedPacket
    {
    public:
        uint32_t id;
        ArenaStats stats;
        
        SmsgArenaTeamStats() : SmsgArenaTeamStats(0, {0})
        {
        }

        SmsgArenaTeamStats(uint32_t id, ArenaStats stats) :
            ManagedPacket(SMSG_ARENA_TEAM_STATS, 4 * 7),
            id(id),
            stats(stats)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 * 7; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << id;
            packet << stats.rating;
            packet << stats.played_week;
            packet << stats.won_week;
            packet << stats.played_season;
            packet << stats.won_season;
            packet << stats.ranking;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
