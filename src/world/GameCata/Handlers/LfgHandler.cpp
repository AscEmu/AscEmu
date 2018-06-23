/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/LFG/LFGMgr.h"
#include "Storage/MySQLDataStore.hpp"

void WorldSession::HandleLfgLockInfoOpcode(WorldPacket& recv_data)
{
    bool requestFromPlayer = recv_data.readBit();
    LogNotice("CMSG_LFG_LOCK_INFO_REQUEST received from %s", requestFromPlayer ? "player" : "group");

    //\todo handle player lock info and group lock info here
}
