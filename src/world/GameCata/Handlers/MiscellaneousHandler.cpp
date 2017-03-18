/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "git_version.h"
#include "AuthCodes.h"
#include "Management/WordFilter.h"
#include "Management/ArenaTeam.h"
#include "Management/Battleground/Battleground.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Players/PlayerClasses.hpp"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "Map/MapMgr.h"
#include "Map/WorldCreator.h"

void WorldSession::HandleLoadScreenOpcode(WorldPacket& recv_data)
{
    uint32_t mapId;

    recv_data >> mapId;
    recv_data.readBit();
}

void WorldSession::HandleReadyForAccountDataTimesOpcode(WorldPacket& recv_data)
{
    SendAccountDataTimes(GLOBAL_CACHE_MASK);
}

void WorldSession::HandleUITimeRequestOpcode(WorldPacket& recv_data)
{
    WorldPacket data(SMSG_UI_TIME, 4);
    data << uint32(time(NULL));
    SendPacket(&data);
}

void WorldSession::HandleTimeSyncRespOpcode(WorldPacket& recv_data)
{
    uint32 counter, clientTicks;
    recv_data >> counter >> clientTicks;
}
