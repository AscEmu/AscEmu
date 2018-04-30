/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/WorldSession.h"
#include "Log.hpp"
#include "Units/Players/Player.h"
#include "Server/Packets/CmsgDelIgnore.h"
#include "Server/Packets/CmsgAddIgnore.h"
#include "Server/Packets/CmsgDelFriend.h"
#include "Server/Packets/CmsgAddFriend.h"
#include "Server/Packets/CmsgContactList.h"
#include "Server/Packets/CmsgSetContactNotes.h"

using namespace AscEmu::Packets;

void WorldSession::handleFriendListOpcode(WorldPacket& recvPacket)
{
    CmsgContactList recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    _player->Social_SendFriendList(recv_packet.list_flag);
}

void WorldSession::handleAddFriendOpcode(WorldPacket& recvPacket)
{
    CmsgAddFriend recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    _player->Social_AddFriend(recv_packet.name.c_str(), recv_packet.note.size() ? recv_packet.note.c_str() : nullptr);
}

void WorldSession::handleDelFriendOpcode(WorldPacket& recvPacket)
{
    CmsgDelFriend recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    _player->Social_RemoveFriend((uint32)recv_packet.guid);
}

void WorldSession::handleAddIgnoreOpcode(WorldPacket& recvPacket)
{
    CmsgAddIgnore recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    _player->Social_AddIgnore(recv_packet.name.c_str());
}

void WorldSession::handleDelIgnoreOpcode(WorldPacket& recvPacket)
{
    CmsgDelIgnore recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    _player->Social_RemoveIgnore((uint32)recv_packet.guid);
}

void WorldSession::HandleSetFriendNote(WorldPacket& recvPacket)
{
    CmsgSetContactNotes recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    _player->Social_SetNote((uint32)recv_packet.guid, recv_packet.note.size() ? recv_packet.note.c_str() : nullptr);
}
