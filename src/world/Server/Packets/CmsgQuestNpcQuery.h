/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <array>
#include <cstdint>

namespace AscEmu::Packets
{
    // 5.x: 50 quest ids (unused slots are 0) and the number of used slots; 6.x and later: the count, then up to 100 ids
    class CmsgQuestNpcQuery : public ManagedPacket
    {
    public:
        static constexpr uint32_t MOP_QUEST_ID_COUNT = 50;
        static constexpr uint32_t QUEST_ID_COUNT = 100;

        std::array<uint32_t, QUEST_ID_COUNT> questIds = {};
        uint32_t questCount = 0;

        CmsgQuestNpcQuery() :
            ManagedPacket(CMSG_QUEST_NPC_QUERY, 4)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                for (uint32_t i = 0; i < MOP_QUEST_ID_COUNT; ++i)
                    packet >> questIds[i];

                packet >> questCount;
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                packet >> questCount;
                if (questCount > QUEST_ID_COUNT)
                    return false;

                for (uint32_t i = 0; i < questCount; ++i)
                    packet >> questIds[i];

                return !packet.hadReadFailure();
            }

            return false;
        }
    };
}
