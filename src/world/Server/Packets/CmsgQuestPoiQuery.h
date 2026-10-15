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

        CmsgQuestPoiQuery() : CmsgQuestPoiQuery(0, { 0 })
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
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> questCount;
            }
            else if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // count, then a fixed array of which the first count entries are used (6.x and 7.x 50, 8.x and 9.x
                // 125, 10.x and later 175 entries)
                packet >> questCount;
                if (questCount > 175)
                    return false;
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
