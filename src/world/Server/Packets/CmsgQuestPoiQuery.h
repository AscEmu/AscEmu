/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgQuestPoiQuery : public ManagedPacket
    {
    public:
        uint32_t questCount;
        std::vector<uint32_t> questIds;

        CmsgQuestPoiQuery() : CmsgQuestPoiQuery(0, {})
        {
        }

        CmsgQuestPoiQuery(uint32_t questCount, std::vector<uint32_t> questIds) :
            ManagedPacket(CMSG_QUEST_POI_QUERY, 4 + 4 * questCount),
            questCount(questCount),
            questIds(questIds)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            questIds.clear();

            if (m_protocol.isForever())
            {
                packet >> questCount;
                questIds.reserve(questCount);
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> questCount;
            }
            else if (m_protocol.isMop())
            {
                questCount = packet.readBits(22);
            }
            else
            {
                return false;
            }

            for (uint32_t i = 0; i < questCount; ++i)
            {
                uint32_t questId;
                packet >> questId;

                questIds.push_back(questId);
            }

            return true;
        }
    };
}
