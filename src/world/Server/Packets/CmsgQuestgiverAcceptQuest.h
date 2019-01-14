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
    class CmsgQuestgiverAcceptQuest : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t questId;

        CmsgQuestgiverAcceptQuest() : CmsgQuestgiverAcceptQuest(0, 0)
        {
        }

        CmsgQuestgiverAcceptQuest(uint64_t guid, uint32_t questId) :
            ManagedPacket(CMSG_QUESTGIVER_ACCEPT_QUEST, 12),
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
            packet >> guid >> questId;
            return true;
        }
    };
}}
