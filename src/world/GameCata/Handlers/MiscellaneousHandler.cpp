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
    data << uint32_t(time(nullptr));
    SendPacket(&data);
}

void WorldSession::HandleTimeSyncRespOpcode(WorldPacket& recv_data)
{
    uint32_t counter;
    uint32_t clientTicks;
    recv_data >> counter;
    recv_data >> clientTicks;
}

void WorldSession::HandleObjectUpdateFailedOpcode(WorldPacket& recv_data)
{
    uint8_t guid[8];
    guid[6] = recv_data.readBit();
    guid[7] = recv_data.readBit();
    guid[4] = recv_data.readBit();
    guid[0] = recv_data.readBit();
    guid[1] = recv_data.readBit();
    guid[5] = recv_data.readBit();
    guid[3] = recv_data.readBit();
    guid[2] = recv_data.readBit();

    recv_data.ReadByteSeq(guid[6]);
    recv_data.ReadByteSeq(guid[7]);
    recv_data.ReadByteSeq(guid[2]);
    recv_data.ReadByteSeq(guid[3]);
    recv_data.ReadByteSeq(guid[1]);
    recv_data.ReadByteSeq(guid[4]);
    recv_data.ReadByteSeq(guid[0]);
    recv_data.ReadByteSeq(guid[5]);

    uint64_t playerguid = *(uint64_t*)guid;
    LogError("HandleObjectUpdateFailedOpcode : Object update failed for playerguid %u", Arcemu::Util::GUID_LOPART(playerguid));

    if (_player->GetGUID() == playerguid)
    {
        LogoutPlayer(true);
        return;
    }
}

void WorldSession::HandleRequestHotfix(WorldPacket& recv_data)
{
    uint32_t type;
    recv_data >> type;

    uint32_t count = recv_data.readBits(23);

    ObjectGuid* guids = new ObjectGuid[count];
    for (uint32_t i = 0; i < count; ++i)
    {
        guids[i][0] = recv_data.readBit();
        guids[i][4] = recv_data.readBit();
        guids[i][7] = recv_data.readBit();
        guids[i][2] = recv_data.readBit();
        guids[i][5] = recv_data.readBit();
        guids[i][3] = recv_data.readBit();
        guids[i][6] = recv_data.readBit();
        guids[i][1] = recv_data.readBit();
    }

    uint32_t entry;
    for (uint32_t i = 0; i < count; ++i)
    {
        recv_data.ReadByteSeq(guids[i][5]);
        recv_data.ReadByteSeq(guids[i][6]);
        recv_data.ReadByteSeq(guids[i][7]);
        recv_data.ReadByteSeq(guids[i][0]);
        recv_data.ReadByteSeq(guids[i][1]);
        recv_data.ReadByteSeq(guids[i][3]);
        recv_data.ReadByteSeq(guids[i][4]);
        recv_data >> entry;
        recv_data.ReadByteSeq(guids[i][2]);

        /*switch (type)
        {
            case DB2_REPLY_ITEM:
                SendItemDb2Reply(entry);
                break;
            case DB2_REPLY_SPARSE:
                SendItemSparseDb2Reply(entry);
                break;
            default:
                LogDebug("WorldSession::HandleRequestHotfix : Received unknown hotfix type %u", type);
                recv_data.clear();
                break;
        }*/
    }
}

void WorldSession::HandleRequestCemeteryListOpcode(WorldPacket& recv_data)
{
    LOG_DEBUG("Received CMSG_REQUEST_CEMETERY_LIST");

    QueryResult* result = WorldDatabase.Query("SELECT id FROM graveyards WHERE faction = %u OR faction = 3;", _player->GetTeam());
    if (result)
    {
        WorldPacket data(SMSG_REQUEST_CEMETERY_LIST_RESPONSE);
        data.writeBit(false);               //unk bit
        data.flushBits();
        data.writeBits(result->GetRowCount(), 24);
        data.flushBits();

        do
        {
            Field* field = result->Fetch();
            data << uint32_t(field[0].GetUInt32());
        } while (result->NextRow());
        delete result;

        SendPacket(&data);
    }
}

void WorldSession::HandleCorpseReclaimOpcode(WorldPacket& recv_data)
{
    uint64_t guid;
    recv_data >> guid;

    Corpse* pCorpse = objmgr.GetCorpseByOwner(GetPlayer()->GetLowGUID());
    if (pCorpse == nullptr)
        return;

    GetPlayer()->ResurrectPlayer();
    GetPlayer()->SetHealth(GetPlayer()->GetMaxHealth() / 2);
}
