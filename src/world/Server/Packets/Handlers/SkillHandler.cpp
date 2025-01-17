/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/Packets/ManagedPacket.h"
#include "Server/Packets/CmsgUnlearnSkill.h"
#include "Server/Packets/CmsgLearnTalent.h"
#include "Server/Packets/CmsgLearnTalentMultiple.h"
#include "Objects/Units/Players/Player.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleUnlearnSkillOpcode(WorldPacket& recvPacket)
{
    CmsgUnlearnSkill srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->removeSkillLine(static_cast<uint16_t>(srlPacket.skillLineId));
}

void WorldSession::handleLearnTalentOpcode(WorldPacket& recvPacket)
{
    CmsgLearnTalent srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->learnTalent(srlPacket.talentId, srlPacket.requestedRank);
    _player->smsg_TalentsInfo(false);
}

void WorldSession::handleUnlearnTalents(WorldPacket& /*recvPacket*/)
{
    const uint32_t resetPrice = _player->calcTalentResetCost(_player->getTalentResetsCount());
    if (!_player->hasEnoughCoinage(resetPrice))
        return;

    _player->setTalentResetsCount(_player->getTalentResetsCount() + 1);
    _player->modCoinage(-static_cast<int32_t>(resetPrice));
    _player->resetTalents();
}


void WorldSession::handleLearnMultipleTalentsOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING < Cata
#if VERSION_STRING > TBC
    CmsgLearnTalentMultiple srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debug("Recieved CMSG_LEARN_TALENTS_MULTIPLE");

    for (auto learnTalent : srlPacket.multipleTalents)
        _player->learnTalent(learnTalent.talentId, learnTalent.talentRank);

    _player->smsg_TalentsInfo(false);
#endif
#endif
}

void WorldSession::handleLearnPreviewTalentsOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
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
#endif
}
