/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/Channel.hpp"
#include "Chat/ChannelMgr.hpp"
#include "Logging/Logger.hpp"
#include "Server/Packets/CmsgJoinChannel.h"
#include "Server/Packets/SmsgChannelMemberCount.h"
#include "Server/Packets/CmsgChannelDisplayList.h"
#include "Server/Packets/CmsgChannelModerate.h"
#include "Server/Packets/CmsgChannelAnnouncements.h"
#include "Server/Packets/CmsgChannelUnban.h"
#include "Server/Packets/CmsgChannelBan.h"
#include "Server/Packets/CmsgChannelKick.h"
#include "Server/Packets/CmsgChannelInvite.h"
#include "Server/Packets/CmsgChannelUnmute.h"
#include "Server/Packets/CmsgChannelMute.h"
#include "Server/Packets/CmsgChannelUnmoderator.h"
#include "Server/Packets/CmsgChannelModerator.h"
#include "Server/Packets/CmsgChannelOwner.h"
#include "Server/Packets/CmsgChannelSetOwner.h"
#include "Server/Packets/CmsgChannelPassword.h"
#include "Server/Packets/CmsgChannelList.h"
#include "Server/Packets/CmsgLeaveChannel.h"
#include "Server/Packets/CmsgGetChannelMemberCount.h"
#include "Server/WorldSession.h"
#include "Server/World.h"
#include "Management/ObjectMgr.hpp"
#include "Utilities/Strings.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleChannelJoin(WorldPacket& recvPacket)
{
    CmsgJoinChannel srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (!sWorld.settings.gm.gmClientChannelName.empty() && AscEmu::Util::Strings::isEqual(sWorld.settings.gm.gmClientChannelName.c_str(), srlPacket.channelName.c_str()) && !hasPermissions())
        return;

    const auto channel = sChannelMgr.getOrCreateChannel(srlPacket.channelName, _player, srlPacket.dbcId);
    if (channel == nullptr)
        return;

    channel->attemptJoin(_player, srlPacket.password.c_str());
    sLogger.debug("ChannelJoin {}", srlPacket.channelName);
}

void WorldSession::handleGetChannelMemberCount(WorldPacket& recvPacket)
{
    CmsgGetChannelMemberCount srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    if (channel)
        SendPacket(SmgsChannelMemberCount(srlPacket.name, channel->getChannelFlags(), uint32_t(channel->getMemberCount())).serialise().get());
}

void WorldSession::handleChannelLeave(WorldPacket& recvPacket)
{
    CmsgLeaveChannel srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    if (channel)
        channel->leaveChannel(_player);
}

void WorldSession::handleChannelList(WorldPacket& recvPacket)
{
    CmsgChannelList srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    if (channel)
        channel->listMembers(_player, true);
}

void WorldSession::handleChannelPassword(WorldPacket& recvPacket)
{
    CmsgChannelPassword srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    if (channel)
        channel->setPassword(_player, srlPacket.password.c_str());
}

void WorldSession::handleChannelSetOwner(WorldPacket& recvPacket)
{
    CmsgChannelSetOwner srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    const auto player = sObjectMgr.getPlayer(srlPacket.setName.c_str(), false);
    if (channel && player)
        channel->setOwner(_player, player);
}

void WorldSession::handleChannelOwner(WorldPacket& recvPacket)
{
    CmsgChannelOwner srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    if (channel)
        channel->getOwner(_player);
}

void WorldSession::handleChannelModerator(WorldPacket& recvPacket)
{
    CmsgChannelModerator srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    const auto player = sObjectMgr.getPlayer(srlPacket.modName.c_str(), false);
    if (channel && player)
        channel->giveModerator(_player, player);
}

void WorldSession::handleChannelUnmoderator(WorldPacket& recvPacket)
{
    CmsgChannelUnmoderator srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    const auto player = sObjectMgr.getPlayer(srlPacket.unmodName.c_str(), false);
    if (channel && player)
        channel->takeModerator(_player, player);
}

void WorldSession::handleChannelMute(WorldPacket& recvPacket)
{
    CmsgChannelMute srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    const auto player = sObjectMgr.getPlayer(srlPacket.muteName.c_str(), false);
    if (channel && player)
        channel->mutePlayer(_player, player);
}

void WorldSession::handleChannelUnmute(WorldPacket& recvPacket)
{
    CmsgChannelUnmute srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    const auto player = sObjectMgr.getPlayer(srlPacket.unmuteName.c_str(), false);
    if (channel && player)
        channel->unMutePlayer(_player, player);
}

void WorldSession::handleChannelInvite(WorldPacket& recvPacket)
{
    CmsgChannelInvite srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    const auto player = sObjectMgr.getPlayer(srlPacket.inviteName.c_str(), false);
    if (channel && player)
        channel->invitePlayer(_player, player);
}

void WorldSession::handleChannelKick(WorldPacket& recvPacket)
{
    CmsgChannelKick srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    const auto player = sObjectMgr.getPlayer(srlPacket.kickName.c_str(), false);
    if (channel && player)
        channel->kickOrBanPlayer(_player, player, false);
}

void WorldSession::handleChannelBan(WorldPacket& recvPacket)
{
    CmsgChannelBan srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    const auto player = sObjectMgr.getPlayer(srlPacket.banName.c_str(), false);
    if (channel && player)
        channel->kickOrBanPlayer(_player, player, true);
}

void WorldSession::handleChannelUnban(WorldPacket& recvPacket)
{
    CmsgChannelUnban srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    const auto playerInfo = sObjectMgr.getCachedCharacterInfoByName(srlPacket.unbanName);
    if (channel && playerInfo)
        channel->unBanPlayer(_player, playerInfo);
}

void WorldSession::handleChannelAnnounce(WorldPacket& recvPacket)
{
    CmsgChannelAnnouncements srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    if (channel)
        channel->enableAnnouncements(_player);
}

void WorldSession::handleChannelModerate(WorldPacket& recvPacket)
{
    CmsgChannelModerate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    if (channel)
        channel->moderateChannel(_player);
}

void WorldSession::handleChannelRosterQuery(WorldPacket& recvPacket)
{
    CmsgChannelDisplayList srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = sChannelMgr.getChannel(srlPacket.name, _player);
    if (channel)
        channel->listMembers(_player, false);
}
