/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname;
    uint32_t channelId;

    recvPacket >> channelId;
    recvPacket.readBit();       // has voice
    recvPacket.readBit();       // zone update

    uint32_t channelLength = recvPacket.readBits(8);
    uint32_t passwordLength = recvPacket.readBits(8);

    channelname = recvPacket.ReadString(channelLength);
    std::string pass = recvPacket.ReadString(passwordLength);

    if (sWorld.settings.gm.gmClientChannelName.size() && !stricmp(sWorld.settings.gm.gmClientChannelName.c_str(), channelname.c_str()) && !GetPermissionCount())
        return;

    Channel* channel = channelmgr.GetCreateChannel(channelname.c_str(), _player, channelId);
    if (channel == nullptr)
        return;

    channel->AttemptJoin(_player, pass.c_str());
    LogDebugFlag(LF_OPCODE, "ChannelJoin %s", channelname.c_str());
}

void WorldSession::HandleChatIgnoredOpcode(WorldPacket & recvPacket)
{
    uint8_t unk;
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

//\todo Rewrite for cata - after this all functions are copied from wotlk
void WorldSession::HandleChannelLeave(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname;
    uint32 code = 0;

    recvPacket >> code;
    recvPacket >> channelname;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn == nullptr)
        return;

    chn->Part(_player);
}

void WorldSession::HandleChannelList(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname;

    recvPacket >> channelname;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn != nullptr)
        chn->List(_player);
}

void WorldSession::HandleChannelPassword(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, pass;

    recvPacket >> channelname;
    recvPacket >> pass;
    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn)
        chn->Password(_player, pass.c_str());
}

void WorldSession::HandleChannelSetOwner(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;

    recvPacket >> channelname;
    recvPacket >> newp;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    Player * plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->SetOwner(_player, plr);
}

void WorldSession::HandleChannelOwner(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, pass;

    recvPacket >> channelname;
    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn)
        chn->GetOwner(_player);
}

void WorldSession::HandleChannelModerator(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;

    recvPacket >> channelname;
    recvPacket >> newp;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    Player * plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->GiveModerator(_player, plr);
}

void WorldSession::HandleChannelUnmoderator(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;

    recvPacket >> channelname;
    recvPacket >> newp;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    Player * plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->TakeModerator(_player, plr);
}

void WorldSession::HandleChannelMute(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;

    recvPacket >> channelname;
    recvPacket >> newp;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    Player * plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->Mute(_player, plr);
}

void WorldSession::HandleChannelUnmute(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;

    recvPacket >> channelname;
    recvPacket >> newp;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    Player * plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->Unmute(_player, plr);
}

void WorldSession::HandleChannelInvite(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;

    recvPacket >> channelname;
    recvPacket >> newp;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    Player * plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->Invite(_player, plr);
}
void WorldSession::HandleChannelKick(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;

    recvPacket >> channelname;
    recvPacket >> newp;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    Player * plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->Kick(_player, plr, false);
}

void WorldSession::HandleChannelBan(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;

    recvPacket >> channelname;
    recvPacket >> newp;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    Player * plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->Kick(_player, plr, true);
}

void WorldSession::HandleChannelUnban(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;

    recvPacket >> channelname;
    recvPacket >> newp;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    PlayerInfo * plr = objmgr.GetPlayerInfoByName(newp.c_str());
    if (chn && plr)
        chn->Unban(_player, plr);
}

void WorldSession::HandleChannelAnnounce(WorldPacket& recvPacket)
{
     CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname;
    recvPacket >> channelname;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn)
        chn->Announce(_player);
}

void WorldSession::HandleChannelModerate(WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname;
    recvPacket >> channelname;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn)
        chn->Moderate(_player);
}

void WorldSession::HandleChannelRosterQuery(WorldPacket& recvPacket)
{
    std::string channelname;
    recvPacket >> channelname;

    Channel * chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn)
        chn->List(_player);
}

void WorldSession::HandleChannelNumMembersQuery(WorldPacket& recvPacket)
{
    std::string channel_name;
    WorldPacket data(SMSG_CHANNEL_MEMBER_COUNT, recvPacket.size() + 4);
    recvPacket >> channel_name;
    Channel * chn = channelmgr.GetChannel(channel_name.c_str(), _player);
    if (chn)
    {
        data << channel_name;
        data << uint8(chn->m_flags);
        data << uint32(chn->GetNumMembers());
        SendPacket(&data);
    }
}
