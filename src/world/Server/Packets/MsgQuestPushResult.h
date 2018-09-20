/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class MsgQuestPushResult : public ManagedPacket
    {
    public:
        uint64_t giverGuid;
        uint32_t questId;
        uint8_t pushResult;

        MsgQuestPushResult() : MsgQuestPushResult(0, 0, 0)
        {
        }

        MsgQuestPushResult(uint64_t giverGuid, uint32_t questId, uint8_t pushResult) :
            ManagedPacket(CMSG_QUESTGIVER_CHOOSE_REWARD, 9),
            giverGuid(giverGuid),
            questId(questId),
            pushResult(pushResult)
        {
        }

        size_t expectedSize() const { return 9; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << giverGuid << pushResult;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> giverGuid;
            if (packet.size() >= 13)
                packet >> questId;
            
            packet >> pushResult;
            return true;
        }
    };
}}
