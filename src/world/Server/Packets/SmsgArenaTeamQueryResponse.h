/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

#include "Management/ArenaTeam.hpp"

namespace AscEmu::Packets
{
    class SmsgArenaTeamQueryResponse : public ManagedPacket
    {
    public:
        uint32_t id;
        utf8_string name;
        uint32_t playerCount;
        ArenaTeamEmblem emblem;
        
        SmsgArenaTeamQueryResponse() : SmsgArenaTeamQueryResponse(0, "", 0, {0})
        {
        }

        SmsgArenaTeamQueryResponse(uint32_t id, std::string name, uint32_t playerCount, ArenaTeamEmblem emblem) :
            ManagedPacket(SMSG_ARENA_TEAM_QUERY_RESPONSE, 4 * 7 + name.size() + 1),
            id(id),
            name(name),
            playerCount(playerCount),
            emblem(emblem)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 * 7 + name.size() + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet << id;
                packet << name;
                packet << playerCount;
                packet << emblem.emblemColour;
                packet << emblem.emblemStyle;
                packet << emblem.borderColour;
                packet << emblem.borderStyle;
                packet << emblem.backgroundColour;

                return true;
            }
            else if (m_protocol.expansion == WoW::Expansion::_Cata)
            {
                packet << id;
                packet << name;
                packet << playerCount;
                packet << emblem.backgroundColour;
                packet << emblem.emblemStyle;
                packet << emblem.emblemColour;
                packet << emblem.borderStyle;
                packet << emblem.borderColour;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
