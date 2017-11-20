/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "FastQueue.h"
#include "Threading/Mutex.h"
#include "WorldPacket.h"
#include "Management/Item.h"
#include "Exceptions/PlayerExceptions.hpp"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Units/Creatures/Pet.h"

void WorldSession::HandleLearnTalentOpcode(WorldPacket& recv_data)
{
    uint32_t talent_id;
    uint32_t requested_rank;

    recv_data >> talent_id;
    recv_data >> requested_rank;

    _player->LearnTalent(talent_id, requested_rank);
}

void WorldSession::HandleLearnPreviewTalentsOpcode(WorldPacket& recv_data)
{
    int32_t current_tab;
    uint32_t talent_count;
    uint32_t talent_id;
    uint32_t talent_rank;

    //if currentTab -1 player has already the spec.
    recv_data >> current_tab;
    recv_data >> talent_count;

    for (uint32_t i = 0; i < talent_count; ++i)
    {
        recv_data >> talent_id;
        recv_data >> talent_rank;

        _player->LearnTalent(talent_id, talent_rank, true);
    }

    _player->smsg_TalentsInfo(false);
}

void WorldSession::HandleUnlearnTalents(WorldPacket& /*recvData*/)
{
    uint32_t price = GetPlayer()->CalcTalentResetCost(GetPlayer()->GetTalentResetTimes());
    if (!GetPlayer()->HasGold(price))
        return;

    GetPlayer()->SetTalentResetTimes(GetPlayer()->GetTalentResetTimes() + 1);
    GetPlayer()->ModGold(-(int32_t)price);
    GetPlayer()->Reset_Talents();
}

void WorldSession::HandleUnlearnSkillOpcode(WorldPacket& recv_data)
{
    uint32_t skill_line_id;
    uint32_t points_remaining = _player->GetPrimaryProfessionPoints();

    recv_data >> skill_line_id;

    _player->RemoveSpellsFromLine(skill_line_id);
    _player->_RemoveSkillLine(skill_line_id);

    if (points_remaining == _player->GetPrimaryProfessionPoints())
    {
        auto skill_line = sSkillLineStore.LookupEntry(skill_line_id);
        if (!skill_line)
            return;

        if (skill_line->type == SKILL_TYPE_PROFESSION && points_remaining < 2)
            _player->SetPrimaryProfessionPoints(points_remaining + 1);
    }
}

void WorldSession::HandlePetLearnTalent(WorldPacket& recvPacket)
{
    uint64_t guid;
    uint32_t talent_id;
    uint32_t talent_rank;

    recvPacket >> guid;
    recvPacket >> talent_id;
    recvPacket >> talent_rank;

    Pet* player_pet = _player->GetSummon();
    if (player_pet == nullptr)
        return;

    if (guid != player_pet->GetGUID())
        return;

    if (player_pet->GetTPs() < 1)
        return;

    DBC::Structures::TalentEntry const* talent_entry = sTalentStore.LookupEntry(talent_id);
    if (talent_entry == nullptr)
        return;

    if (talent_entry->DependsOn > 0)
    {
        DBC::Structures::TalentEntry const* depends_talent = sTalentStore.LookupEntry(talent_entry->DependsOn);
        if (depends_talent == nullptr)
            return;

        bool req_ok = false;
        for (uint8_t i = 0; i < 5; ++i)
        {
            if (depends_talent->RankID[i] != 0)
            {
                if (player_pet->HasSpell(depends_talent->RankID[i]))
                {
                    req_ok = true;
                    break;
                }
            }
        }
        if (!req_ok)
            return;
    }

    if (player_pet->GetSpentTPs() < (talent_entry->Row * 3))
        return;

    if (talent_rank > 0 && talent_entry->RankID[talent_rank - 1] != 0)
        player_pet->RemoveSpell(talent_entry->RankID[talent_rank - 1]);

    SpellInfo* spell_info = sSpellCustomizations.GetSpellInfo(talent_entry->RankID[talent_rank]);
    if (spell_info != nullptr)
    {
        player_pet->AddSpell(spell_info, true);
        player_pet->SetTPs(player_pet->GetTPs() - 1);
    }

    player_pet->SendTalentsToOwner();
}
