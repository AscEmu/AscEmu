/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgArenaTeamInvite : public ManagedPacket
    {
    public:
        uint32_t teamId;
        std::string playerName;

        CmsgArenaTeamInvite() : CmsgArenaTeamInvite(0, "")
        {
        }

        CmsgArenaTeamInvite(uint32_t teamId, std::string playerName) :
            ManagedPacket(CMSG_ARENA_TEAM_INVITE, 4),
            teamId(teamId),
            playerName(playerName)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> teamId >> playerName;
                return true;
            }

            return false;
        }
    };
}
