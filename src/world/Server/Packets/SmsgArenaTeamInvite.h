/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgArenaTeamInvite : public ManagedPacket
    {
    public:
        std::string playerName;
        std::string teamName;

        SmsgArenaTeamInvite() : SmsgArenaTeamInvite("", "")
        {
        }

        SmsgArenaTeamInvite(std::string playerName, std::string teamName) :
            ManagedPacket(SMSG_ARENA_TEAM_INVITE, playerName.size() + 1 + teamName.size() + 1),
            playerName(std::move(playerName)),
            teamName(std::move(teamName))
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << playerName << teamName;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
