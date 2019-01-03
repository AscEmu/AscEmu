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
    class CmsgQuestPoiQuery : public ManagedPacket
    {
#if VERSION_STRING > TBC
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> questCount;
            for (uint32_t i = 0; i < questCount; ++i)
            {
                uint32_t questId;
                packet >> questId;

                questIds.push_back(questId);
            }
            return true;
        }
#endif
    };
}}
