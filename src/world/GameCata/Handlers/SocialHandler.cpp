/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/WorldSession.h"
#include "Log.hpp"
#include "Units/Players/Player.h"

//\todo Rewrite for cata - after this all functions are copied from wotlk

void WorldSession::HandleFriendListOpcode(WorldPacket& recvData)
{
    uint32 flag;
    recvData >> flag;
    _player->Social_SendFriendList(flag);
}

void WorldSession::HandleAddFriendOpcode(WorldPacket& recvData)
{
    LOG_DEBUG("WORLD: Received CMSG_ADD_FRIEND");

    std::string name, note;
    recvData >> name;
    recvData >> note;

    _player->Social_AddFriend(name.c_str(), note.size() ? note.c_str() : NULL);
}

void WorldSession::HandleDelFriendOpcode(WorldPacket& recvData)
{
    LOG_DEBUG("WORLD: Received CMSG_DEL_FRIEND");

    uint64 friendGuid;
    recvData >> friendGuid;

    _player->Social_RemoveFriend(static_cast<uint32>(friendGuid));
}

void WorldSession::HandleAddIgnoreOpcode(WorldPacket& recvData)
{
    LOG_DEBUG("WORLD: Received CMSG_ADD_IGNORE");

    std::string ignoreName = "UNKNOWN";
    recvData >> ignoreName;

    _player->Social_AddIgnore(ignoreName.c_str());
}

void WorldSession::HandleDelIgnoreOpcode(WorldPacket& recvData)
{
    LOG_DEBUG("WORLD: Received CMSG_DEL_IGNORE");

    uint64 ignoreGuid;
    recvData >> ignoreGuid;

    _player->Social_RemoveIgnore(static_cast<uint32>(ignoreGuid));
}

void WorldSession::HandleSetFriendNote(WorldPacket& recvData)
{
    uint64 guid;
    std::string note;

    recvData >> guid >> note;
    _player->Social_SetNote(static_cast<uint32>(guid), note.size() ? note.c_str() : NULL);
}
