/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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
#include "Server/Packets/SmsgPlayerbinderror.h"
#include "Server/Packets/SmsgTrainerList.h"
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
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_TABARDVENDOR_ACTIVATE: {} (guidLowPart).", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    MsgTabardvendorActivate managedPacket(srlPacket.guid);
    sendManagedPacket(managedPacket);
}

// helper
void WorldSession::sendTabardHelp(Creature* creature)
{
    if (creature == nullptr)
        return;

    MsgTabardvendorActivate managedPacket(creature->getGuid());
    sendManagedPacket(managedPacket);
}

void WorldSession::handleBankerActivateOpcode(WorldPacket& recvPacket)
{
    CmsgBankerActivate srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_BANKER_ACTIVATE: {} (guidLowPart).", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    SmsgShowBank managedPacket(srlPacket.guid);
    sendManagedPacket(managedPacket);
}

// helper
void WorldSession::sendBankerList(Creature* creature)
{
    if (creature == nullptr)
        return;

    SmsgShowBank managedPacket(creature->getGuid());
    sendManagedPacket(managedPacket);
}

void WorldSession::handleAuctionHelloOpcode(WorldPacket& recvPacket)
{
    MsgAuctionHello srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_AUCTION_HELLO: {} (guidLowPart).", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    sendAuctionList(creature);
}

// helper
void WorldSession::sendAuctionList(Creature* creature)
{
    if (creature == nullptr)
        return;

    const auto auctionHouse = sAuctionMgr.getAuctionHouse(creature->getEntry());
    if (auctionHouse == nullptr)
        return;

    MsgAuctionHello sendPacket(creature->getGuid(), auctionHouse->getId(), auctionHouse->isEnabled ? 1U : 0U);
    sendManagedPacket(sendPacket);
}

// helper
void WorldSession::sendSpiritHealerRequest(Creature* creature)
{
    SmsgSpiritHealerConfirm managedPacket(creature->getGuid());
    sendManagedPacket(managedPacket);
}

void WorldSession::handleTrainerBuySpellOpcode(WorldPacket& recvPacket)
{
    CmsgTrainerBuySpell srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_TRAINER_BUY_SPELL: {} (guidLowPart).", srlPacket.guid.getGuidLowPart());

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
        if ((itr.castSpell && itr.castSpell->getId() == srlPacket.spellId) || (itr.learnSpell && itr.learnSpell->getId() == srlPacket.spellId))
        {
            trainerSpell = &itr;
            break;
        }
    }

    if (trainerSpell == nullptr)
    {
        sCheatLog.writefromsession(this, "Attempted to learn non-obtainable spell. Player: {}.", _player->getName());
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

    // Mop has no SMSG_TRAINER_BUY_SUCCEEDED equivalent - the client infers
    // success from the resulting spell-learn/gold updates alone.
    if (!srlPacket.getClientProtocol().isMop())
    {
        SmsgTrainerBuySucceeded managedPacket(srlPacket.guid.getRawGuid(), srlPacket.spellId);
        sendManagedPacket(managedPacket);
    }
}

void WorldSession::handleCharterShowListOpcode(WorldPacket& recvPacket)
{
    CmsgPetitionShowlist srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_CHARTER_SHOW_LIST: {} (guidLowPart).", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    sendCharterRequest(creature);
}

// helper
void WorldSession::sendCharterRequest(Creature* creature)
{
    if (creature == nullptr)
        return;

    SmsgPetitionShowlist managedPacket(creature->getGuid(), creature->isTabardDesigner());
    sendManagedPacket(managedPacket);
}

void WorldSession::handleGossipHelloOpcode(WorldPacket& recvPacket)
{
    CmsgGossipHello srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_GOSSIP_HELLO: {} (guidLowPart).", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature != nullptr)
    {
        // makes npc stop when for example on its waypoint path // aaron02
        creature->pauseMovement(30000);
        creature->SetSpawnLocation(creature->GetPosition());

        if (_player->isStealthed())
            _player->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);

        _player->onTalkReputation(creature->getServersideFactionEntry());

        if (const auto script = GossipScript::getInterface(creature))
            script->onHello(creature, _player);
    }
}


void WorldSession::handleGossipSelectOptionOpcode(WorldPacket& recvPacket)
{
    CmsgGossipSelectOption srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_GOSSIP_SELECT_OPTION: {} (gossipId), {} (option), {} (guidLow).",
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
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_BINDER_ACTIVATE: {} (guidLowPart).", srlPacket.guid.getGuidLowPart());

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
        SmsgGossipComplete managedPacket;
        sendManagedPacket(managedPacket);

        // Send "already bound here" packet
        SmsgPlayerbinderror errorPacket(1);
        sendManagedPacket(errorPacket);
        return;
    }

    if (!_player->m_hasBindDialogOpen)
    {
        SmsgGossipComplete managedPacket;
        sendManagedPacket(managedPacket);

        SmsgBinderConfirm managedbinderPacket(creature->getGuid(), _player->getZoneId());
        sendManagedPacket(managedbinderPacket);

        _player->m_hasBindDialogOpen = true;
        return;
    }

    _player->m_hasBindDialogOpen = false;
    SmsgGossipComplete managedPacket;
    sendManagedPacket(managedPacket);
    creature->castSpell(_player->getGuid(), 3286, true);
}

void WorldSession::handleTrainerListOpcode(WorldPacket& recvPacket)
{
    CmsgTrainerList srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    _player->onTalkReputation(creature->getServersideFactionEntry());
    sendTrainerList(creature);
}

void WorldSession::handleStabledPetList(WorldPacket& recvPacket)
{
    MsgListStabledPets srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
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

    // Since cata all stable slots are automatically unlocked
    if (_socket->getClientProtocol().expansion >= WoW::Expansion::_Cata)
    {
        MsgListStabledPets sendPacket(npcguid, PET_SLOT_MAX_STABLE_SLOT, stableList);
        sendManagedPacket(sendPacket);
    }
    else
    {
        MsgListStabledPets sendPacket(npcguid, _player->m_stableSlotCount, stableList);
        sendManagedPacket(sendPacket);
    }
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
        uiMessage = _player->getSession()->localizedWorldSrv(ServerString::SS_WHAT_CAN_I_TEACH_YOU);
    else
        uiMessage = trainer->UIMessage;

    SmsgTrainerList managedPacket(creature, _player, uiMessage);
    sendManagedPacket(managedPacket);
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

        const auto spellsRequired = sSpellMgr.getSpellsRequiredRangeForSpell(spellId);
        for (const auto& itr : spellsRequired)
        {
            if (!_player->hasSpell(itr.second))
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
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debug("Received: CMSG_NPC_TEXT_QUERY: {} (textId)", srlPacket.text_id);

    _player->setTargetGuid(srlPacket.guid);

    WorldPacket data;
    data.initialize(SMSG_NPC_TEXT_UPDATE);
    data << srlPacket.text_id;

    const auto localesNpcText = (language > 0) ? sMySQLStore.getLocalizedNpcGossipText(srlPacket.text_id, language) : nullptr;
#if VERSION_STRING <= Cata

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
            data << _player->getSession()->localizedWorldSrv(ServerString::SS_HEY_HOW_CAN_I_HELP_YOU);
            data << _player->getSession()->localizedWorldSrv(ServerString::SS_HEY_HOW_CAN_I_HELP_YOU);
            data << uint32_t(0x00);           // Language

            for (uint8_t e = 0; e < GOSSIP_EMOTE_COUNT; e++)
            {
                data << uint32_t(0x00);       // Emote delay
                data << uint32_t(0x00);       // Emote
            }
        }
    }
#else // Mop
    ByteBuffer buffer;
    if (const auto pGossip = sMySQLStore.getNpcGossipText(srlPacket.text_id))
    {
        if (localesNpcText)
        {
            for (uint8_t i = 0; i < 8; ++i)
                buffer << float(pGossip->textHolder[i].probability); // probability

            for (uint8_t i = 0; i < 8; ++i)
                buffer << uint32_t(srlPacket.text_id); // broadcast text id

            for (uint8_t i = 0; i < 8; ++i)
            {
                if (strlen(localesNpcText->texts[i][0]) == 0)
                    buffer << localesNpcText->texts[i][1];
                else
                    buffer << localesNpcText->texts[i][0];
            }
        }
        else
        {
            for (uint8_t i = 0; i < 8; ++i)
                buffer << float(pGossip->textHolder[i].probability); // probability

            for (uint8_t i = 0; i < 8; ++i)
                buffer << uint32_t(srlPacket.text_id); // broadcast text id

            for (uint8_t i = 0; i < 8; ++i)
            {
                if (pGossip->textHolder[i].texts[0].empty())
                    buffer << pGossip->textHolder[i].texts[1];
                else
                    buffer << pGossip->textHolder[i].texts[0];
            }
        }
    }
    else
    {
        buffer << uint32_t(1); // unk 

        for (uint8_t i = 0; i < 7; ++i)
            buffer << uint32_t(0); // probability

        buffer << uint32_t(1);  //unk

        for (uint8_t i = 0; i < 7; ++i)
            buffer << uint32_t(0); // broadcast text id

        buffer << std::string(_player->getSession()->localizedWorldSrv(ServerString::SS_HEY_HOW_CAN_I_HELP_YOU));
    }

    data << uint32_t(buffer.size());
    data.append(buffer);
    data.writeBit(1); // write cache?
    data.flushBits();
#endif

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
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_BUY_BANK_SLOT: {} (guidLow).", srlPacket.guid.getGuidLow());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr || !creature->isBanker())
    {
        SmsgBuyBankSlotResult managedPacket(BankslotError::NotABanker);
        sendManagedPacket(managedPacket);
        return;
    }

    const uint8_t slots = _player->getBankSlots() + 1U;
    const auto bank_bag_slot_prices = sBankBagSlotPricesStore.lookupEntry(slots);
    if (bank_bag_slot_prices == nullptr)
    {
        SmsgBuyBankSlotResult managedPacket(BankslotError::TooMany);
        sendManagedPacket(managedPacket);
        return;
    }

    const auto price = bank_bag_slot_prices->Price;
    if (!_player->hasEnoughCoinage(price))
    {
        SmsgBuyBankSlotResult managedPacket(BankslotError::InsufficientFunds);
        sendManagedPacket(managedPacket);
        return;
    }

    _player->setBankSlots(slots);
    _player->modCoinage(-static_cast<int32_t>(price));
#if VERSION_STRING > TBC
    _player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT, 1, 0, 0);
#endif

}
