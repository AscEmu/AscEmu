/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

struct ArenaTeamsList
{
    uint64_t playerGuid;
    uint8_t slot;
    uint32_t teamId;
    uint32_t teamRating;
    uint32_t seasonGames;
    uint32_t seasonWins;
    uint32_t memberSeasonGames;
    uint32_t memberPersonalRating;
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
        size_t expectedSize() const override { return static_cast<size_t>(8 + 1 + 4 + 4 + 4 + 4 + 4 + 4) * arenaTeams.size(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion > WoW::Expansion::_Classic && m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                for (const auto& teamMembers : arenaTeams)
                {
                    packet << teamMembers.playerGuid << teamMembers.slot << teamMembers.teamId <<
                        teamMembers.teamRating << teamMembers.seasonGames << teamMembers.seasonWins <<
                        teamMembers.memberSeasonGames << teamMembers.memberPersonalRating;
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion > WoW::Expansion::_Classic && m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                guid.init(unpackedGuid);
                return true;
            }

            return false;
        }
    };
}
