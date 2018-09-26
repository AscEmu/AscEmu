/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/MsgQuestPushResult.h"

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
