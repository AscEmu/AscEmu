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
