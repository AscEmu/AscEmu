/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/MsgQuestPushResult.h"
#include "Server/Packets/CmsgQuestgiverAcceptQuest.h"
#include "Server/Packets/CmsgQuestQuery.h"

using namespace AscEmu::Packets;

void WorldSession::handleQuestPushResultOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    MsgQuestPushResult srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received MSG_QUEST_PUSH_RESULT");

    if (_player->GetQuestSharer())
    {
        const auto questSharerPlayer = objmgr.GetPlayer(_player->GetQuestSharer());
        if (questSharerPlayer)
        {
            const uint64_t guid = recvPacket.size() >= 13 ? _player->getGuid() : srlPacket.giverGuid;
            questSharerPlayer->GetSession()->SendPacket(MsgQuestPushResult(guid, 0, srlPacket.pushResult).serialise().get());
            _player->SetQuestSharer(0);
        }
    }
}

void WorldSession::handleQuestgiverAcceptQuestOpcode(WorldPacket& recvPacket)
{
    CmsgQuestgiverAcceptQuest srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->AcceptQuest(srlPacket.guid, srlPacket.questId);
}

void WorldSession::handleQuestQueryOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgQuestQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (const auto questProperties = sMySQLStore.getQuestProperties(srlPacket.questId))
    {
        WorldPacket* worldPacket = BuildQuestQueryResponse(questProperties);
        SendPacket(worldPacket);
        delete worldPacket;
    }
    else
    {
        LOG_DEBUG("Invalid quest Id %u.", srlPacket.questId);
    }
}
