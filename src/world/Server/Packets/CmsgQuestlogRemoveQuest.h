/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgQuestlogRemoveQuest : public ManagedPacket
    {
    public:
        uint8_t questLogSlot;

        CmsgQuestlogRemoveQuest() : CmsgQuestlogRemoveQuest(0)
        {
        }

        CmsgQuestlogRemoveQuest(uint8_t questLogSlot) :
            ManagedPacket(CMSG_QUESTLOG_REMOVE_QUEST, 1),
            questLogSlot(questLogSlot)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> questLogSlot;
            return true;
        }
    };
}
