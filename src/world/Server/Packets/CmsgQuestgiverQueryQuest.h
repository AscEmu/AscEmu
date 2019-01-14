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
    class CmsgQuestgiverQueryQuest : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t questId;

        // wotlk specific
        uint8_t unknown;

        CmsgQuestgiverQueryQuest() : CmsgQuestgiverQueryQuest(0, 0)
        {
        }

        CmsgQuestgiverQueryQuest(uint64_t guid, uint32_t questId) :
            ManagedPacket(CMSG_QUESTGIVER_QUERY_QUEST, 12),
            guid(guid),
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
#if VERSION_STRING > TBC
            packet >> unknown;
#endif
            guid.Init(unpackedGuid);
            return true;
        }
    };
}}
