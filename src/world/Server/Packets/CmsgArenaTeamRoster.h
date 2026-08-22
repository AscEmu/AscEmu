/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgArenaTeamRoster : public ManagedPacket
    {
    public:
        uint32_t teamId;

        CmsgArenaTeamRoster() : CmsgArenaTeamRoster(0)
        {
        }

        CmsgArenaTeamRoster(uint32_t teamId) :
            ManagedPacket(CMSG_ARENA_TEAM_ROSTER, 4),
            teamId(teamId)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> teamId;
                return true;
            }

            return false;
        }
    };
}
