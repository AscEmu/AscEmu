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
    class CmsgQuestgiverChooseReward : public ManagedPacket
    {
    public:
        WoWGuid questgiverGuid;
        uint32_t questId;
        uint32_t rewardSlot;

        CmsgQuestgiverChooseReward() : CmsgQuestgiverChooseReward(0, 0, 0)
        {
        }

        CmsgQuestgiverChooseReward(uint64_t questgiverGuid, uint32_t questId, uint32_t rewardSlot) :
            ManagedPacket(CMSG_QUESTGIVER_CHOOSE_REWARD, 12),
            questgiverGuid(questgiverGuid),
            questId(questId),
            rewardSlot(rewardSlot)
        {
        }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid >> questId >> rewardSlot;
            questgiverGuid.Init(unpackedGuid);
            return true;
        }
    };
}}
