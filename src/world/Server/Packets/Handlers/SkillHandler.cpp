/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

void WorldSession::HandleLearnTalentOpcode(WorldPacket& recv_data)
{
    LOG_DEBUG("Recieved packet CMSG_LEARN_TALENT.");

    uint32 talentId;
    uint32 requestedRank;

    recv_data >> talentId;
    recv_data >> requestedRank;

    _player->LearnTalent(talentId, requestedRank);
}

void WorldSession::HandleLearnPreviewTalentsOpcode(WorldPacket& recv_data)
{
    LOG_DEBUG("Recieved packet CMSG_LEARN_PREVIEW_TALENTS.");

    int32 currentTab;
    uint32 talentCount;
    uint32 talentId;
    uint32 talentRank;

    //if currentTab -1 player has already the spec.
    recv_data >> currentTab;
    recv_data >> talentCount;

    for (uint32 i = 0; i < talentCount; ++i)
    {
        recv_data >> talentId;
        recv_data >> talentRank;

        _player->LearnTalent(talentId, talentRank, true);
    }

    _player->smsg_TalentsInfo(false);
}

void WorldSession::HandlePetLearnTalentOpcode(WorldPacket& recvPacket)
{
    uint64 guid;
    uint32 talentId;
    uint32 talentRank;

    recvPacket >> guid;
    recvPacket >> talentId;
    recvPacket >> talentRank;

    Pet* player_pet = _player->GetSummon();
    if (player_pet == nullptr)
        return;

    if (guid != player_pet->GetGUID())
        return;

    if (player_pet->GetTPs() < 1)
        return;

    DBC::Structures::TalentEntry const* talent_entry = sTalentStore.LookupEntry(talentId);
    if (talent_entry == nullptr)
        return;

    if (talent_entry->DependsOn > 0)
    {
        DBC::Structures::TalentEntry const* depends_talent = sTalentStore.LookupEntry(talent_entry->DependsOn);
        if (depends_talent == nullptr)
            return;

        bool req_ok = false;
        for (uint8 i = 0; i < 5; ++i)
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

    if (talentRank > 0 && talent_entry->RankID[talentRank - 1] != 0)
        player_pet->RemoveSpell(talent_entry->RankID[talentRank - 1]);

    SpellInfo* spell_info = sSpellCustomizations.GetSpellInfo(talent_entry->RankID[talentRank]);
    if (spell_info != nullptr)
    {
        player_pet->AddSpell(spell_info, true);
        player_pet->SetTPs(player_pet->GetTPs() - 1);
    }

    player_pet->SendTalentsToOwner();
}

void WorldSession::HandleTalentWipeConfirmOpcode(WorldPacket& recv_data)
{
    uint32 wipe_price = _player->CalcTalentResetCost(GetPlayer()->GetTalentResetTimes());
    if (!_player->HasGold(wipe_price))
        return;

    _player->SetTalentResetTimes(GetPlayer()->GetTalentResetTimes() + 1);
    _player->ModGold(-(int32)wipe_price);
    _player->Reset_Talents();
}

void WorldSession::HandleUnlearnSkillOpcode(WorldPacket& recv_data)
{
    uint32 skillId;
    recv_data >> skillId;

    uint32 primaryPoints = _player->GetPrimaryProfessionPoints();
    _player->RemoveSpellsFromLine(skillId);
    _player->_RemoveSkillLine(skillId);

    if (primaryPoints == _player->GetPrimaryProfessionPoints())
    {
        DBC::Structures::SkillLineEntry const* skill_line_entry = sSkillLineStore.LookupEntry(skillId);
        if (skill_line_entry == nullptr)
            return;

        if (skill_line_entry->type == SKILL_TYPE_PROFESSION && primaryPoints < 2)
            _player->SetPrimaryProfessionPoints(primaryPoints + 1);
    }
}
