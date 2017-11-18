/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/AuctionHouse.h"
#include "Management/AuctionMgr.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Spell/SpellAuras.h"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "Units/Creatures/Pet.h"

trainertype trainer_types[TRAINER_TYPE_MAX] =
{
    { "Warrior"         , 0 },
    { "Paladin"         , 0 },
    { "Rogue"           , 0 },
    { "Warlock"         , 0 },
    { "Mage"            , 0 },
    { "Shaman"          , 0 },
    { "Priest"          , 0 },
    { "Hunter"          , 0 },
    { "Druid"           , 0 },
    { "Leatherwork"     , 2 },
    { "Skinning"        , 2 },
    { "Fishing"         , 2 },
    { "First Aid"       , 2 },
    { "Physician"       , 2 },
    { "Engineer"        , 2 },
    { "Weapon Master"   , 0 },
};


//////////////////////////////////////////////////////////////////////////////////////////
/// This function handles MSG_TABARDVENDOR_ACTIVATE:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleTabardVendorActivateOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_ASSERT;

    uint64 guid;
    recv_data >> guid;
    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature) return;

    SendTabardHelp(pCreature);
}

void WorldSession::SendTabardHelp(Creature* pCreature)
{
    WorldPacket data(8);
    data.Initialize(MSG_TABARDVENDOR_ACTIVATE);
    data << pCreature->GetGUID();
    SendPacket(&data);
}


//////////////////////////////////////////////////////////////////////////////////////////
/// This function handles CMSG_BANKER_ACTIVATE:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleBankerActivateOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_ASSERT;

    uint64 guid;
    recv_data >> guid;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature) return;

    SendBankerList(pCreature);
}

void WorldSession::SendBankerList(Creature* pCreature)
{

    WorldPacket data(8);
    data.Initialize(SMSG_SHOW_BANK);
    data << pCreature->GetGUID();
    SendPacket(&data);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// This function handles CMSG_TRAINER_LIST
//////////////////////////////////////////////////////////////////////////////////////////
//NOTE: we select prerequirements for spell that TEACHES you
//not by spell that you learn!

void WorldSession::HandleTrainerListOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_ASSERT;

    // Inits, grab creature, check.
    uint64 guid;
    recv_data >> guid;
    Creature* train = GetPlayer()->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!train) return;

    _player->Reputation_OnTalk(train->m_factionDBC);
    SendTrainerList(train);
}

#if VERSION_STRING != Cata
void WorldSession::SendTrainerList(Creature* pCreature)
{
    Trainer* pTrainer = pCreature->GetTrainer();
    //if (pTrainer == 0 || !CanTrainAt(_player, pTrainer)) return;
    if (pTrainer == NULL)
        return;

    if (!_player->CanTrainAt(pTrainer))
        Arcemu::Gossip::Menu::SendSimpleMenu(pCreature->GetGUID(), pTrainer->Cannot_Train_GossipTextId, GetPlayer());
    else
    {
        WorldPacket data(SMSG_TRAINER_LIST, 5000);
        TrainerSpell* pSpell;
        uint32 Spacer = 0;
        uint32 Count = 0;
        uint8 Status;
        std::string Text;

        data << pCreature->GetGUID();
        data << pTrainer->TrainerType;

        data << uint32(0);
        for (std::vector<TrainerSpell>::iterator itr = pTrainer->Spells.begin(); itr != pTrainer->Spells.end(); ++itr)
        {
            pSpell = &(*itr);
            Status = TrainerGetSpellStatus(pSpell);
            if (pSpell->pCastRealSpell != NULL)
                data << pSpell->pCastSpell->getId();
            else if (pSpell->pLearnSpell)
                data << pSpell->pLearnSpell->getId();
            else
                continue;

            data << Status;
            data << pSpell->Cost;
            data << Spacer;
            data << uint32(pSpell->IsProfession);
            data << uint8(pSpell->RequiredLevel);
            data << pSpell->RequiredSkillLine;
            data << pSpell->RequiredSkillLineValue;
            data << pSpell->RequiredSpell;
            data << Spacer;    //this is like a spell override or something, ex : (id=34568 or id=34547) or (id=36270 or id=34546) or (id=36271 or id=34548)
            data << Spacer;
            ++Count;
        }

        *(uint32*)&data.contents()[12] = Count;

        if (stricmp(pTrainer->UIMessage, "DMSG") == 0)
            data << _player->GetSession()->LocalizedWorldSrv(37);
        else
            data << pTrainer->UIMessage;
        SendPacket(&data);
    }
}

void WorldSession::HandleTrainerBuySpellOpcode(WorldPacket & recvPacket)
{
    CHECK_INWORLD_ASSERT;

    uint64 Guid;
    uint32 TeachingSpellID;

    recvPacket >> Guid >> TeachingSpellID;
    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(Guid));

    //////////////////////////////////////////////////////////////////////////////////////////
    // Checks
    if (pCreature == NULL) return;

    Trainer* pTrainer = pCreature->GetTrainer();
    if (pTrainer == 0)
        return;

    // Check if the trainer offers that spell
    TrainerSpell* pSpell = NULL;
    for (std::vector<TrainerSpell>::iterator itr = pTrainer->Spells.begin(); itr != pTrainer->Spells.end(); ++itr)
    {
        if ((itr->pCastSpell && itr->pCastSpell->getId() == TeachingSpellID) ||
            (itr->pLearnSpell && itr->pLearnSpell->getId() == TeachingSpellID))
        {
            pSpell = &(*itr);
        }
    }

    // If the trainer doesn't offer it, this is probably some packet mangling
    if (pSpell == NULL)
    {
        // Disconnecting the player
        sCheatLog.writefromsession(this, "Player %s tried learning none-obtainable spell - Possibly using WPE", _player->GetName());
        this->Disconnect();
        return;
    }

    // We can't learn it
    if (TrainerGetSpellStatus(pSpell) > 0)
        return;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Teaching
    _player->ModGold(-(int32)pSpell->Cost);

    if (pSpell->pCastSpell)
    {
        _player->CastSpell(_player, pSpell->pCastSpell->getId(), true);
    }
    else
    {
        //Showing the learning spellvisuals
        _player->playSpellVisual(pCreature->GetGUID(), 1459);
        _player->playSpellVisual(_player->GetGUID(), 362);

        // add the spell itself
        _player->addSpell(pSpell->pLearnSpell->getId());
    }

    if (pSpell->DeleteSpell)
    {
        // Remove old spell.
        if (pSpell->pLearnSpell)
            _player->removeSpell(pSpell->DeleteSpell, true, true, pSpell->pLearnSpell->getId());
        else if (pSpell->pCastSpell)
            _player->removeSpell(pSpell->DeleteSpell, true, true, pSpell->pCastRealSpell->getId());
        else
            _player->removeSpell(pSpell->DeleteSpell, true, false, 0);
    }

    _player->_UpdateSkillFields();

    WorldPacket data(SMSG_TRAINER_BUY_SUCCEEDED, 12);

    data << uint64(Guid) << uint32(TeachingSpellID);        // GUID of the trainer, ID of the spell we bought
    this->SendPacket(&data);
}

uint8 WorldSession::TrainerGetSpellStatus(TrainerSpell* pSpell)
{
    if (!pSpell->pCastSpell && !pSpell->pLearnSpell)
        return TRAINER_STATUS_NOT_LEARNABLE;

    if (pSpell->pCastRealSpell && (_player->HasSpell(pSpell->pCastRealSpell->getId()) || _player->HasDeletedSpell(pSpell->pCastRealSpell->getId())))
        return TRAINER_STATUS_ALREADY_HAVE;

    if (pSpell->pLearnSpell && (_player->HasSpell(pSpell->pLearnSpell->getId()) || _player->HasDeletedSpell(pSpell->pLearnSpell->getId())))
        return TRAINER_STATUS_ALREADY_HAVE;

    if (pSpell->DeleteSpell && _player->HasDeletedSpell(pSpell->DeleteSpell))
        return TRAINER_STATUS_ALREADY_HAVE;

    if ((pSpell->RequiredLevel && _player->getLevel() < pSpell->RequiredLevel)
        || (pSpell->RequiredSpell && !_player->HasSpell(pSpell->RequiredSpell))
        || (pSpell->Cost && !_player->HasGold(pSpell->Cost))
        || (pSpell->RequiredSkillLine && _player->_GetSkillLineCurrent(pSpell->RequiredSkillLine, true) < pSpell->RequiredSkillLineValue)
        || (pSpell->IsProfession && _player->GetPrimaryProfessionPoints() == 0)     //check level 1 professions if we can learn a new profession
       )
        return TRAINER_STATUS_NOT_LEARNABLE;
    return TRAINER_STATUS_LEARNABLE;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
/// This function handles CMSG_PETITION_SHOWLIST:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleCharterShowListOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 guid;
    recv_data >> guid;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature) return;

    SendCharterRequest(pCreature);
}

void WorldSession::SendCharterRequest(Creature* pCreature)
{
    if (!pCreature)
        return;

    if (!pCreature->isTabardDesigner())
    {
        WorldPacket data(SMSG_PETITION_SHOWLIST, 81);

        data << pCreature->GetGUID();
        data << uint8(0x03);        //number of charter types in packet

        //////////////////////////////////////////////////////////////////////////////////////////
        //2v2 arena charters
        data << uint32(0x01);       //petition number (in packet)
        data << uint32(ARENA_TEAM_CHARTER_2v2); //itemid
        data << uint32(0x3F21);     //item displayid
        data << uint32(ARENA_TEAM_CHARTER_2v2_COST); //charter cost
        data << uint32(0x01);       //unknown, (charter type? seems to be 0x0 for guilds and 0x1 for arena charters)
        data << uint32(0x01);       // Signatures required (besides petition owner)

        //////////////////////////////////////////////////////////////////////////////////////////
        //3v3 arena charters
        data << uint32(0x02);       //petition number (in packet)
        data << uint32(ARENA_TEAM_CHARTER_3v3); //itemid
        data << uint32(0x3F21);     //item displayid
        data << uint32(ARENA_TEAM_CHARTER_3v3_COST); //charter cost
        data << uint32(0x01);
        data << uint32(0x02);       // Signatures required (besides petition owner)

        //////////////////////////////////////////////////////////////////////////////////////////
        //5v5 arena charters
        data << uint32(0x03);       //petition number (in packet)
        data << uint32(ARENA_TEAM_CHARTER_5v5); //itemid
        data << uint32(0x3F21);     //item displayid
        data << uint32(ARENA_TEAM_CHARTER_5v5_COST); //charter cost
        data << uint32(0x01);
        data << uint32(0x04);       // Signatures required (besides petition owner)

        SendPacket(&data);
    }
    else
    {
        WorldPacket data(33);
        data.Initialize(SMSG_PETITION_SHOWLIST);
        data << pCreature->GetGUID();
        data << uint8(1);               // num charters in packet (although appears to only turn off the cost display, maybe due to packet not being parsed /shrug)
        data << uint32(1);              // charter 1 in packet
        data << uint32(0x16E7);         // ItemId of the guild charter
        data << uint32(0x3F21);         // item displayid

        data << uint32(1000);           // charter price
        data << uint32(0);              // unknown, maybe charter type
        data << uint32(9);              // amount of unique players needed to sign the charter
        SendPacket(&data);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// This function handles MSG_AUCTION_HELLO:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleAuctionHelloOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 guid;
    recv_data >> guid;
    Creature* auctioneer = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!auctioneer)
        return;

    SendAuctionList(auctioneer);
}

void WorldSession::SendAuctionList(Creature* auctioneer)
{
    AuctionHouse* AH = sAuctionMgr.GetAuctionHouse(auctioneer->GetEntry());
    if (!AH)
    {
        sChatHandler.BlueSystemMessage(this, "Report to devs: Unbound auction house npc %u.", auctioneer->GetEntry());
        return;
    }

    WorldPacket data(MSG_AUCTION_HELLO, 12);
    data << uint64(auctioneer->GetGUID());
    data << uint32(AH->GetID());
    data << uint8(AH->enabled ? 1 : 0);         // Alleycat - Need to correct this properly.
    SendPacket(&data);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// This function handles CMSG_GOSSIP_HELLO:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGossipHelloOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 guid;

    recv_data >> guid;
    Creature* qst_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));

    if (qst_giver != nullptr)
    {
        //stop when talked to
        if (qst_giver->GetAIInterface())
            qst_giver->GetAIInterface()->StopMovement(30000);

        // unstealth meh
        if (_player->IsStealth())
            _player->RemoveAllAuraType(SPELL_AURA_MOD_STEALTH);

        // reputation
        _player->Reputation_OnTalk(qst_giver->m_factionDBC);

        LOG_DEBUG("WORLD: Received CMSG_GOSSIP_HELLO from %u", Arcemu::Util::GUID_LOPART(guid));

        Arcemu::Gossip::Script* script = Arcemu::Gossip::Script::GetInterface(qst_giver);
        if (script != nullptr)
            script->OnHello(qst_giver, GetPlayer());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// This function handles CMSG_GOSSIP_SELECT_OPTION:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGossipSelectOptionOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint32 option;
    uint32 gossipId;
    uint64 guid;

    recv_data >> guid;
    recv_data >> gossipId;
    recv_data >> option;

    LOG_DETAIL("WORLD: CMSG_GOSSIP_SELECT_OPTION GossipId: %u Item: %i senderGuid %.8X", gossipId, option, guid);

    Arcemu::Gossip::Script* script = nullptr;
    uint32 guidtype = GET_TYPE_FROM_GUID(guid);

    Object* object;
    if (guidtype == HIGHGUID_TYPE_ITEM)         //Item objects are retrieved differently.
    {
        object = GetPlayer()->GetItemInterface()->GetItemByGUID(guid);
        if (object != nullptr)
            script = Arcemu::Gossip::Script::GetInterface(static_cast<Item*>(object));
    }
    else
    {
        object = GetPlayer()->GetMapMgr()->_GetObject(guid);
    }

    if (object != nullptr)
    {
        if (guidtype == HIGHGUID_TYPE_UNIT)
            script = Arcemu::Gossip::Script::GetInterface(static_cast<Creature*>(object));
        else if (guidtype == HIGHGUID_TYPE_GAMEOBJECT)
            script = Arcemu::Gossip::Script::GetInterface(static_cast<GameObject*>(object));
    }

    if (script != nullptr)
    {
        std::string str;
        if (recv_data.rpos() != recv_data.wpos())
            recv_data >> str;

        if (str.length() > 0)
            script->OnSelectOption(object, GetPlayer(), option, str.c_str(), gossipId);
        else
            script->OnSelectOption(object, GetPlayer(), option, nullptr, gossipId);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// This function handles CMSG_SPIRIT_HEALER_ACTIVATE:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleSpiritHealerActivateOpcode(WorldPacket& /*recvData*/)
{
    CHECK_INWORLD_RETURN

        if (!_player->IsDead())
            return;

    GetPlayer()->DeathDurabilityLoss(0.25);
    GetPlayer()->ResurrectPlayer();

    if (_player->getLevel() > 10)
    {
        Aura* aur = GetPlayer()->getAuraWithId(15007);

        if (aur == nullptr)        // If the player already have the aura, just extend it.
        {
            SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(15007);    //resurrection sickness
            SpellCastTargets targets;
            targets.m_unitTarget = GetPlayer()->GetGUID();
            Spell* sp = sSpellFactoryMgr.NewSpell(_player, spellInfo, true, nullptr);
            sp->prepare(&targets);
        }

        //calc new duration
        int32 duration = 600000;        //10mins

        if (_player->getLevel() < 20)
            duration = (_player->getLevel() - 10) * 60000;

        _player->SetAurDuration(15007, duration);                   //cebernic: change this to setaurduration() to be refreshed.
    }

    GetPlayer()->SetHealth(GetPlayer()->GetMaxHealth() / 2);
}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_NPC_TEXT_QUERY:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleNpcTextQueryOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    WorldPacket data;
    uint32 textID;
    uint64 targetGuid;

    recv_data >> textID;
    LOG_DETAIL("WORLD: CMSG_NPC_TEXT_QUERY ID '%u'", textID);

    recv_data >> targetGuid;
    GetPlayer()->SetTargetGUID(targetGuid);

    MySQLStructure::NpcText const* pGossip = sMySQLStore.getNpcText(textID);
    MySQLStructure::LocalesNpcText const* lnc = (language > 0) ? sMySQLStore.getLocalizedNpcText(textID, language) : nullptr;

    data.Initialize(SMSG_NPC_TEXT_UPDATE);
    data << textID;

    if (pGossip)
    {
        for (uint8 i = 0; i < 8; i++)
        {
            data << float(pGossip->textHolder[i].probability);

            if (lnc)
            {
                if (strlen(lnc->texts[i][0]) == 0)
                {
                    data << lnc->texts[i][1];
                }
                else
                {
                    data << lnc->texts[i][0];
                }

                if (strlen(lnc->texts[i][1]) == 0)
                {
                    data << lnc->texts[i][0];
                }
                else
                {
                    data << lnc->texts[i][1];
                }
            }
            else
            {
                if (pGossip->textHolder[i].texts[0].size() == 0)
                {
                    data << pGossip->textHolder[i].texts[1];
                }
                else
                {
                    data << pGossip->textHolder[i].texts[0];
                }

                if (pGossip->textHolder[i].texts[1].size() == 0)
                {
                    data << pGossip->textHolder[i].texts[0];
                }
                else
                {
                    data << pGossip->textHolder[i].texts[1];
                }
            }
            data << pGossip->textHolder[i].language;

            for (uint8 e = 0; e < GOSSIP_EMOTE_COUNT; e++)
            {
                data << uint32(pGossip->textHolder[i].gossipEmotes[e].delay);
                data << uint32(pGossip->textHolder[i].gossipEmotes[e].emote);
            }
        }
    }
    else
    {
        for (uint8 i = 0; i < 8; i++)
        {
            data << float(1.0f);            // Prob
            data << _player->GetSession()->LocalizedWorldSrv(70);
            data << _player->GetSession()->LocalizedWorldSrv(70);
            data << uint32(0x00);           // Language

            for (uint8 e = 0; e < GOSSIP_EMOTE_COUNT; e++)
            {
                data << uint32(0x00);       // Emote delay
                data << uint32(0x00);       // Emote
            }
        }
    }

    SendPacket(&data);
    return;
}

void WorldSession::HandleBinderActivateOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN;
    uint64 guid;
    recv_data >> guid;

    Creature* creatureBinder = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!creatureBinder)
        return;

    SendInnkeeperBind(creatureBinder);
}

#define BIND_SPELL_ID 3286

#if VERSION_STRING != Cata
void WorldSession::SendInnkeeperBind(Creature* pCreature)
{
    WorldPacket data(45);

    if (!_player->bHasBindDialogOpen)
    {
        OutPacket(SMSG_GOSSIP_COMPLETE, 0, NULL);

        data.Initialize(SMSG_BINDER_CONFIRM);
        data << pCreature->GetGUID() << _player->GetZoneId();
        SendPacket(&data);

        _player->bHasBindDialogOpen = true;
        return;
    }

    _player->bHasBindDialogOpen = false;
    OutPacket(SMSG_GOSSIP_COMPLETE, 0, NULL);

    pCreature->CastSpell(_player->GetGUID(), BIND_SPELL_ID, true);
}
#endif

#undef BIND_SPELL_ID

void WorldSession::SendSpiritHealerRequest(Creature* pCreature)
{
    WorldPacket data(SMSG_SPIRIT_HEALER_CONFIRM, 8);
    data << pCreature->GetGUID();
    SendPacket(&data);
}

void WorldSession::SendStabledPetList(uint64 npcguid)
{
    WorldPacket data(10 + (_player->m_Pets.size() * 25));
    data.SetOpcode(MSG_LIST_STABLED_PETS);

    data << npcguid;

    data << uint8(_player->m_Pets.size());
    data << uint8(_player->m_StableSlotCount);
    for (std::map<uint32, PlayerPet*>::iterator itr = _player->m_Pets.begin(); itr != _player->m_Pets.end(); ++itr)
    {
        data << uint32(itr->first);             // pet no
        data << uint32(itr->second->entry);     // entryid
        data << uint32(itr->second->level);     // level
        data << itr->second->name;              // name
        if (itr->second->stablestate == STABLE_STATE_ACTIVE)
            data << uint8(STABLE_STATE_ACTIVE);
        else
        {
            data << uint8(STABLE_STATE_PASSIVE + 1);
        }
    }

    SendPacket(&data);
}
