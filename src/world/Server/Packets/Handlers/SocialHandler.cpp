/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/WorldSession.h"
#include "Objects/Units/Players/Player.hpp"
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

    _player->sendFriendLists(srlPacket.list_flag);
}

void WorldSession::handleAddFriendOpcode(WorldPacket& recvPacket)
{
    CmsgAddFriend srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->addToFriendList(srlPacket.name, !srlPacket.note.empty() ? srlPacket.note : "");
}

void WorldSession::handleDelFriendOpcode(WorldPacket& recvPacket)
{
    CmsgDelFriend srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->removeFromFriendList(static_cast<uint32_t>(srlPacket.guid));
}

void WorldSession::handleAddIgnoreOpcode(WorldPacket& recvPacket)
{
    CmsgAddIgnore srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->addToIgnoreList(srlPacket.name);
}

void WorldSession::handleDelIgnoreOpcode(WorldPacket& recvPacket)
{
    CmsgDelIgnore srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->removeFromIgnoreList(static_cast<uint32_t>(srlPacket.guid));
}

void WorldSession::handleSetFriendNote(WorldPacket& recvPacket)
{
    CmsgSetContactNotes srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->addNoteToFriend(static_cast<uint32_t>(srlPacket.guid), srlPacket.note.size() ? srlPacket.note : "");
}
