/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgArenaTeamQuery : public ManagedPacket
    {
    public:
        uint32_t teamId;

        CmsgArenaTeamQuery() : CmsgArenaTeamQuery(0)
        {
        }

        CmsgArenaTeamQuery(uint32_t teamId) :
            ManagedPacket(CMSG_ARENA_TEAM_QUERY, 4),
            teamId(teamId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> teamId;
            return true;
        }
    };
}}
