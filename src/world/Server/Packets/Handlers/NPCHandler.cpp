/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Data/Flags.hpp"
#include "Logging/Logger.hpp"
#include "Management/AuctionHouse.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Server/Packets/ManagedPacket.h"
#include "Server/WorldSession.h"
#include "Server/Packets/MsgTabardvendorActivate.h"
#include "Server/Packets/CmsgBankerActivate.h"
#include "Server/Packets/SmsgShowBank.h"
#include "Server/Packets/MsgAuctionHello.h"
#include "Server/Packets/SmsgSpiritHealerConfirm.h"
#include "Server/Packets/CmsgTrainerBuySpell.h"
#include "Server/Packets/SmsgTrainerBuySucceeded.h"
#include "Server/Packets/SmsgPetitionShowlist.h"
#include "Server/Packets/CmsgPetitionShowlist.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Map/Management/MapMgr.hpp"
#include "Management/AuctionMgr.hpp"
#include "Management/ObjectMgr.hpp"
#include "Server/Packets/CmsgGossipSelectOption.h"
#include "Server/Packets/CmsgGossipHello.h"
#include "Management/ItemInterface.h"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Server/Packets/SmsgBinderConfirm.h"
#include "Server/Packets/CmsgTrainerList.h"
#include "Server/Packets/CmsgBinderActivate.h"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSessionLog.hpp"
#include "Server/Packets/MsgListStabledPets.h"
#include "Server/Packets/CmsgNpcTextQuery.h"
#include "Storage/MySQLDataStore.hpp"
#include "Spell/SpellMgr.hpp"
#include "Server/Packets/CmsgBuyBankSlot.h"
#include "Server/Packets/SmsgBuyBankSlotResult.h"
#include "Server/Packets/SmsgGossipComplete.h"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Storage/WorldStrings.h"
#include "Storage/WDB/WDBStructures.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleTabardVendorActivateOpcode(WorldPacket& recvPacket)
{
    MsgTabardvendorActivate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_TABARDVENDOR_ACTIVATE: {} (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    SendPacket(MsgTabardvendorActivate(srlPacket.guid).serialise().get());
}

//helper
void WorldSession::sendTabardHelp(Creature* creature)
{
    if (creature == nullptr)
        return;

    SendPacket(MsgTabardvendorActivate(creature->getGuid()).serialise().get());
}

void WorldSession::handleBankerActivateOpcode(WorldPacket& recvPacket)
{
    CmsgBankerActivate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_BANKER_ACTIVATE: {} (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    SendPacket(SmsgShowBank(srlPacket.guid).serialise().get());
}

//helper
void WorldSession::sendBankerList(Creature* creature)
{
    if (creature == nullptr)
        return;

    SendPacket(SmsgShowBank(creature->getGuid()).serialise().get());
}

void WorldSession::handleAuctionHelloOpcode(WorldPacket& recvPacket)
{
    MsgAuctionHello srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_AUCTION_HELLO: {} (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    sendAuctionList(creature);
}

//helper
void WorldSession::sendAuctionList(Creature* creature)
{
    if (creature == nullptr)
        return;

    const auto auctionHouse = sAuctionMgr.getAuctionHouse(creature->getEntry());
    if (auctionHouse == nullptr)
        return;

    SendPacket(MsgAuctionHello(creature->getGuid(), auctionHouse->getId(), auctionHouse->isEnabled ? 1U : 0U).serialise().get());
}

// helper
void WorldSession::sendSpiritHealerRequest(Creature* creature)
{
    SendPacket(SmsgSpiritHealerConfirm(creature->getGuid()).serialise().get());
}

void WorldSession::handleTrainerBuySpellOpcode(WorldPacket& recvPacket)
{
    CmsgTrainerBuySpell srlPacket;
    if (!srlPacket.deserialise((recvPacket)))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_TRAINER_BUY_SPELL: {} (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->getWorldMapCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    const auto trainer = creature->GetTrainer();
    if (trainer == nullptr)
        return;

    TrainerSpell const* trainerSpell = nullptr;

    auto its = sObjectMgr.getTrainerSpellSetById(trainer->spellset_id);

    for (auto& itr : *its)
    {
        if ((itr.castSpell && itr.castSpell->getId() == srlPacket.spellId) ||
            (itr.learnSpell && itr.learnSpell->getId() == srlPacket.spellId))
        {
            trainerSpell = &itr;
            break;
        }
    }

    if (trainerSpell == nullptr)
    {
        sCheatLog.writefromsession(this, "%s tried to learn none-obtainable spell - Possibly using WPE", _player->getName().c_str());
        Disconnect();
        return;
    }

    const auto spellStatus = trainerGetSpellStatus(trainerSpell);
    if (spellStatus == TRAINER_SPELL_RED || spellStatus == TRAINER_SPELL_GRAY)
        return;

    // teach the spell
    _player->modCoinage(-static_cast<int32_t>(trainerSpell->cost));
    creature->playSpellVisual(179, 0);

    if (trainerSpell->castSpell != nullptr)
    {
        _player->castSpell(_player, trainerSpell->castSpell->getId(), true);
    }
    else
    {
        _player->playSpellVisual(362, 1);

        if (trainerSpell->learnSpell != nullptr)
            _player->addSpell(trainerSpell->learnSpell->getId());
    }

    if (trainerSpell->deleteSpell)
    {
        if (trainerSpell->learnSpell)
            _player->removeSpell(trainerSpell->deleteSpell, true);
        else if (trainerSpell->castSpell)
            _player->removeSpell(trainerSpell->deleteSpell, true);
        else
            _player->removeSpell(trainerSpell->deleteSpell, true);
    }

    SendPacket(SmsgTrainerBuySucceeded(srlPacket.guid.getRawGuid(), srlPacket.spellId).serialise().get());
}

void WorldSession::handleCharterShowListOpcode(WorldPacket& recvPacket)
{
    CmsgPetitionShowlist srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_CHARTER_SHOW_LIST: {} (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    sendCharterRequest(creature);
}

//helper
void WorldSession::sendCharterRequest(Creature* creature)
{
    if (creature == nullptr)
        return;

    SendPacket(SmsgPetitionShowlist(creature->getGuid(), creature->isTabardDesigner()).serialise().get());
}

void WorldSession::handleGossipHelloOpcode(WorldPacket& recvPacket)
{
    CmsgGossipHello srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_GOSSIP_HELLO: {} (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature != nullptr)
    {
        // makes npc stop when for example on its waypoint path // aaron02
        creature->pauseMovement(30000);
        creature->SetSpawnLocation(creature->GetPosition());

        if (_player->isStealthed())
            _player->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);

        _player->onTalkReputation(creature->m_factionEntry);

        if (const auto script = GossipScript::getInterface(creature))
            script->onHello(creature, _player);
    }
}


void WorldSession::handleGossipSelectOptionOpcode(WorldPacket& recvPacket)
{
    CmsgGossipSelectOption srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_GOSSIP_SELECT_OPTION: {} (gossipId), {} (option), {} (guidLow)",
        srlPacket.gossip_id, srlPacket.option, srlPacket.guid.getGuidLow());


    GossipScript* script = nullptr;
    Object* object = nullptr;

    switch (srlPacket.guid.getHigh())
    {
        case HighGuid::Item:
        {
            if (const auto item = _player->getItemInterface()->GetItemByGUID(srlPacket.guid))
            {
                script = GossipScript::getInterface(item);
                object = item;
            }
        } break;
        case HighGuid::Unit:
        {
            if (const auto creature = dynamic_cast<Creature*>(_player->getWorldMap()->getObject(srlPacket.guid)))
            {
                script = GossipScript::getInterface(creature);
                object = creature;
            }
        } break;
        case HighGuid::GameObject:
        {
            if (const auto gameObject = dynamic_cast<GameObject*>(_player->getWorldMap()->getObject(srlPacket.guid)))
            {
                script = GossipScript::getInterface(gameObject);
                object = gameObject;
            }
        } break;
        default:
            break;
    }

    if (script && object)
    {
        if (srlPacket.input.length() > 0)
            script->onSelectOption(object, _player, srlPacket.option, srlPacket.input.c_str(), srlPacket.gossip_id);
        else
            script->onSelectOption(object, _player, srlPacket.option, nullptr, srlPacket.gossip_id);
    }
}

void WorldSession::handleBinderActivateOpcode(WorldPacket& recvPacket)
{
    CmsgBinderActivate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_BINDER_ACTIVATE: {} (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    sendInnkeeperBind(creature);
}

void WorldSession::sendInnkeeperBind(Creature* creature)
{
    // Check if the bind position is same as old bind position
    // but do not send error if player has no Hearthstone
    if (_player->hasItem(6948) && _player->isInRange(_player->getBindPosition(), 10.0f * 10.0f))
    {
        SendPacket(SmsgGossipComplete().serialise().get());

        // Send "already bound here" packet
        WorldPacket data(SMSG_PLAYERBINDERROR, 1);
        data << uint32_t(1);
        SendPacket(&data);
        return;
    }

    if (!_player->m_hasBindDialogOpen)
    {
        SendPacket(SmsgGossipComplete().serialise().get());

        SendPacket(SmsgBinderConfirm(creature->getGuid(), _player->getZoneId()).serialise().get());

        _player->m_hasBindDialogOpen = true;
        return;
    }

    _player->m_hasBindDialogOpen = false;
    SendPacket(SmsgGossipComplete().serialise().get());
    creature->castSpell(_player->getGuid(), 3286, true);
}

void WorldSession::handleTrainerListOpcode(WorldPacket& recvPacket)
{
    CmsgTrainerList srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    _player->onTalkReputation(creature->m_factionEntry);
    sendTrainerList(creature);
}

void WorldSession::handleStabledPetList(WorldPacket& recvPacket)
{
    MsgListStabledPets srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (_player->getClass() != HUNTER)
    {
        GossipMenu::sendSimpleMenu(srlPacket.guid, 13584, _player);
        return;
    }

    sendStabledPetList(srlPacket.guid);
}

void WorldSession::sendStabledPetList(uint64_t npcguid)
{
    std::map<uint8_t, PlayerStablePet> stableList;
    PlayerStablePet stablePet;

    for (const auto& [petId, cachedPet] : _player->getPetCacheMap())
    {
        stablePet.petNumber = petId;
        stablePet.entry = cachedPet->entry;
        stablePet.level = cachedPet->level;
        stablePet.name.assign(cachedPet->name);
        stableList.emplace(cachedPet->slot, stablePet);
    }

#if VERSION_STRING >= Cata
    // Since cata all stable slots are automatically unlocked
    SendPacket(MsgListStabledPets(npcguid, PET_SLOT_MAX_STABLE_SLOT, stableList).serialise().get());
#else
    SendPacket(MsgListStabledPets(npcguid, _player->m_stableSlotCount, stableList).serialise().get());
#endif
}

void WorldSession::sendTrainerList(Creature* creature)
{
    const auto trainer = creature->GetTrainer();
    if (trainer == nullptr)
        return;

    if (!_player->canTrainAt(trainer))
    {
        GossipMenu::sendSimpleMenu(creature->getGuid(), trainer->Cannot_Train_GossipTextId, _player);
        return;
    }

    std::string uiMessage;
    if (trainer->UIMessage == "DMSG")
        uiMessage = _player->getSession()->LocalizedWorldSrv(ServerString::SS_WHAT_CAN_I_TEACH_YOU);
    else
        uiMessage = trainer->UIMessage;

    const size_t size = 8 + 4 + 4 + 4 + uiMessage.size()
        + (sObjectMgr.getTrainerSpellSetById(trainer->spellset_id)->size() * (4 + 1 + 4 + 4 + 4 + 1 + 4 + 4 + 4 + 4 + 4));
    WorldPacket data(SMSG_TRAINER_LIST, size);

    data << creature->getGuid();
    data << uint32_t(trainer->TrainerType);

#if VERSION_STRING >= Cata
    data << uint32_t(1); // Unk
#endif

    size_t count_p = data.wpos();
    data << uint32_t(sObjectMgr.getTrainerSpellSetById(trainer->spellset_id)->size());

    uint32_t count = 0;

    auto its = sObjectMgr.getTrainerSpellSetById(trainer->spellset_id);

    for (auto& spellItr : *sObjectMgr.getTrainerSpellSetById(trainer->spellset_id))
    {
        auto trainerSpell = spellItr;

        const auto spellInfo = trainerSpell.castRealSpell != nullptr ? trainerSpell.castSpell : trainerSpell.learnSpell;
        if (spellInfo == nullptr)
            continue;

        if (!_player->isSpellFitByClassAndRace(spellInfo->getId()))
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

        data << uint32_t(spellInfo->getId());
        data << uint8_t(trainerGetSpellStatus(&trainerSpell));
        data << uint32_t(trainerSpell.cost);
#if VERSION_STRING < Cata
        data << uint32_t(0); // Unk
        data << uint32_t(trainerSpell.isPrimaryProfession);
#endif
        data << uint8_t(trainerSpell.requiredLevel);
        data << uint32_t(trainerSpell.requiredSkillLine);
        data << uint32_t(trainerSpell.requiredSkillLineValue);

        // Get the required spells to learn this spell
        uint8_t requiredSpellCount = 0;
        const auto maxRequiredCount = TrainerSpell::getMaxRequiredSpellCount();
        for (const auto requiredSpell : trainerSpell.requiredSpell)
        {
            if (requiredSpell == 0)
                continue;

            data << uint32_t(requiredSpell);
            ++requiredSpellCount;

            if (requiredSpellCount >= maxRequiredCount)
                break;

            const auto requiredSpells = sSpellMgr.getSpellsRequiredForSpellBounds(requiredSpell);
            for (auto itr = requiredSpells.first; itr != requiredSpells.second && requiredSpellCount <= maxRequiredCount; ++itr)
            {
                data << uint32_t(itr->second);
                ++requiredSpellCount;
            }

            if (requiredSpellCount >= maxRequiredCount)
                break;
        }

        while (requiredSpellCount < maxRequiredCount)
        {
            data << uint32_t(0);
            ++requiredSpellCount;
        }

#if VERSION_STRING >= Cata
        data << uint32_t(trainerSpell.isPrimaryProfession && _player->getFreePrimaryProfessionPoints() != 0);
        data << uint32_t(trainerSpell.isPrimaryProfession);
#endif
        ++count;
    }

    data.put<uint32_t>(count_p, count);
    data << uiMessage;

    sLogger.info("SendTrainerList : {} TrainerSpells in list", count);

    SendPacket(&data);
}

TrainerSpellState WorldSession::trainerGetSpellStatus(TrainerSpell const* trainerSpell) const
{
    if (trainerSpell == nullptr)
        return TRAINER_SPELL_RED;

    if (trainerSpell->castSpell == nullptr && trainerSpell->learnSpell == nullptr)
        return TRAINER_SPELL_RED;

    if (trainerSpell->learnSpell != nullptr && (_player->hasSpell(trainerSpell->learnSpell->getId()) || _player->hasDeletedSpell(trainerSpell->learnSpell->getId())))
        return TRAINER_SPELL_GRAY;

    if (trainerSpell->castRealSpell != nullptr && (_player->hasSpell(trainerSpell->castRealSpell->getId()) || _player->hasDeletedSpell(trainerSpell->castRealSpell->getId())))
        return TRAINER_SPELL_GRAY;

    if (trainerSpell->deleteSpell != 0 && _player->hasDeletedSpell(trainerSpell->deleteSpell))
        return TRAINER_SPELL_GRAY;

    if (trainerSpell->requiredLevel && _player->getLevel() < trainerSpell->requiredLevel)
        return TRAINER_SPELL_RED;

    if (trainerSpell->requiredSkillLine && _player->getSkillLineCurrent(trainerSpell->requiredSkillLine, true) < trainerSpell->requiredSkillLineValue)
        return TRAINER_SPELL_RED;

    if (trainerSpell->cost != 0 && !_player->hasEnoughCoinage(trainerSpell->cost))
        return TRAINER_SPELL_RED;

    for (const auto spellId : trainerSpell->requiredSpell)
    {
        if (spellId == 0)
            continue;

        if (!_player->hasSpell(spellId))
            return TRAINER_SPELL_RED;

        const auto spellsRequired = sSpellMgr.getSpellsRequiredForSpellBounds(spellId);
        for (auto itr = spellsRequired.first; itr != spellsRequired.second; ++itr)
        {
            if (!_player->hasSpell(itr->second))
                return TRAINER_SPELL_RED;
        }
    }

    return TRAINER_SPELL_GREEN;
}

void WorldSession::handleSpiritHealerActivateOpcode(WorldPacket& /*recvPacket*/)
{
    if (!_player->isDead())
        return;

    _player->calcDeathDurabilityLoss(0.25);
    _player->resurrect();

    if (_player->getLevel() > 10)
    {
        const auto aura = _player->getAuraWithId(15007);
        if (aura == nullptr)
        {
            const auto spellInfo = sSpellMgr.getSpellInfo(15007);
            SpellCastTargets targets(_player->getGuid());
            const auto spell = sSpellMgr.newSpell(_player, spellInfo, true, nullptr);
            spell->prepare(&targets);
        }

        uint32_t duration = 600000;

        if (_player->getLevel() < 20)
            duration = (_player->getLevel() - 10) * 60000;

        if (const auto aur = _player->getAuraWithId(15007))
            aur->setNewMaxDuration(duration);
    }

    _player->setHealth(_player->getMaxHealth() / 2);
}

void WorldSession::handleNpcTextQueryOpcode(WorldPacket& recvPacket)
{
    CmsgNpcTextQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debug("Received: CMSG_NPC_TEXT_QUERY: {} (textId)", srlPacket.text_id);

    _player->setTargetGuid(srlPacket.guid);

    const auto localesNpcText = (language > 0) ? sMySQLStore.getLocalizedNpcGossipText(srlPacket.text_id, language) : nullptr;

    WorldPacket data;
    data.Initialize(SMSG_NPC_TEXT_UPDATE);
    data << srlPacket.text_id;

    if (const auto pGossip = sMySQLStore.getNpcGossipText(srlPacket.text_id))
    {
        for (uint8_t i = 0; i < 8; ++i)
        {
            data << float(pGossip->textHolder[i].probability);

            if (localesNpcText)
            {
                if (strlen(localesNpcText->texts[i][0]) == 0)
                    data << localesNpcText->texts[i][1];
                else
                    data << localesNpcText->texts[i][0];

                if (strlen(localesNpcText->texts[i][1]) == 0)
                    data << localesNpcText->texts[i][0];
                else
                    data << localesNpcText->texts[i][1];
            }
            else
            {
                if (pGossip->textHolder[i].texts[0].empty())
                    data << pGossip->textHolder[i].texts[1];
                else
                    data << pGossip->textHolder[i].texts[0];

                if (pGossip->textHolder[i].texts[1].empty())
                    data << pGossip->textHolder[i].texts[0];
                else
                    data << pGossip->textHolder[i].texts[1];
            }

            data << pGossip->textHolder[i].language;

            for (uint8_t e = 0; e < GOSSIP_EMOTE_COUNT; ++e)
            {
                data << uint32_t(pGossip->textHolder[i].gossipEmotes[e].delay);
                data << uint32_t(pGossip->textHolder[i].gossipEmotes[e].emote);
            }
        }
    }
    else
    {
        for (uint8_t i = 0; i < 8; ++i)
        {
            data << float(1.0f);              // Prob
            data << _player->getSession()->LocalizedWorldSrv(ServerString::SS_HEY_HOW_CAN_I_HELP_YOU);
            data << _player->getSession()->LocalizedWorldSrv(ServerString::SS_HEY_HOW_CAN_I_HELP_YOU);
            data << uint32_t(0x00);           // Language

            for (uint8_t e = 0; e < GOSSIP_EMOTE_COUNT; e++)
            {
                data << uint32_t(0x00);       // Emote delay
                data << uint32_t(0x00);       // Emote
            }
        }
    }

    SendPacket(&data);
}

namespace BankslotError
{
    enum
    {
        TooMany = 0,
        InsufficientFunds = 1,
        NotABanker = 2
    };
}

void WorldSession::handleBuyBankSlotOpcode(WorldPacket& recvPacket)
{
    CmsgBuyBankSlot srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_BUY_BANK_SLOT: {} (guidLow)", srlPacket.guid.getGuidLow());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr || !creature->isBanker())
    {
        SendPacket(SmsgBuyBankSlotResult(BankslotError::NotABanker).serialise().get());
        return;
    }

    const uint8_t slots = _player->getBankSlots() + 1U;
    const auto bank_bag_slot_prices = sBankBagSlotPricesStore.lookupEntry(slots);
    if (bank_bag_slot_prices == nullptr)
    {
        SendPacket(SmsgBuyBankSlotResult(BankslotError::TooMany).serialise().get());
        return;
    }

    const auto price = bank_bag_slot_prices->Price;
    if (!_player->hasEnoughCoinage(price))
    {
        SendPacket(SmsgBuyBankSlotResult(BankslotError::InsufficientFunds).serialise().get());
        return;
    }

    _player->setBankSlots(slots);
    _player->modCoinage(-static_cast<int32_t>(price));
#if VERSION_STRING > TBC
    _player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT, 1, 0, 0);
#endif

}
