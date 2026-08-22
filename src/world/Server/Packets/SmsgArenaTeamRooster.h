/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/ArenaTeam.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgArenaTeamRooster : public ManagedPacket
    {
    public:
        uint32_t teamId;
        uint32_t memberCount;
        uint32_t playersPerTeam;
        std::vector<ArenaTeamPacketList> arenaTeamList;
        uint8_t unknown;

        SmsgArenaTeamRooster() : SmsgArenaTeamRooster(0, 0, 0, {}, 0)
        {
        }

        SmsgArenaTeamRooster(uint32_t teamId, uint32_t memberCount, uint32_t playersPerTeam, std::vector<ArenaTeamPacketList> arenaTeamList, uint8_t unknown = 0) :
            ManagedPacket(SMSG_ARENA_TEAM_ROSTER, 8 + memberCount * sizeof(ArenaTeamPacketList)),
            teamId(teamId),
            memberCount(memberCount),
            playersPerTeam(playersPerTeam),
            arenaTeamList(std::move(arenaTeamList)),
            unknown(unknown)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << teamId;
                if (m_protocol.expansion > WoW::Expansion::_TBC)
                {
                    packet << unknown;
                }
                packet << memberCount << playersPerTeam;

                for (const auto& teamListMember : arenaTeamList)
                {
                    packet << teamListMember.guid << teamListMember.isLoggedIn << teamListMember.name << teamListMember.isLeader << teamListMember.lastLevel << teamListMember.cl
                        << teamListMember.playedWeek << teamListMember.wonWeek << teamListMember.playedSeason << teamListMember.wonSeason << teamListMember.rating;
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
