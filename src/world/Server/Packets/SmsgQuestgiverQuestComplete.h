/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

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

        // wotlk specific
        uint32_t bonusArenaPoints;

        // Forever / modern quest completion UI flow
        bool useQuestReward = false;
        bool launchGossip = false;
        bool launchQuest = false;
        bool hideChatMessage = false;

        SmsgQuestgiverQuestComplete() : SmsgQuestgiverQuestComplete(0, 0, 0, 0, 0, 0, false, false, false, false)
        {
        }

        SmsgQuestgiverQuestComplete(uint32_t questId, uint32_t xp, uint32_t rewardMoney, uint32_t bonusHonor, uint32_t bonusTalent, uint32_t bonusArenaPoints = 0, bool useQuestReward = false, bool launchGossip = false, bool launchQuest = false, bool hideChatMessage = false) :
            ManagedPacket(SMSG_QUESTGIVER_QUEST_COMPLETE, 0),
            questId(questId), xp(xp), rewardMoney(rewardMoney), bonusHonor(bonusHonor), bonusTalent(bonusTalent), bonusArenaPoints(bonusArenaPoints), useQuestReward(useQuestReward), launchGossip(launchGossip), launchQuest(launchQuest), hideChatMessage(hideChatMessage)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 24; // guessed, 4 * 8 + 4 missing for now
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                // Modern QuestGiverQuestComplete layout. The empty ItemInstance is
                // ItemID + 7-bit modifier count + optional ItemBonus bit.
                packet << static_cast<int32_t>(questId);
                packet << static_cast<int32_t>(xp);
                packet << static_cast<int64_t>(rewardMoney);
                packet << int32_t(0); // SkillLineIDReward
                packet << int32_t(0); // NumSkillUpsReward
                packet << int32_t(0); // ItemReward.ItemID
                packet.writeBits(0U, 7); // ItemReward.Modifications count
                packet.flushBits();
                packet.writeBit(false); // ItemReward.ItemBonus absent
                packet.flushBits();
                packet.writeBit(useQuestReward);
                packet.writeBit(launchGossip);
                packet.writeBit(launchQuest);
                packet.writeBit(hideChatMessage);
                packet.flushBits();
                return true;
            }

            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << questId << xp << rewardMoney << bonusHonor << bonusTalent << bonusArenaPoints;

                return true;
            }
            else if (m_protocol.isCata())
            {
                packet << bonusTalent;
                packet << uint32_t(0);  // skillpoints
                packet << rewardMoney << xp << questId;
                packet << uint32_t(0);  // skillid

                packet.writeBit(0);     // reward items?
                packet.writeBit(1);
                packet.flushBits();

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(1);
                packet.writeBit(0);
                packet.flushBits();

                packet << bonusTalent << rewardMoney << questId;
                packet << uint32_t(0);  // skillid
                packet << xp;
                packet << uint32_t(0);  // skillpoints

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
