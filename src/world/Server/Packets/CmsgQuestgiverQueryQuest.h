/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgQuestgiverQueryQuest : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t questId;

        // wotlk specific
        uint8_t unknown = 0;

        CmsgQuestgiverQueryQuest() : CmsgQuestgiverQueryQuest(0, 0)
        {
        }

        CmsgQuestgiverQueryQuest(uint64_t guid, uint32_t questId) :
            ManagedPacket(CMSG_QUESTGIVER_QUERY_QUEST, 12),
            guid(guid),
            questId(questId)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid >> questId;
            if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                packet >> unknown;
            }
            guid.init(unpackedGuid);
            return true;
        }
    };
}
