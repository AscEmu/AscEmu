/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
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

void WorldSession::SendTrainerList(Creature* pCreature)
{
    Trainer* trainer = pCreature->GetTrainer();
    if (trainer == nullptr)
        return;

    if (!_player->CanTrainAt(trainer))
        Arcemu::Gossip::Menu::SendSimpleMenu(pCreature->GetGUID(), trainer->Cannot_Train_GossipTextId, GetPlayer());
    else
    {
        WorldPacket data(SMSG_TRAINER_LIST, 5000);
        std::string text;

        data << pCreature->GetGUID();
        data << trainer->TrainerType;

        data << uint32_t(1);                    // different value for each trainer, also found in CMSG_TRAINER_BUY_SPELL

        size_t count_pos = data.wpos();
        data << uint32_t(trainer->Spells.size());

        bool can_learn_primary_prof = GetPlayer()->getFreePrimaryProfessionPoints() < 2;

        uint32_t count = 0;
        for (std::vector<TrainerSpell>::iterator itr = trainer->Spells.begin(); itr != trainer->Spells.end(); ++itr)
        {
            TrainerSpell * pSpell = &(*itr);

            bool valid = true;
            bool primary_prof_first_rank = false;
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (!pSpell->learnedSpell[i])
                    continue;

                if (!_player->isSpellFitByClassAndRace(pSpell->learnedSpell[i]))
                {
                    valid = false;
                    break;
                }

                SpellInfo* learnedSpellInfo = sSpellCustomizations.GetSpellInfo(pSpell->learnedSpell[i]);
                if (learnedSpellInfo && learnedSpellInfo->IsPrimaryProfession())
                    primary_prof_first_rank = true;
            }
            if (!valid)
                continue;

            TrainerSpellState state = TrainerGetSpellStatus(pSpell);

            data << uint32_t(pSpell->spell);                      // learned spell (or cast-spell in profession case)
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


                data << uint32_t(0);        // prevSpellId
                ++maxReq;

                if (maxReq == 2)
                    break;

                SpellsRequiringSpellMapBounds spellsRequired = objmgr.GetSpellsRequiredForSpellBounds(pSpell->learnedSpell[i]);
                for (SpellsRequiringSpellMap::const_iterator itr2 = spellsRequired.first; itr2 != spellsRequired.second && maxReq < 3; ++itr2)
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

            SpellInfo* spell = sSpellCustomizations.GetSpellInfo(pSpell->spell);
            if (spell && spell->IsPrimaryProfession())
                data << uint32_t(primary_prof_first_rank && can_learn_primary_prof ? 1 : 0);
            else
                data << uint32_t(1);

            data << uint32_t(primary_prof_first_rank ? 1 : 0);    // must be equal prev. field to have learn button in enabled state

            ++count;
        }

        if (stricmp(trainer->UIMessage, "DMSG") == 0)
            data << _player->GetSession()->LocalizedWorldSrv(37);
        else
            data << trainer->UIMessage;

        data.put<uint32_t>(count_pos, count);

        SendPacket(&data);
    }
}

void WorldSession::HandleTrainerBuySpellOpcode(WorldPacket & recvPacket)
{
    uint64_t guid;
    uint32_t teachingSpellId;
    uint32_t trainerId;

    recvPacket >> guid;
    recvPacket >> trainerId;
    recvPacket >> teachingSpellId;

    Creature* creature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (creature == nullptr)
        return;

    Trainer* trainer = creature->GetTrainer();
    if (trainer == nullptr)
        return;

    TrainerSpell* pSpell = nullptr;
    for (std::vector<TrainerSpell>::iterator itr = trainer->Spells.begin(); itr != trainer->Spells.end(); ++itr)
    {
        if (itr->spell == teachingSpellId)
        {
            pSpell = &(*itr);
        }
    }

    if (pSpell == nullptr)
    {
        sCheatLog.writefromsession(this, "Player %s tried learning none-obtainable spell - Possibly using WPE", _player->GetName());
        this->Disconnect();
        return;
    }

    if (TrainerGetSpellStatus(pSpell) == TRAINER_SPELL_RED || TRAINER_SPELL_GRAY)
        return;

    _player->ModGold(-(int32_t)pSpell->spellCost);

    if (pSpell->IsCastable())
    {
        _player->CastSpell(_player, pSpell->spell, true);
    }
    else
    {
        _player->playSpellVisual(creature->GetGUID(), 179);
        _player->playSpellVisual(_player->GetGUID(), 362);

        _player->addSpell(pSpell->spell);
    }

    _player->_UpdateSkillFields();

    WorldPacket data(SMSG_TRAINER_BUY_SUCCEEDED, 12);
    data << uint64_t(guid);
    data << uint32_t(teachingSpellId);
    SendPacket(&data);
}

TrainerSpellState WorldSession::TrainerGetSpellStatus(TrainerSpell* pSpell)
{
    if (pSpell == nullptr)
        return TRAINER_SPELL_RED;

    bool hasSpell = true;
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (!pSpell->learnedSpell[i])
            continue;

        if (!_player->HasSpell(pSpell->learnedSpell[i]))
        {
            hasSpell = false;
            break;
        }
    }

    // known spell
    if (hasSpell)
        return TRAINER_SPELL_GRAY;

    // check skill requirement
    if (pSpell->reqSkill && _player->_GetSkillLineCurrent(pSpell->reqSkill, true) < pSpell->reqSkillValue)
        return TRAINER_SPELL_RED;

    // check level requirement
    if (_player->getLevel() < pSpell->reqLevel)
        return TRAINER_SPELL_RED;

    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (!pSpell->learnedSpell[i])
            continue;

        // check race/class requirement     
        if (!_player->isSpellFitByClassAndRace(pSpell->learnedSpell[i]))
            return TRAINER_SPELL_RED;

        SpellsRequiringSpellMapBounds spellsRequired = objmgr.GetSpellsRequiredForSpellBounds(pSpell->learnedSpell[i]);
        for (SpellsRequiringSpellMap::const_iterator itr = spellsRequired.first; itr != spellsRequired.second; ++itr)
        {
            // check additional spell requirement
            if (!_player->HasSpell(itr->second))
                return TRAINER_SPELL_RED;
        }
    }

    return TRAINER_SPELL_GREEN;
}

void WorldSession::SendInnkeeperBind(Creature* pCreature)
{
    uint32_t current_zone = _player->GetZoneId();
    if (_player->m_bind_zoneid == current_zone)
    {
        OutPacket(SMSG_GOSSIP_COMPLETE, 0, NULL);

        WorldPacket data(SMSG_PLAYERBINDERROR, 1);
        data << uint32_t(1);                          // already bound here!
        SendPacket(&data);
        return;
    }

    if (!_player->bHasBindDialogOpen)
    {
        OutPacket(SMSG_GOSSIP_COMPLETE, 0, NULL);

        WorldPacket data(SMSG_BINDER_CONFIRM, 12);
        data << uint64_t(pCreature->GetGUID());
        data << uint32_t(_player->GetZoneId());
        SendPacket(&data);

        _player->bHasBindDialogOpen = true;
        return;
    }

    _player->bHasBindDialogOpen = false;

    OutPacket(SMSG_GOSSIP_COMPLETE, 0, NULL);
    uint64_t player_guid = _player->GetGUID();
    pCreature->CastSpell(player_guid, 3286, true);
}

void WorldSession::HandleBattleMasterHelloOpcode(WorldPacket& recvData)
{
    uint64_t guid;
    recvData >> guid;

    Creature* creature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (creature == nullptr || creature->isBattleMaster() == false)
        return;

    SendBattlegroundList(creature, 0);
}

//\todo Rewrite for cata - after this all functions are copied from wotlk

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

void WorldSession::HandleTabardVendorActivateOpcode(WorldPacket& recvData)
{
    CHECK_INWORLD_ASSERT;

    uint64 guid;
    recvData >> guid;
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

void WorldSession::HandleBankerActivateOpcode(WorldPacket& recvData)
{
    CHECK_INWORLD_ASSERT;

    uint64 guid;
    recvData >> guid;

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

void WorldSession::HandleTrainerListOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;
    Creature* train = GetPlayer()->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!train) return;

    _player->Reputation_OnTalk(train->m_factionDBC);
    SendTrainerList(train);
}

void WorldSession::HandleCharterShowListOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;

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

void WorldSession::HandleAuctionHelloOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;
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

void WorldSession::HandleGossipHelloOpcode(WorldPacket& recvData)
{
    uint64 guid;

    recvData >> guid;
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

void WorldSession::HandleGossipSelectOptionOpcode(WorldPacket& recvData)
{
    uint32 option;
    uint32 gossipId;
    uint64 guid;

    recvData >> guid;
    recvData >> gossipId;
    recvData >> option;

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
        if (recvData.rpos() != recvData.wpos())
            recvData >> str;

        if (str.length() > 0)
            script->OnSelectOption(object, GetPlayer(), option, str.c_str(), gossipId);
        else
            script->OnSelectOption(object, GetPlayer(), option, nullptr, gossipId);
    }
}

void WorldSession::HandleSpiritHealerActivateOpcode(WorldPacket& /*recvData*/)
{
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

void WorldSession::HandleNpcTextQueryOpcode(WorldPacket& recvData)
{
    WorldPacket data;
    uint32 textID;
    uint64 targetGuid;

    recvData >> textID;
    LOG_DETAIL("WORLD: CMSG_NPC_TEXT_QUERY ID '%u'", textID);

    recvData >> targetGuid;
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

void WorldSession::HandleBinderActivateOpcode(WorldPacket& recvData)
{
    uint64 guid;
    recvData >> guid;

    Creature* creatureBinder = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!creatureBinder)
        return;

    SendInnkeeperBind(creatureBinder);
}

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
