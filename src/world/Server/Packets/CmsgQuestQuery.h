/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgQuestQuery : public ManagedPacket
    {
    public:
        uint32_t questId;

        CmsgQuestQuery() : CmsgQuestQuery(0)
        {
        }

        CmsgQuestQuery(uint32_t questId) :
            ManagedPacket(CMSG_QUEST_QUERY, 4),
            questId(questId)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> questId;
            return true;
        }
    };
}
