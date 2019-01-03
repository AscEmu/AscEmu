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
    class CmsgQuestgiverCompleteQuest : public ManagedPacket
    {
    public:
        WoWGuid questgiverGuid;
        uint32_t questId;

        CmsgQuestgiverCompleteQuest() : CmsgQuestgiverCompleteQuest(0, 0)
        {
        }

        CmsgQuestgiverCompleteQuest(uint64_t questgiverGuid, uint32_t questId) :
            ManagedPacket(CMSG_QUESTGIVER_COMPLETE_QUEST, 12),
            questgiverGuid(questgiverGuid),
            questId(questId)
        {
        }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid >> questId;
            questgiverGuid.Init(unpackedGuid);
            return true;
        }
    };
}}
