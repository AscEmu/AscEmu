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
    CHECK_INWORLD_ASSERT;

    uint64_t Guid;
    uint32_t TeachingSpellID;
    uint32_t TrainerId;

    recvPacket >> Guid;
    recvPacket >> TrainerId;
    recvPacket >> TeachingSpellID;

    Creature* creature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(Guid));
    if (creature == nullptr)
        return;

    Trainer* trainer = creature->GetTrainer();
    if (trainer == nullptr)
        return;

    TrainerSpell* pSpell = nullptr;
    for (std::vector<TrainerSpell>::iterator itr = trainer->Spells.begin(); itr != trainer->Spells.end(); ++itr)
    {
        if (itr->spell == TeachingSpellID)
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
    data << uint64_t(Guid);
    data << uint32_t(TeachingSpellID);
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

void WorldSession::HandleBattleMasterHelloOpcode(WorldPacket& recv_data)
{
    uint64_t guid;
    recv_data >> guid;

    Creature* creature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (creature == nullptr || creature->isBattleMaster() == false)
        return;

    SendBattlegroundList(creature, 0);
}
