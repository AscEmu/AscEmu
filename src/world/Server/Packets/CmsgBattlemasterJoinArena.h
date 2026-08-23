/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgBattlemasterJoinArena : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t category;
        uint8_t asGroup;
        uint8_t ratedMatch;

        // Cata+: the client no longer sends guid/asGroup/ratedMatch at all - joining via
        // this opcode is always a group, always-rated queue. Only the arena slot (0/1/2 =
        // 2v2/3v3/5v5) is sent; the team/rating/group requirement is derived server-side.
        uint8_t arenaSlot = 0;

        CmsgBattlemasterJoinArena() : CmsgBattlemasterJoinArena(0, 0, 0, 0)
        {
        }

        CmsgBattlemasterJoinArena(uint64_t guid, uint8_t category, uint8_t asGroup, uint8_t ratedMatch) :
            ManagedPacket(CMSG_BATTLEMASTER_JOIN_ARENA, 0),
            guid(guid),
            category(category),
            asGroup(asGroup),
            ratedMatch(ratedMatch)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid >> category >> asGroup >> ratedMatch;
                guid.init(unpacked_guid);

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet >> arenaSlot;

                return true;
            }

            return false;
        }
    };
}
