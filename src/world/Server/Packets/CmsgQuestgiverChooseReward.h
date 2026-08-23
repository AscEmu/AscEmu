/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
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

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> questId >> rewardSlot;
                questgiverGuid.init(unpackedGuid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                // reward is now an item id, not a slot index
                packet >> rewardSlot;
                packet >> questId;

                questgiverGuid[2] = packet.readBit();
                questgiverGuid[6] = packet.readBit();
                questgiverGuid[0] = packet.readBit();
                questgiverGuid[5] = packet.readBit();
                questgiverGuid[1] = packet.readBit();
                questgiverGuid[3] = packet.readBit();
                questgiverGuid[7] = packet.readBit();
                questgiverGuid[4] = packet.readBit();

                packet.readByteSeq(questgiverGuid[1]);
                packet.readByteSeq(questgiverGuid[2]);
                packet.readByteSeq(questgiverGuid[5]);
                packet.readByteSeq(questgiverGuid[7]);
                packet.readByteSeq(questgiverGuid[0]);
                packet.readByteSeq(questgiverGuid[3]);
                packet.readByteSeq(questgiverGuid[6]);
                packet.readByteSeq(questgiverGuid[4]);
                return true;
            }

            return false;
        }
    };
}
