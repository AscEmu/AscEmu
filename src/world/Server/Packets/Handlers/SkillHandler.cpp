/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/WorldSession.h"
#include "Server/Packets/ManagedPacket.h"

using namespace AscEmu::Packets;


void WorldSession::handleUnlearnSkillOpcode(WorldPacket& recvPacket)
{
    uint32_t skillLineId;

    recvPacket >> skillLineId;

    _player->RemoveSpellsFromLine(skillLineId);
    _player->_RemoveSkillLine(skillLineId);

    uint32_t remainingPoints = _player->getFreePrimaryProfessionPoints();
    if (remainingPoints == _player->getFreePrimaryProfessionPoints())
    {
        const auto skillLineEntry = sSkillLineStore.LookupEntry(skillLineId);
        if (!skillLineEntry)
            return;

        if (skillLineEntry->type == SKILL_TYPE_PROFESSION && remainingPoints < 2)
            _player->setFreePrimaryProfessionPoints(remainingPoints + 1);
    }
}

void WorldSession::handleLearnTalentOpcode(WorldPacket& recvPacket)
{
    uint32_t talentId;
    uint32_t requestedRank;

    recvPacket >> talentId;
    recvPacket >> requestedRank;

    _player->learnTalent(talentId, requestedRank);
    _player->smsg_TalentsInfo(false);
}

void WorldSession::handleUnlearnTalents(WorldPacket& /*recvPacket*/)
{
    const uint32_t resetPrice = _player->CalcTalentResetCost(_player->GetTalentResetTimes());
    if (!_player->HasGold(resetPrice))
        return;

    _player->SetTalentResetTimes(_player->GetTalentResetTimes() + 1);
    _player->ModGold(-static_cast<int32_t>(resetPrice));
    _player->resetTalents();
}

#if VERSION_STRING != Cata
void WorldSession::handleLearnMultipleTalentsOpcode(WorldPacket& recvPacket)
{
    uint32_t talentCount;
    uint32_t talentId;
    uint32_t talentRank;

    LOG_DEBUG("Recieved packet CMSG_LEARN_TALENTS_MULTIPLE.");

    recvPacket >> talentCount;

    for (uint32 i = 0; i < talentCount; ++i)
    {
        recvPacket >> talentId;
        recvPacket >> talentRank;

        _player->learnTalent(talentId, talentRank);
    }

    _player->smsg_TalentsInfo(false);
}
#else
void WorldSession::handleLearnPreviewTalentsOpcode(WorldPacket& recvPacket)
{
    int32_t current_tab;
    uint32_t talent_count;
    uint32_t talent_id;
    uint32_t talent_rank;

    //if currentTab -1 player has already the spec.
    recvPacket >> current_tab;
    recvPacket >> talent_count;

    for (uint32_t i = 0; i < talent_count; ++i)
    {
        recvPacket >> talent_id;
        recvPacket >> talent_rank;

        _player->learnTalent(talent_id, talent_rank);
    }

    _player->smsg_TalentsInfo(false);
}
#endif