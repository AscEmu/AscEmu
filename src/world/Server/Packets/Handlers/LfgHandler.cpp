/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/LFG/LFGMgr.h"
#include "Common.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Packets/CmsgSetLfgComment.h"

using namespace AscEmu::Packets;

void WorldSession::handleLfgSetCommentOpcode(WorldPacket& recvPacket)
{
    CmsgSetLfgComment recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received: CMSG_SET_LFG_COMMENT");

    uint64_t playerGuid = GetPlayer()->getGuid();
    LogDebugFlag(LF_OPCODE, "LfgHandler playerGuid: %lld, comment: %s", playerGuid, recv_packet.comment.c_str());

    sLfgMgr.SetComment(playerGuid, recv_packet.comment);
}

