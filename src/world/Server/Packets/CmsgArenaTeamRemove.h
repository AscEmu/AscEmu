/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgArenaTeamRemove : public ManagedPacket
    {
    public:
        uint32_t teamId;
        std::string playerName;

        CmsgArenaTeamRemove() : CmsgArenaTeamRemove(0, "")
        {
        }

        CmsgArenaTeamRemove(uint32_t teamId, std::string playerName) :
            ManagedPacket(CMSG_ARENA_TEAM_REMOVE, 4),
            teamId(teamId),
            playerName(playerName)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> teamId >> playerName;
            return true;
        }
    };
}
