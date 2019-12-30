/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgQuestgiverQuestComplete : public ManagedPacket
    {
    public:
        uint32_t questId;
        uint32_t xp;
        uint32_t rewardMoney;
        uint32_t bonusHonor;
        uint32_t bonusTalent;

        //wotlk specific
        uint32_t bonusArenaPoints;

        SmsgQuestgiverQuestComplete() : SmsgQuestgiverQuestComplete(0, 0, 0, 0, 0, 0)
        {
        }

        SmsgQuestgiverQuestComplete(uint32_t questId, uint32_t xp, uint32_t rewardMoney, uint32_t bonusHonor, uint32_t bonusTalent, uint32_t bonusArenaPoints = 0) :
            ManagedPacket(SMSG_QUESTGIVER_QUEST_COMPLETE, 0),
            questId(questId), xp(xp), rewardMoney(rewardMoney), bonusHonor(bonusHonor), bonusTalent(bonusTalent), bonusArenaPoints(bonusArenaPoints)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 24;  //guessed, 4 * 8 + 4 missing for now
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << questId << xp << rewardMoney << bonusHonor << bonusTalent << bonusArenaPoints;

            // seems to be ignored by client
            // uint32_t reward item count
            // for (uint8_t i = 0; i < 4; ++i)
            // {
            //     if (qst->reward_item[i])
            //     {
            //         data << uint32_t(qst->reward_item[i]);
            //         data << uint32_t(qst->reward_itemcount[i]);
            //     }
            // }
#else
            packet << bonusTalent;
            packet << uint32_t(0);  // skillpoints
            packet << rewardMoney << xp << questId;
            packet << uint32_t(0);  // skillid

            packet.writeBit(0);     // reward items?
            packet.writeBit(1);
            packet.flushBits();
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
