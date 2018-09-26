/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
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
#include "Units/Players/PlayerClasses.hpp"
#include "Units/Creatures/Creature.h"
#include "Map/MapMgr.h"
#include "Management/AuctionMgr.h"
#include "Server/MainServerDefines.h"
#include "Objects/ObjectMgr.h"
#include "Server/Packets/CmsgGossipSelectOption.h"
#include "Server/Packets/CmsgGossipHello.h"
#include "Management/ItemInterface.h"
#include "Server/Packets/SmsgBinderConfirm.h"
#include "Server/Packets/CmsgTrainerList.h"
#include "Server/Packets/CmsgBinderActivate.h"
#include "Units/Creatures/Pet.h"
#include "Server/Packets/MsgListStabledPets.h"
#include "Server/Packets/CmsgNpcTextQuery.h"
#include "Storage/MySQLDataStore.hpp"
#include "Spell/SpellMgr.h"
#include "Server/Packets/CmsgBuyBankSlot.h"
#include "Server/Packets/SmsgBuyBankSlotResult.h"

using namespace AscEmu::Packets;

void WorldSession::handleTabardVendorActivateOpcode(WorldPacket& recvPacket)
{
    MsgTabardvendorActivate recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received MSG_TABARDVENDOR_ACTIVATE: %u (guidLowPart)", recv_packet.guid.getGuidLowPart());

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    SendPacket(MsgTabardvendorActivate(recv_packet.guid).serialise().get());
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
    CmsgBankerActivate recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_BANKER_ACTIVATE: %u (guidLowPart)", recv_packet.guid.getGuidLowPart());

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    SendPacket(SmsgShowBank(recv_packet.guid).serialise().get());
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
    MsgAuctionHello recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received MSG_AUCTION_HELLO: %u (guidLowPart)", recv_packet.guid.getGuidLowPart());

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLowPart());
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

    SendPacket(MsgAuctionHello(creature->getGuid(), auctionHouse->GetID(), auctionHouse->enabled ? 1 : 0).serialise().get());
}

//helper
void WorldSession::sendSpiritHealerRequest(Creature* creature)
{
    SendPacket(SmsgSpiritHealerConfirm(creature->getGuid()).serialise().get());
}

void WorldSession::handleTrainerBuySpellOpcode(WorldPacket& recvPacket)
{
    CmsgTrainerBuySpell recv_packet;
    if (!recv_packet.deserialise((recvPacket)))
        return;

    LOG_DEBUG("Received CMSG_TRAINER_BUY_SPELL: %u (guidLowPart)", recv_packet.guid.getGuidLowPart());

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    const auto trainer = creature->GetTrainer();
    if (trainer == nullptr)
        return;

    TrainerSpell* trainerSpell = nullptr;
#if VERSION_STRING == Cata
    for (auto itr : trainer->Spells)
    {
        if (itr.spell == recv_packet.spellId)
        {
            trainerSpell = &itr;
            break;
        }
    }
#else
    for (auto itr : trainer->Spells)
    {
        if ((itr.pCastSpell && itr.pCastSpell->getId() == recv_packet.spellId) ||
            (itr.pLearnSpell && itr.pLearnSpell->getId() == recv_packet.spellId))
        {
            trainerSpell = &itr;
            break;
        }
    }
#endif

    if (trainerSpell == nullptr)
    {
        sCheatLog.writefromsession(this, "%s tried to learn none-obtainable spell - Possibly using WPE", GetPlayer()->getName().c_str());
        Disconnect();
        return;
    }

#if VERSION_STRING == Cata
    if (trainerGetSpellStatus(trainerSpell) == TRAINER_SPELL_RED || TRAINER_SPELL_GRAY)
        return;

    GetPlayer()->ModGold(-static_cast<int32_t>(trainerSpell->spellCost));

    if (trainerSpell->IsCastable())
    {
        GetPlayer()->CastSpell(GetPlayer(), trainerSpell->spell, true);
    }
    else
    {
        GetPlayer()->playSpellVisual(creature->getGuid(), 179);
        GetPlayer()->playSpellVisual(GetPlayer()->getGuid(), 362);

        GetPlayer()->addSpell(trainerSpell->spell);
    }
#else
    if (trainerGetSpellStatus(trainerSpell) != TRAINER_STATUS_LEARNABLE)
        return;

    // teach the spell
    GetPlayer()->ModGold(-static_cast<int32>(trainerSpell->Cost));
    if (trainerSpell->pCastSpell)
    {
        GetPlayer()->CastSpell(GetPlayer(), trainerSpell->pCastSpell->getId(), true);
    }
    else
    {
        GetPlayer()->playSpellVisual(creature->getGuid(), 1459);
        GetPlayer()->playSpellVisual(GetPlayer()->getGuid(), 362);

        GetPlayer()->addSpell(trainerSpell->pLearnSpell->getId());
    }

    if (trainerSpell->DeleteSpell)
    {
        if (trainerSpell->pLearnSpell)
            GetPlayer()->removeSpell(trainerSpell->DeleteSpell, true, true, trainerSpell->pLearnSpell->getId());
        else if (trainerSpell->pCastSpell)
            GetPlayer()->removeSpell(trainerSpell->DeleteSpell, true, true, trainerSpell->pCastRealSpell->getId());
        else
            GetPlayer()->removeSpell(trainerSpell->DeleteSpell, true, false, 0);
    }
#endif
    GetPlayer()->_UpdateSkillFields();

    SendPacket(SmsgTrainerBuySucceeded(recv_packet.guid.GetOldGuid(), recv_packet.spellId).serialise().get());
}

void WorldSession::handleCharterShowListOpcode(WorldPacket& recvPacket)
{
    CmsgPetitionShowlist recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_CHARTER_SHOW_LIST: %u (guidLowPart)", recv_packet.guid.getGuidLowPart());

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLowPart());
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
    CmsgGossipHello gossipPacket;
    if (!gossipPacket.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_GOSSIP_HELLO: %u (guidLowPart)", gossipPacket.guid.getGuidLowPart());

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(gossipPacket.guid.getGuidLowPart());
    if (creature != nullptr)
    {
        if (creature->GetAIInterface())
            creature->GetAIInterface()->StopMovement(30000);

        if (GetPlayer()->IsStealth())
            GetPlayer()->RemoveAllAuraType(SPELL_AURA_MOD_STEALTH);

        GetPlayer()->Reputation_OnTalk(creature->m_factionEntry);

        const auto script = Arcemu::Gossip::Script::GetInterface(creature);
        if (script != nullptr)
            script->OnHello(creature, GetPlayer());
    }
}


void WorldSession::handleGossipSelectOptionOpcode(WorldPacket& recvPacket)
{
    CmsgGossipSelectOption gossipSelectPacket;
    if (!gossipSelectPacket.deserialise(recvPacket))
        return;

    LOG_DETAIL("Received CMSG_GOSSIP_SELECT_OPTION: %u (gossipId), %i (option), %u (guidLow)",
        gossipSelectPacket.gossip_id, gossipSelectPacket.option, gossipSelectPacket.guid.getGuidLow());

    Arcemu::Gossip::Script* script = nullptr;

    Object* object;
    if (gossipSelectPacket.guid.isItem())
    {
        object = GetPlayer()->GetItemInterface()->GetItemByGUID(gossipSelectPacket.guid);
        if (object != nullptr)
            script = Arcemu::Gossip::Script::GetInterface(static_cast<Item*>(object));
    }
    else
    {
        object = GetPlayer()->GetMapMgr()->_GetObject(gossipSelectPacket.guid);
    }

    if (object != nullptr)
    {
        if (gossipSelectPacket.guid.isUnit())
            script = Arcemu::Gossip::Script::GetInterface(static_cast<Creature*>(object));
        else if (gossipSelectPacket.guid.isGameObject())
            script = Arcemu::Gossip::Script::GetInterface(static_cast<GameObject*>(object));
    }

    if (script != nullptr)
    {
        if (gossipSelectPacket.input.length() > 0)
            script->OnSelectOption(object, GetPlayer(), gossipSelectPacket.option, gossipSelectPacket.input.c_str(), gossipSelectPacket.gossip_id);
        else
            script->OnSelectOption(object, GetPlayer(), gossipSelectPacket.option, nullptr, gossipSelectPacket.gossip_id);
    }
}

void WorldSession::handleBinderActivateOpcode(WorldPacket& recvPacket)
{
    CmsgBinderActivate recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_BINDER_ACTIVATE: %u (guidLowPart)", recv_packet.guid.getGuidLowPart());

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    sendInnkeeperBind(creature);
}

void WorldSession::sendInnkeeperBind(Creature* creature)
{
    const uint32_t current_zone = GetPlayer()->GetZoneId();
    if (GetPlayer()->m_bind_zoneid == current_zone)
    {
        OutPacket(SMSG_GOSSIP_COMPLETE, 0, nullptr);

        WorldPacket data(SMSG_PLAYERBINDERROR, 1);
        data << uint32_t(1);
        SendPacket(&data);
        return;
    }

    if (!GetPlayer()->bHasBindDialogOpen)
    {
        OutPacket(SMSG_GOSSIP_COMPLETE, 0, nullptr);

        SendPacket(SmsgBinderConfirm(creature->getGuid(), GetPlayer()->GetZoneId()).serialise().get());

        GetPlayer()->bHasBindDialogOpen = true;
        return;
    }

    GetPlayer()->bHasBindDialogOpen = false;
    OutPacket(SMSG_GOSSIP_COMPLETE, 0, nullptr);
    creature->CastSpell(GetPlayer()->getGuid(), 3286, true);
}

void WorldSession::handleTrainerListOpcode(WorldPacket& recvPacket)
{
    CmsgTrainerList recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLowPart());
    if (creature == nullptr)
        return;

    GetPlayer()->Reputation_OnTalk(creature->m_factionEntry);
    sendTrainerList(creature);
}

void WorldSession::handleStabledPetList(WorldPacket& recvPacket)
{
    MsgListStabledPets recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (GetPlayer()->getClass() != HUNTER)
    {
        Arcemu::Gossip::Menu::SendSimpleMenu(recv_packet.guid, 13584, GetPlayer());
        return;
    }

    sendStabledPetList(recv_packet.guid);
}

void WorldSession::sendStabledPetList(uint64 npcguid)
{
    std::vector<PlayerStablePetList> stableList;
    PlayerStablePetList stablePet;

    for (const auto itr : GetPlayer()->m_Pets)
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

    SendPacket(MsgListStabledPets(npcguid, static_cast<uint8_t>(GetPlayer()->m_Pets.size()), GetPlayer()->m_StableSlotCount, stableList).serialise().get());
}

#if VERSION_STRING != Cata
void WorldSession::sendTrainerList(Creature* creature)
{
    const auto trainer = creature->GetTrainer();
    if (trainer == nullptr)
        return;

    if (!GetPlayer()->CanTrainAt(trainer))
    {
        Arcemu::Gossip::Menu::SendSimpleMenu(creature->getGuid(), trainer->Cannot_Train_GossipTextId, GetPlayer());
    }
    else
    {
        WorldPacket data(SMSG_TRAINER_LIST, 5000);
        TrainerSpell* pSpell;
        uint32_t Spacer = 0;
        uint8_t Status;
        std::string Text;

        data << creature->getGuid();
        data << trainer->TrainerType;

        size_t count_p = data.wpos();
        data << uint32_t(trainer->Spells.size());

        uint32_t count = 0;
        for (auto itr : trainer->Spells)
        {
            pSpell = &itr;
            Status = trainerGetSpellStatus(pSpell);
            if (pSpell->pCastRealSpell)
                data << pSpell->pCastSpell->getId();
            else if (pSpell->pLearnSpell)
                data << pSpell->pLearnSpell->getId();
            else
                continue;

            data << Status;
            data << pSpell->Cost;
            data << Spacer;
            data << uint32_t(pSpell->IsProfession);
            data << uint8_t(pSpell->RequiredLevel);
            data << pSpell->RequiredSkillLine;
            data << pSpell->RequiredSkillLineValue;
            data << pSpell->RequiredSpell;
            data << Spacer;    //this is like a spell override or something, ex : (id=34568 or id=34547) or (id=36270 or id=34546) or (id=36271 or id=34548)
            data << Spacer;
            ++count;
        }

        data.put<uint32_t>(count_p, count);

        if (stricmp(trainer->UIMessage, "DMSG") == 0)
            data << GetPlayer()->GetSession()->LocalizedWorldSrv(37);
        else
            data << trainer->UIMessage;
        SendPacket(&data);
    }
}

uint8_t WorldSession::trainerGetSpellStatus(TrainerSpell* trainerSpell)
{
    if (!trainerSpell->pCastSpell && !trainerSpell->pLearnSpell)
        return TRAINER_STATUS_NOT_LEARNABLE;

    if (trainerSpell->pCastRealSpell && (GetPlayer()->HasSpell(trainerSpell->pCastRealSpell->getId()) || GetPlayer()->HasDeletedSpell(trainerSpell->pCastRealSpell->getId())))
        return TRAINER_STATUS_ALREADY_HAVE;

    if (trainerSpell->pLearnSpell && (GetPlayer()->HasSpell(trainerSpell->pLearnSpell->getId()) || GetPlayer()->HasDeletedSpell(trainerSpell->pLearnSpell->getId())))
        return TRAINER_STATUS_ALREADY_HAVE;

    if (trainerSpell->DeleteSpell && GetPlayer()->HasDeletedSpell(trainerSpell->DeleteSpell))
        return TRAINER_STATUS_ALREADY_HAVE;

    if ((trainerSpell->RequiredLevel && GetPlayer()->getLevel() < trainerSpell->RequiredLevel)
        || (trainerSpell->RequiredSpell && !GetPlayer()->HasSpell(trainerSpell->RequiredSpell))
        || (trainerSpell->Cost && !GetPlayer()->HasGold(trainerSpell->Cost))
        || (trainerSpell->RequiredSkillLine && GetPlayer()->_GetSkillLineCurrent(trainerSpell->RequiredSkillLine, true) < trainerSpell->RequiredSkillLineValue)
        || (trainerSpell->IsProfession && GetPlayer()->getFreePrimaryProfessionPoints() == 0)
        )
        return TRAINER_STATUS_NOT_LEARNABLE;
    return TRAINER_STATUS_LEARNABLE;
}
#else
void WorldSession::sendTrainerList(Creature* creature)
{
    Trainer* trainer = creature->GetTrainer();
    if (trainer == nullptr)
        return;

    if (!GetPlayer()->CanTrainAt(trainer))
    {
        Arcemu::Gossip::Menu::SendSimpleMenu(creature->getGuid(), trainer->Cannot_Train_GossipTextId, GetPlayer());
    }
    else
    {
        WorldPacket data(SMSG_TRAINER_LIST, 5000);
        std::string text;

        data << creature->getGuid();
        data << trainer->TrainerType;

        data << uint32_t(1);                    // different value for each trainer, also found in CMSG_TRAINER_BUY_SPELL

        size_t count_pos = data.wpos();
        data << uint32_t(trainer->Spells.size());

        bool can_learn_primary_prof = GetPlayer()->getFreePrimaryProfessionPoints() < 2;

        uint32_t count = 0;
        for (auto itr : trainer->Spells)
        {
            TrainerSpell* pSpell = &itr;

            bool valid = true;
            bool primary_prof_first_rank = false;
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (!pSpell->learnedSpell[i])
                    continue;

                if (!GetPlayer()->isSpellFitByClassAndRace(pSpell->learnedSpell[i]))
                {
                    valid = false;
                    break;
                }

                SpellInfo* learnedSpellInfo = sSpellCustomizations.GetSpellInfo(pSpell->learnedSpell[i]);
                if (learnedSpellInfo && learnedSpellInfo->isPrimaryProfession())
                    primary_prof_first_rank = true;
            }
            if (!valid)
                continue;

            TrainerSpellState state = trainerGetSpellStatus(pSpell);

            data << uint32_t(pSpell->spell);
            data << uint8_t(state);
            data << uint32_t(floor(pSpell->spellCost));

            data << uint8_t(pSpell->reqLevel);
            data << uint32_t(pSpell->reqSkill);
            data << uint32_t(pSpell->reqSkillValue);

            uint8_t maxReq = 0;
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (!pSpell->learnedSpell[i])
                    continue;


                data << uint32_t(0);
                ++maxReq;

                if (maxReq == 2)
                    break;

                SpellsRequiringSpellMapBounds spellsRequired = objmgr.GetSpellsRequiredForSpellBounds(pSpell->learnedSpell[i]);
                for (auto itr2 = spellsRequired.first; itr2 != spellsRequired.second && maxReq < 3; ++itr2)
                {
                    data << uint32_t(itr2->second);
                    ++maxReq;
                }

                if (maxReq == 2)
                    break;
            }
            while (maxReq < 2)
            {
                data << uint32_t(0);
                ++maxReq;
            }

            const auto spellInfo = sSpellCustomizations.GetSpellInfo(pSpell->spell);
            if (spellInfo && spellInfo->isPrimaryProfession())
                data << uint32_t(primary_prof_first_rank && can_learn_primary_prof ? 1 : 0);
            else
                data << uint32_t(1);

            data << uint32_t(primary_prof_first_rank ? 1 : 0);    // must be equal prev. field to have learn button in enabled state

            ++count;
        }

        if (stricmp(trainer->UIMessage, "DMSG") == 0)
            data << GetPlayer()->GetSession()->LocalizedWorldSrv(37);
        else
            data << trainer->UIMessage;

        data.put<uint32_t>(count_pos, count);

        SendPacket(&data);
    }
}

TrainerSpellState WorldSession::trainerGetSpellStatus(TrainerSpell* trainerSpell)
{
    if (trainerSpell == nullptr)
        return TRAINER_SPELL_RED;

    bool hasSpell = true;
    for (uint32_t spellId : trainerSpell->learnedSpell)
    {
        if (!spellId)
            continue;

        if (!GetPlayer()->HasSpell(spellId))
        {
            hasSpell = false;
            break;
        }
    }

    if (hasSpell)
        return TRAINER_SPELL_GRAY;

    if (trainerSpell->reqSkill && GetPlayer()->_GetSkillLineCurrent(trainerSpell->reqSkill, true) < trainerSpell->reqSkillValue)
        return TRAINER_SPELL_RED;

    if (GetPlayer()->getLevel() < trainerSpell->reqLevel)
        return TRAINER_SPELL_RED;

    for (uint32_t spellId : trainerSpell->learnedSpell)
    {
        if (spellId == 0)
            continue;

        if (!GetPlayer()->isSpellFitByClassAndRace(spellId))
            return TRAINER_SPELL_RED;

        SpellsRequiringSpellMapBounds spellsRequired = objmgr.GetSpellsRequiredForSpellBounds(spellId);
        for (auto itr = spellsRequired.first; itr != spellsRequired.second; ++itr)
        {
            if (!GetPlayer()->HasSpell(itr->second))
                return TRAINER_SPELL_RED;
        }
    }

    return TRAINER_SPELL_GREEN;
}
#endif

void WorldSession::handleSpiritHealerActivateOpcode(WorldPacket& /*recvPacket*/)
{
    if (!GetPlayer()->IsDead())
        return;

    GetPlayer()->DeathDurabilityLoss(0.25);
    GetPlayer()->ResurrectPlayer();

    if (GetPlayer()->getLevel() > 10)
    {
        const auto aura = GetPlayer()->getAuraWithId(15007);
        if (aura == nullptr)
        {
            SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(15007);
            SpellCastTargets targets;
            targets.m_unitTarget = GetPlayer()->getGuid();
            const auto spell = sSpellFactoryMgr.NewSpell(GetPlayer(), spellInfo, true, nullptr);
            spell->prepare(&targets);
        }

        int32_t duration = 600000;

        if (GetPlayer()->getLevel() < 20)
            duration = (GetPlayer()->getLevel() - 10) * 60000;

        GetPlayer()->SetAurDuration(15007, duration);
    }

    GetPlayer()->setHealth(GetPlayer()->getMaxHealth() / 2);
}

void WorldSession::handleNpcTextQueryOpcode(WorldPacket& recvPacket)
{
    CmsgNpcTextQuery npcTextPacket;
    if (!npcTextPacket.deserialise(recvPacket))
        return;

    LOG_DETAIL("Received: CMSG_NPC_TEXT_QUERY: %u (textId)", npcTextPacket.text_id);

    GetPlayer()->setTargetGuid(npcTextPacket.guid);

    const auto localesNpcText = (language > 0) ? sMySQLStore.getLocalizedNpcText(npcTextPacket.text_id, language) : nullptr;

    WorldPacket data;
    data.Initialize(SMSG_NPC_TEXT_UPDATE);
    data << npcTextPacket.text_id;

    if (const auto pGossip = sMySQLStore.getNpcText(npcTextPacket.text_id))
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
            data << GetPlayer()->GetSession()->LocalizedWorldSrv(70);
            data << GetPlayer()->GetSession()->LocalizedWorldSrv(70);
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
    CmsgBuyBankSlot recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_BUY_BANK_SLOT: %u (guidLow)", recv_packet.guid.getGuidLow());

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLow());
    if (creature == nullptr || !creature->isBanker())
    {
        SendPacket(SmsgBuyBankSlotResult(BankslotError::NotABanker).serialise().get());
        return;
    }

    const uint8_t slots = GetPlayer()->getBankSlots();
    const auto bank_bag_slot_prices = sBankBagSlotPricesStore.LookupEntry(slots + 1);
    if (bank_bag_slot_prices == nullptr)
    {
        SendPacket(SmsgBuyBankSlotResult(BankslotError::TooMany).serialise().get());
        return;
    }

    const auto price = bank_bag_slot_prices->Price;
    if (!GetPlayer()->HasGold(price))
    {
        SendPacket(SmsgBuyBankSlotResult(BankslotError::InsufficientFunds).serialise().get());
        return;
    }

    GetPlayer()->setBankSlots(slots + 1);
    GetPlayer()->ModGold(-static_cast<int32_t>(price));
#if VERSION_STRING > TBC
    GetPlayer()->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT, 1, 0, 0);
#endif

}
