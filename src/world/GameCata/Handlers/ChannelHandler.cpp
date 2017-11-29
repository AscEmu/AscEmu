/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Server/WorldSession.h"
#include "Server/World.h"
#include "Objects/ObjectMgr.h"


void WorldSession::HandleChannelJoin(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, pass;
    uint32 channel_id;

    recvPacket >> channel_id;
    recvPacket.readBit();       // has voice
    recvPacket.readBit();       // zone update

    uint32_t channelLength = recvPacket.readBits(8);
    uint32_t passwordLength = recvPacket.readBits(8);

    channelname = recvPacket.ReadString(channelLength);
    pass = recvPacket.ReadString(passwordLength);

    if (sWorld.settings.gm.gmClientChannelName.size() && !stricmp(sWorld.settings.gm.gmClientChannelName.c_str(), channelname.c_str()) && !GetPermissionCount())
        return;

    Channel* channel = channelmgr.GetCreateChannel(channelname.c_str(), _player, channel_id);
    if (channel == nullptr)
        return;

    channel->AttemptJoin(_player, pass.c_str());
    LogDebugFlag(LF_OPCODE, "ChannelJoin %s", channelname.c_str());
}

void WorldSession::HandleChatIgnoredOpcode(WorldPacket & recvPacket)
{
    uint8 unk;
    recvPacket >> unk;

    ObjectGuid playerGuid;

    playerGuid[5] = recvPacket.readBit();
    playerGuid[2] = recvPacket.readBit();
    playerGuid[6] = recvPacket.readBit();
    playerGuid[4] = recvPacket.readBit();
    playerGuid[7] = recvPacket.readBit();
    playerGuid[0] = recvPacket.readBit();
    playerGuid[1] = recvPacket.readBit();
    playerGuid[3] = recvPacket.readBit();

    recvPacket.ReadByteSeq(playerGuid[0]);
    recvPacket.ReadByteSeq(playerGuid[6]);
    recvPacket.ReadByteSeq(playerGuid[5]);
    recvPacket.ReadByteSeq(playerGuid[1]);
    recvPacket.ReadByteSeq(playerGuid[4]);
    recvPacket.ReadByteSeq(playerGuid[3]);
    recvPacket.ReadByteSeq(playerGuid[7]);
    recvPacket.ReadByteSeq(playerGuid[2]);

    Player* player = objmgr.GetPlayer((uint32)playerGuid);
    if (player == nullptr || player->GetSession() == nullptr)
    {
        return;
    }

    WorldPacket* data = sChatHandler.FillMessageData(CHAT_MSG_IGNORED, LANG_UNIVERSAL, _player->GetName(), _player->GetGUID());
    player->GetSession()->SendPacket(data);
    delete data;
}
