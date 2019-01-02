/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgArenaTeamDisband : public ManagedPacket
    {
    public:
        uint32_t teamId;

        CmsgArenaTeamDisband() : CmsgArenaTeamDisband(0)
        {
        }

        CmsgArenaTeamDisband(uint32_t teamId) :
            ManagedPacket(CMSG_ARENA_TEAM_DISBAND, 4),
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
