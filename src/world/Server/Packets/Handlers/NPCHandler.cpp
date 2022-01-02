/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


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
#include "Map/MapMgr.h"
#include "Management/AuctionMgr.h"
#include "Server/MainServerDefines.h"
#include "Management/ObjectMgr.h"
#include "Server/Packets/CmsgGossipSelectOption.h"
#include "Server/Packets/CmsgGossipHello.h"
#include "Management/ItemInterface.h"
#include "Management/Gossip/GossipScript.hpp"
#include "Server/Packets/SmsgBinderConfirm.h"
#include "Server/Packets/CmsgTrainerList.h"
#include "Server/Packets/CmsgBinderActivate.h"
#include "Objects/Units/Creatures/Pet.h"
#include "Server/Packets/MsgListStabledPets.h"
#include "Server/Packets/CmsgNpcTextQuery.h"
#include "Storage/MySQLDataStore.hpp"
#include "Spell/SpellMgr.hpp"
#include "Server/Packets/CmsgBuyBankSlot.h"
#include "Server/Packets/SmsgBuyBankSlotResult.h"
#include "Server/Packets/SmsgGossipComplete.h"
#include "Storage/WorldStrings.h"

using namespace AscEmu::Packets;

void WorldSession::handleTabardVendorActivateOpcode(WorldPacket& recvPacket)
{
    MsgTabardvendorActivate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_TABARDVENDOR_ACTIVATE: %u (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
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

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_BANKER_ACTIVATE: %u (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
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

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_AUCTION_HELLO: %u (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    sendAuctionList(creature);
}

//helper
void WorldSession::sendAuctionList(Creature* creature)
{
    if (creature == nullptr)
        return;

    const auto auctionHouse = sAuctionMgr.GetAuctionHouse(creature->getEntry());
    if (auctionHouse == nullptr)
        return;

    SendPacket(MsgAuctionHello(creature->getGuid(), auctionHouse->getId(), auctionHouse->isEnabled ? 1 : 0).serialise().get());
}

//helper
void WorldSession::sendSpiritHealerRequest(Creature* creature)
{
    SendPacket(SmsgSpiritHealerConfirm(creature->getGuid()).serialise().get());
}

void WorldSession::handleTrainerBuySpellOpcode(WorldPacket& recvPacket)
{
    CmsgTrainerBuySpell srlPacket;
    if (!srlPacket.deserialise((recvPacket)))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_TRAINER_BUY_SPELL: %u (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->GetMapMgrCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    const auto trainer = creature->GetTrainer();
    if (trainer == nullptr)
        return;

    TrainerSpell const* trainerSpell = nullptr;
    for (const auto itr : trainer->Spells)
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

    if (trainerSpell->castSpell != nullptr)
    {
        _player->castSpell(_player, trainerSpell->castSpell->getId(), true);
    }
    else
    {
        creature->playSpellVisual(179, 0);
        _player->playSpellVisual(362, 1);

        if (trainerSpell->learnSpell != nullptr)
            _player->addSpell(trainerSpell->learnSpell->getId());
    }

    if (trainerSpell->deleteSpell)
    {
        if (trainerSpell->learnSpell)
            _player->removeSpell(trainerSpell->deleteSpell, true, true, trainerSpell->learnSpell->getId());
        else if (trainerSpell->castSpell)
            _player->removeSpell(trainerSpell->deleteSpell, true, true, trainerSpell->castSpell->getId());
        else
            _player->removeSpell(trainerSpell->deleteSpell, true, false, 0);
    }

    SendPacket(SmsgTrainerBuySucceeded(srlPacket.guid.getRawGuid(), srlPacket.spellId).serialise().get());
}

void WorldSession::handleCharterShowListOpcode(WorldPacket& recvPacket)
{
    CmsgPetitionShowlist srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_CHARTER_SHOW_LIST: %u (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
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

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_GOSSIP_HELLO: %u (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
    if (creature != nullptr)
    {
        // makes npc stop when for example on its waypoint path // aaron02
        creature->pauseMovement(30000);
        creature->SetSpawnLocation(creature->GetPosition());

        if (_player->isStealthed())
            _player->RemoveAllAuraType(SPELL_AURA_MOD_STEALTH);

        _player->Reputation_OnTalk(creature->m_factionEntry);

        if (const auto script = GossipScript::getInterface(creature))
            script->onHello(creature, _player);
    }
}


void WorldSession::handleGossipSelectOptionOpcode(WorldPacket& recvPacket)
{
    CmsgGossipSelectOption srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_GOSSIP_SELECT_OPTION: %u (gossipId), %i (option), %u (guidLow)",
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
            if (const auto creature = dynamic_cast<Creature*>(_player->GetMapMgr()->_GetObject(srlPacket.guid)))
            {
                script = GossipScript::getInterface(creature);
                object = creature;
            }
        } break;
        case HighGuid::GameObject:
        {
            if (const auto gameObject = dynamic_cast<GameObject*>(_player->GetMapMgr()->_GetObject(srlPacket.guid)))
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

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_BINDER_ACTIVATE: %u (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    sendInnkeeperBind(creature);
}

void WorldSession::sendInnkeeperBind(Creature* creature)
{
    // Check if the bind position is same as old bind position
    if (_player->isInRange(_player->getBindPosition(), 10.0f * 10.0f))
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

        SendPacket(SmsgBinderConfirm(creature->getGuid(), _player->GetZoneId()).serialise().get());

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

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    _player->Reputation_OnTalk(creature->m_factionEntry);
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
    std::vector<PlayerStablePetList> stableList;
    PlayerStablePetList stablePet;

    for (const auto itr : _player->m_Pets)
    {
        stablePet.petNumber = itr.first;
        stablePet.entry = itr.second->entry;
        stablePet.level = itr.second->level;
        stablePet.name = itr.second->name;
        if (itr.second->stablestate == STABLE_STATE_ACTIVE)
            stablePet.stableState = STABLE_STATE_ACTIVE;
        else
            stablePet.stableState = STABLE_STATE_PASSIVE + 1;

        stableList.push_back(stablePet);
    }

    SendPacket(MsgListStabledPets(npcguid, static_cast<uint8_t>(_player->m_Pets.size()), _player->m_StableSlotCount, stableList).serialise().get());
}

void WorldSession::sendTrainerList(Creature* creature)
{
    const auto trainer = creature->GetTrainer();
    if (trainer == nullptr)
        return;

    if (!_player->CanTrainAt(trainer))
    {
        GossipMenu::sendSimpleMenu(creature->getGuid(), trainer->Cannot_Train_GossipTextId, _player);
        return;
    }

    std::string uiMessage;
    if (stricmp(trainer->UIMessage, "DMSG") == 0)
        uiMessage = _player->GetSession()->LocalizedWorldSrv(ServerString::SS_WHAT_CAN_I_TEACH_YOU);
    else
        uiMessage = trainer->UIMessage;

    const size_t size = 8 + 4 + 4 + 4 + uiMessage.size()
        + (trainer->Spells.size() * (4 + 1 + 4 + 4 + 4 + 1 + 4 + 4 + 4 + 4 + 4));
    WorldPacket data(SMSG_TRAINER_LIST, size);

    data << creature->getGuid();
    data << uint32_t(trainer->TrainerType);

#if VERSION_STRING >= Cata
    data << uint32_t(1); // Unk
#endif

    size_t count_p = data.wpos();
    data << uint32_t(trainer->Spells.size());

    uint32_t count = 0;
    for (const auto spellItr : trainer->Spells)
    {
        auto* const trainerSpell = &spellItr;

        const auto spellInfo = trainerSpell->castRealSpell != nullptr ? trainerSpell->castSpell : trainerSpell->learnSpell;
        if (spellInfo == nullptr)
            continue;

        if (!_player->isSpellFitByClassAndRace(spellInfo->getId()))
            continue;

        data << uint32_t(spellInfo->getId());
        data << uint8_t(trainerGetSpellStatus(trainerSpell));
        data << uint32_t(trainerSpell->cost);
#if VERSION_STRING < Cata
        data << uint32_t(0); // Unk
        data << uint32_t(trainerSpell->isPrimaryProfession);
#endif
        data << uint8_t(trainerSpell->requiredLevel);
        data << uint32_t(trainerSpell->requiredSkillLine);
        data << uint32_t(trainerSpell->requiredSkillLineValue);

        // Get the required spells to learn this spell
        uint8_t requiredSpellCount = 0;
        const auto maxRequiredCount = TrainerSpell::getMaxRequiredSpellCount();
        for (uint8_t i = 0; i < maxRequiredCount; ++i)
        {
            if (trainerSpell->requiredSpell[i] == 0)
                continue;

            data << uint32_t(trainerSpell->requiredSpell[i]);
            ++requiredSpellCount;

            if (requiredSpellCount >= maxRequiredCount)
                break;

            const auto requiredSpells = sObjectMgr.GetSpellsRequiredForSpellBounds(trainerSpell->requiredSpell[i]);
            for (auto itr2 = requiredSpells.first; itr2 != requiredSpells.second && requiredSpellCount <= maxRequiredCount; ++itr2)
            {
                data << uint32_t(itr2->second);
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
        data << uint32_t(trainerSpell->isPrimaryProfession && _player->getFreePrimaryProfessionPoints() != 0);
        data << uint32_t(trainerSpell->isPrimaryProfession);
#endif
        ++count;
    }

    data.put<uint32_t>(count_p, count);
    data << uiMessage;

    SendPacket(&data);
}

TrainerSpellState WorldSession::trainerGetSpellStatus(TrainerSpell const* trainerSpell) const
{
    if (trainerSpell == nullptr)
        return TRAINER_SPELL_RED;

    if (trainerSpell->castSpell == nullptr && trainerSpell->learnSpell == nullptr)
        return TRAINER_SPELL_RED;

    if (trainerSpell->learnSpell != nullptr && (_player->HasSpell(trainerSpell->learnSpell->getId()) || _player->HasDeletedSpell(trainerSpell->learnSpell->getId())))
        return TRAINER_SPELL_GRAY;

    if (trainerSpell->castRealSpell != nullptr && (_player->HasSpell(trainerSpell->castRealSpell->getId()) || _player->HasDeletedSpell(trainerSpell->castRealSpell->getId())))
        return TRAINER_SPELL_GRAY;

    if (trainerSpell->deleteSpell != 0 && _player->HasDeletedSpell(trainerSpell->deleteSpell))
        return TRAINER_SPELL_GRAY;

    if (trainerSpell->requiredLevel && _player->getLevel() < trainerSpell->requiredLevel)
        return TRAINER_SPELL_RED;

    if (trainerSpell->requiredSkillLine && _player->_GetSkillLineCurrent(trainerSpell->requiredSkillLine, true) < trainerSpell->requiredSkillLineValue)
        return TRAINER_SPELL_RED;

    if (trainerSpell->cost != 0 && !_player->hasEnoughCoinage(trainerSpell->cost))
        return TRAINER_SPELL_RED;

    for (const auto spellId : trainerSpell->requiredSpell)
    {
        if (spellId == 0)
            continue;

        const auto spellsRequired = sObjectMgr.GetSpellsRequiredForSpellBounds(spellId);
        for (auto itr = spellsRequired.first; itr != spellsRequired.second; ++itr)
        {
            if (!_player->HasSpell(itr->second))
                return TRAINER_SPELL_RED;
        }
    }

    return TRAINER_SPELL_GREEN;
}

void WorldSession::handleSpiritHealerActivateOpcode(WorldPacket& /*recvPacket*/)
{
    if (!_player->isDead())
        return;

    _player->DeathDurabilityLoss(0.25);
    _player->ResurrectPlayer();

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

        int32_t duration = 600000;

        if (_player->getLevel() < 20)
            duration = (_player->getLevel() - 10) * 60000;

        _player->SetAurDuration(15007, duration);
    }

    _player->setHealth(_player->getMaxHealth() / 2);
}

void WorldSession::handleNpcTextQueryOpcode(WorldPacket& recvPacket)
{
    CmsgNpcTextQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debug("Received: CMSG_NPC_TEXT_QUERY: %u (textId)", srlPacket.text_id);

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
            data << _player->GetSession()->LocalizedWorldSrv(ServerString::SS_HEY_HOW_CAN_I_HELP_YOU);
            data << _player->GetSession()->LocalizedWorldSrv(ServerString::SS_HEY_HOW_CAN_I_HELP_YOU);
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

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_BUY_BANK_SLOT: %u (guidLow)", srlPacket.guid.getGuidLow());

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr || !creature->isBanker())
    {
        SendPacket(SmsgBuyBankSlotResult(BankslotError::NotABanker).serialise().get());
        return;
    }

    const uint8_t slots = _player->getBankSlots();
    const auto bank_bag_slot_prices = sBankBagSlotPricesStore.LookupEntry(slots + 1);
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

    _player->setBankSlots(slots + 1);
    _player->modCoinage(-static_cast<int32_t>(price));
#if VERSION_STRING > TBC
    _player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT, 1, 0, 0);
#endif

}
