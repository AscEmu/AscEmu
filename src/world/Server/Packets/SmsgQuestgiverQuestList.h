/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    struct QuestListEntry
    {
        uint32_t questId = 0;
        uint32_t statusIcon = 0; // resolved status->icon mapping (4/0/2, see QuestMgr)
        int32_t questLevel = 0;
        uint32_t questFlags = 0;  // >= WotLK only
        bool isRepeatable = false; // >= WotLK only
        std::string title;
    };

    // Populated by QuestMgr::OnActivateQuestGiver, which owns the quest-relation iteration,
    // status computation (CalcQuestStatus) and dedup rules.
    struct QuestgiverQuestListInput
    {
        uint64_t questGiverGuid = 0;
        std::string greeting; // "" for a gameobject quest giver, else the localized hello line
        bool isValid = false; // false if the quest giver has no quest relations at all
        uint8_t activeQuestsCount = 0;
        std::vector<QuestListEntry> quests;
    };

    class SmsgQuestgiverQuestList : public ManagedPacket
    {
    public:
        QuestgiverQuestListInput input;

        SmsgQuestgiverQuestList() : SmsgQuestgiverQuestList(QuestgiverQuestListInput{})
        {
        }

        explicit SmsgQuestgiverQuestList(QuestgiverQuestListInput input) :
            ManagedPacket(SMSG_QUESTGIVER_QUEST_LIST, 0),
            input(std::move(input))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 100;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // Quests with !input.isValid naturally serialise as an empty (count == 0) list,
                // matching a genuinely quest-less giver.
                const WoWGuid guid(input.questGiverGuid);

                packet << uint32_t(1); // Emote
                packet << uint32_t(1); // Delay

                packet.writeBit(guid[2]);
                packet.writeBits(input.greeting.length(), 11);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[0]);

                const size_t countPos = packet.bitwpos();
                packet.writeBits(0, 19); // quest count, patched below

                ByteBuffer questData;
                for (const auto& quest : input.quests)
                {
                    packet.writeBit(quest.isRepeatable);
                    packet.writeBits(quest.title.length(), 9);

                    questData << uint32_t(quest.questFlags);
                    questData << uint32_t(quest.questId);
                    questData.writeString(quest.title);
                    questData << uint32_t(0); // Flags2 - not modelled by AscEmu's QuestProperties
                    questData << uint32_t(quest.statusIcon);
                    questData << uint32_t(quest.questLevel);
                }

                packet.writeBit(guid[1]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[7]);

                packet.putBits(countPos, input.quests.size(), 19);
                packet.flushBits();

                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[7]);
                packet.append(questData);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[2]);
                packet.writeString(input.greeting);
                packet.writeByteSeq(guid[4]);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << input.questGiverGuid;
                packet << input.greeting;

                packet << uint32_t(1); // Emote Delay
                packet << uint32_t(1); // Emote

                if (!input.isValid)
                {
                    packet << uint8_t(0);
                    return true;
                }

                packet << uint8_t(input.activeQuestsCount);

                for (const auto& quest : input.quests)
                {
                    packet << quest.questId;
                    packet << quest.statusIcon;
                    packet << int32_t(quest.questLevel);

                    if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                    {
                        packet << uint32_t(quest.questFlags);
                        packet << uint8_t(quest.isRepeatable);
                    }

                    packet << quest.title;
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
