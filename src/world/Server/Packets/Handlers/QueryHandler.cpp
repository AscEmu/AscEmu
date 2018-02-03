/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#include "Server/Packets/CmsgNameQuery.h"
#include "Server/Packets/SmsgNameQueryResponse.h"
#include "Server/Packets/SmsgQueryTimeResponse.h"
#include "Log.hpp"
#include "Objects/ObjectMgr.h"

using namespace AscEmu::Packets;

#if VERSION_STRING != Cata
void WorldSession::handleNameQueryOpcode(WorldPacket& recvData)
{
    CmsgNameQuery query;
    if (!query.deserialise(recvData))
    {
        Disconnect();
        return;
    }

    const auto info = objmgr.GetPlayerInfo(query.guid.getLowGuid());
    if (!info)
        return;

    LOG_DEBUG("Received CMSG_NAME_QUERY for: %s", info->name);
    SendPacket(SmsgNameQueryResponse(query.guid, info->name, info->race, info->gender, info->cl).serialise().get());
}

void WorldSession::handleQueryTimeOpcode(WorldPacket&)
{
    SendPacket(SmsgQueryTimeResponse(UNIXTIME).serialise().get());
}
#endif