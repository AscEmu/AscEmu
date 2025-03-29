/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

struct ArenaTeamsList
{
    uint64_t playerGuid;
    uint16_t teamType;
    uint32_t teamId;
    uint32_t teamRating;
    uint32_t playedWeek;
    uint32_t wonWeek;
    uint32_t playedSeason;
};

namespace AscEmu::Packets
{
    class MsgInspectArenaTeams : public ManagedPacket
    {
    public:
        WoWGuid guid;
        std::vector<ArenaTeamsList> arenaTeams;

        MsgInspectArenaTeams() : MsgInspectArenaTeams(0, {})
        {
        }

        MsgInspectArenaTeams(uint64_t guid, std::vector<ArenaTeamsList> arenaTeams) :
            ManagedPacket(MSG_INSPECT_ARENA_TEAMS, 8),
            guid(guid),
            arenaTeams(arenaTeams)
        {
        }

    protected:
        size_t expectedSize() const override { return static_cast<size_t>(8 + 2 + 4 + 4 + 4 + 4 + 4) * arenaTeams.size(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            for (const auto& teamMembers : arenaTeams)
            {
                packet << teamMembers.playerGuid << teamMembers.teamType << teamMembers.teamId << 
                    teamMembers.teamRating << teamMembers.playedWeek << teamMembers.wonWeek << teamMembers.playedSeason;
            }
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
