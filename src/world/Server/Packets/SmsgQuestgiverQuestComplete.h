/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "QuestPacketsGuid128.h"

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
            return 24; // guessed, 4 * 8 + 4 missing for now
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // no reward item, no skill reward
                if (m_protocol.isWoD())
                {
                    packet << int32_t(questId);
                    packet << int32_t(0);           // skill line
                    packet << int32_t(rewardMoney);
                    packet << int32_t(0);           // skill ups
                    packet << int32_t(xp);
                    packet << int32_t(bonusTalent);
                    writeEmptyItemInstance(packet, m_protocol, 0);
                    packet.writeBit(false);         // use quest reward
                    packet.writeBit(false);         // launch gossip
                    packet.flushBits();
                    return true;
                }

                packet << int32_t(questId);
                packet << int32_t(xp);
                packet << int64_t(rewardMoney);
                packet << int32_t(0);               // skill line
                packet << int32_t(0);               // skill ups

                // 12.x: the item before the bits
                if (m_protocol.isMidnight())
                    writeEmptyItemInstance(packet, m_protocol, 0);

                packet.writeBit(false);             // use quest reward
                packet.writeBit(false);             // launch gossip
                packet.writeBit(false);             // launch quest
                packet.writeBit(false);             // hide chat message
                if (m_protocol.isLegion())
                    packet.flushBits();

                if (!m_protocol.isMidnight())
                    writeEmptyItemInstance(packet, m_protocol, 0);
                else
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
