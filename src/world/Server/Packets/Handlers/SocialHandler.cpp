/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
    CmsgContactList srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->Social_SendFriendList(srlPacket.list_flag);
}

void WorldSession::handleAddFriendOpcode(WorldPacket& recvPacket)
{
    CmsgAddFriend srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->Social_AddFriend(srlPacket.name.c_str(), srlPacket.note.size() ? srlPacket.note.c_str() : nullptr);
}

void WorldSession::handleDelFriendOpcode(WorldPacket& recvPacket)
{
    CmsgDelFriend srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->Social_RemoveFriend((uint32)srlPacket.guid);
}

void WorldSession::handleAddIgnoreOpcode(WorldPacket& recvPacket)
{
    CmsgAddIgnore srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->Social_AddIgnore(srlPacket.name.c_str());
}

void WorldSession::handleDelIgnoreOpcode(WorldPacket& recvPacket)
{
    CmsgDelIgnore srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->Social_RemoveIgnore((uint32)srlPacket.guid);
}

void WorldSession::handleSetFriendNote(WorldPacket& recvPacket)
{
    CmsgSetContactNotes srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->Social_SetNote((uint32)srlPacket.guid, srlPacket.note.size() ? srlPacket.note.c_str() : nullptr);
}
