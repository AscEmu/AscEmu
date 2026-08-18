/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Logging/Logger.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "WoWGuid.hpp"

#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class SmsgTrainerList : public ManagedPacket
    {
    public:
        Creature* creature;
        Player* player;
        std::string uiMessage;

        SmsgTrainerList() : SmsgTrainerList(nullptr, nullptr, "")
        {
        }

        SmsgTrainerList(Creature* creature, Player* player, std::string uiMessage) :
            ManagedPacket(SMSG_TRAINER_LIST, 0),
            creature(creature),
            player(player),
            uiMessage(std::move(uiMessage))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (creature == nullptr)
                return 0;

            const auto trainer = creature->GetTrainer();
            if (trainer == nullptr)
                return 0;

            if (m_protocol.isMop())
                return 8 + 4 + 1 + uiMessage.size()
                    + (sObjectMgr.getTrainerSpellSetById(trainer->spellset_id)->size() * (1 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 1));

            return 8 + 4 + 4 + 4 + uiMessage.size()
                + (sObjectMgr.getTrainerSpellSetById(trainer->spellset_id)->size() * (4 + 1 + 4 + 4 + 4 + 1 + 4 + 4 + 4 + 4 + 4));
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (creature == nullptr || player == nullptr)
                return false;

            const auto trainer = creature->GetTrainer();
            if (trainer == nullptr)
                return false;

            if (m_protocol.isMop())
            {
                const WoWGuid guid = creature->getGuid();

                packet.writeBit(guid[4]);
                packet.writeBit(guid[5]);

                const size_t countBitPos = packet.bitwpos();
                packet.writeBits(0, 19); // placeholder, patched below

                packet.writeBits(uiMessage.size(), 11);

                packet.writeBit(guid[6]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[0]);
                packet.flushBits();

                packet.writeByteSeq(guid[4]);

                uint32_t count = 0;

                for (auto& spellItr : *sObjectMgr.getTrainerSpellSetById(trainer->spellset_id))
                {
                    auto trainerSpell = spellItr;

                    const auto spellInfo = trainerSpell.castRealSpell != nullptr ? trainerSpell.castSpell : trainerSpell.learnSpell;
                    if (spellInfo == nullptr)
                        continue;

                    if (!player->isSpellFitByClassAndRace(spellInfo->getId()))
                        continue;

                    if (spellItr.isStatic == 0)
                    {
                        // trainer has max level to train, skip all spells higher.
                        if (trainer->can_train_max_level)
                            if (spellItr.requiredLevel > trainer->can_train_max_level)
                                continue;

                        // trainer has min_skill_value, skip all spells lower
                        if (trainer->can_train_min_skill_value)
                            if (spellItr.requiredSkillLineValue < trainer->can_train_min_skill_value)
                                continue;

                        // trainer has max_skill_value, skip all spells higher
                        if (trainer->can_train_max_skill_value)
                            if (spellItr.requiredSkillLineValue > trainer->can_train_max_skill_value)
                                continue;
                    }

                    packet << static_cast<uint8_t>(trainerSpell.requiredLevel);
                    packet << static_cast<uint32_t>(trainerSpell.cost); // reputation multiplier
                    packet << static_cast<uint32_t>(spellInfo->getId());

                    uint8_t requiredSpellCount = 0;
                    constexpr uint8_t maxRequiredCount = 3;
                    for (const auto requiredSpell : trainerSpell.requiredSpell)
                    {
                        if (requiredSpell == 0)
                            continue;

                        packet << static_cast<uint32_t>(requiredSpell);
                        ++requiredSpellCount;

                        if (requiredSpellCount >= maxRequiredCount)
                            break;

                        const auto requiredSpells = sSpellMgr.getSpellsRequiredRangeForSpell(requiredSpell);
                        for (const auto& itr : requiredSpells)
                        {
                            packet << static_cast<uint32_t>(itr.second);
                            ++requiredSpellCount;

                            if (requiredSpellCount >= maxRequiredCount)
                                break;
                        }

                        if (requiredSpellCount >= maxRequiredCount)
                            break;
                    }

                    while (requiredSpellCount < maxRequiredCount)
                    {
                        packet << static_cast<uint32_t>(0);
                        ++requiredSpellCount;
                    }

                    packet << static_cast<uint32_t>(trainerSpell.requiredSkillLine);
                    packet << static_cast<uint32_t>(trainerSpell.requiredSkillLineValue);
                    packet << static_cast<uint8_t>(player->getSession()->trainerGetSpellStatus(&trainerSpell));

                    ++count;
                }

                packet.writeString(uiMessage);

                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[3]);

                packet << static_cast<uint32_t>(1); // Unk

                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[2]);

                packet << static_cast<uint32_t>(trainer->TrainerType);

                packet.putBits(countBitPos, count, 19);

                sLogger.info("SendTrainerList : {} TrainerSpells in list", count);

                return true;
            }
            else
            {
                packet << creature->getGuid();
                packet << static_cast<uint32_t>(trainer->TrainerType);

                if (m_protocol.expansion >= WoW::Expansion::_Cata)
                    packet << static_cast<uint32_t>(1); // Unk

                size_t count_p = packet.wpos();
                packet << static_cast<uint32_t>(sObjectMgr.getTrainerSpellSetById(trainer->spellset_id)->size());

                uint32_t count = 0;

                for (auto& spellItr : *sObjectMgr.getTrainerSpellSetById(trainer->spellset_id))
                {
                    auto trainerSpell = spellItr;

                    const auto spellInfo = trainerSpell.castRealSpell != nullptr ? trainerSpell.castSpell : trainerSpell.learnSpell;
                    if (spellInfo == nullptr)
                        continue;

                    if (!player->isSpellFitByClassAndRace(spellInfo->getId()))
                        continue;

                    if (spellItr.isStatic == 0)
                    {
                        // trainer has max level to train, skip all spells higher.
                        if (trainer->can_train_max_level)
                            if (spellItr.requiredLevel > trainer->can_train_max_level)
                                continue;

                        // trainer has min_skill_value, skip all spells lower
                        if (trainer->can_train_min_skill_value)
                            if (spellItr.requiredSkillLineValue < trainer->can_train_min_skill_value)
                                continue;

                        // trainer has max_skill_value, skip all spells higher
                        if (trainer->can_train_max_skill_value)
                            if (spellItr.requiredSkillLineValue > trainer->can_train_max_skill_value)
                                continue;
                    }

                    packet << static_cast<uint32_t>(spellInfo->getId());
                    packet << static_cast<uint8_t>(player->getSession()->trainerGetSpellStatus(&trainerSpell));
                    packet << static_cast<uint32_t>(trainerSpell.cost);
                    if (m_protocol.expansion < WoW::Expansion::_Cata)
                    {
                        packet << uint32_t(0); // Unk
                        packet << uint32_t(trainerSpell.isPrimaryProfession);
                    }
                    packet << static_cast<uint8_t>(trainerSpell.requiredLevel);
                    packet << static_cast<uint32_t>(trainerSpell.requiredSkillLine);
                    packet << static_cast<uint32_t>(trainerSpell.requiredSkillLineValue);

                    // Get the required spells to learn this spell
                    uint8_t requiredSpellCount = 0;
                    const auto maxRequiredCount = TrainerSpell::getMaxRequiredSpellCount();
                    for (const auto requiredSpell : trainerSpell.requiredSpell)
                    {
                        if (requiredSpell == 0)
                            continue;

                        packet << static_cast<uint32_t>(requiredSpell);
                        ++requiredSpellCount;

                        if (requiredSpellCount >= maxRequiredCount)
                            break;

                        const auto requiredSpells = sSpellMgr.getSpellsRequiredRangeForSpell(requiredSpell);
                        for (const auto& itr : requiredSpells)
                        {
                            packet << static_cast<uint32_t>(itr.second);
                            ++requiredSpellCount;

                            if (requiredSpellCount > maxRequiredCount)
                                break;
                        }

                        if (requiredSpellCount >= maxRequiredCount)
                            break;
                    }

                    while (requiredSpellCount < maxRequiredCount)
                    {
                        packet << static_cast<uint32_t>(0);
                        ++requiredSpellCount;
                    }

                    if (m_protocol.expansion >= WoW::Expansion::_Cata)
                    {
                        packet << static_cast<uint32_t>(trainerSpell.isPrimaryProfession && player->getFreePrimaryProfessionPoints() != 0);
                        packet << static_cast<uint32_t>(trainerSpell.isPrimaryProfession);
                    }
                    ++count;
                }

                packet.put<uint32_t>(count_p, count);
                packet << uiMessage;

                sLogger.info("SendTrainerList : {} TrainerSpells in list", count);

                return true;
            }
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
