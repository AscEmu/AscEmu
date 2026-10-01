/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgUiMapQuestLinesResponse : public ManagedPacket
    {
    public:
        int32_t uiMapId = 0;
        std::vector<uint32_t> questLineXQuestIds;
        std::vector<uint32_t> questIds;
        std::vector<uint32_t> questLineIds;

        SmsgUiMapQuestLinesResponse() : ManagedPacket(SMSG_UI_MAP_QUEST_LINES_RESPONSE, 16) { }
        explicit SmsgUiMapQuestLinesResponse(int32_t uiMapId) : ManagedPacket(SMSG_UI_MAP_QUEST_LINES_RESPONSE, 16), uiMapId(uiMapId) { }

    protected:
        size_t expectedSize() const override { return 16 + 4 * (questLineXQuestIds.size() + questIds.size() + questLineIds.size()); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet << uiMapId;
            packet << static_cast<uint32_t>(questLineXQuestIds.size());
            packet << static_cast<uint32_t>(questIds.size());
            packet << static_cast<uint32_t>(questLineIds.size());
            for (const auto id : questLineXQuestIds)
                packet << id;
            for (const auto id : questIds)
                packet << id;
            for (const auto id : questLineIds)
                packet << id;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
