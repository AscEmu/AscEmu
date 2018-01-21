/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#include "Util.hpp"

void WorldSession::HandleLoadScreenOpcode(WorldPacket& recvData)
{
    uint32_t mapId;

    recvData >> mapId;
    recvData.readBit();
}

void WorldSession::HandleReadyForAccountDataTimesOpcode(WorldPacket& /*recvData*/)
{
    SendAccountDataTimes(GLOBAL_CACHE_MASK);
}

void WorldSession::HandleUITimeRequestOpcode(WorldPacket& /*recvData*/)
{
    WorldPacket data(SMSG_UI_TIME, 4);
    data << uint32_t(time(nullptr));
    SendPacket(&data);
}

void WorldSession::HandleTimeSyncRespOpcode(WorldPacket& recvData)
{
    uint32_t counter;
    uint32_t clientTicks;
    recvData >> counter;
    recvData >> clientTicks;
}

void WorldSession::HandleObjectUpdateFailedOpcode(WorldPacket& recvData)
{
    ObjectGuid guid;

    guid[6] = recvData.readBit();
    guid[7] = recvData.readBit();
    guid[4] = recvData.readBit();
    guid[0] = recvData.readBit();
    guid[1] = recvData.readBit();
    guid[5] = recvData.readBit();
    guid[3] = recvData.readBit();
    guid[2] = recvData.readBit();

    recvData.ReadByteSeq(guid[6]);
    recvData.ReadByteSeq(guid[7]);
    recvData.ReadByteSeq(guid[2]);
    recvData.ReadByteSeq(guid[3]);
    recvData.ReadByteSeq(guid[1]);
    recvData.ReadByteSeq(guid[4]);
    recvData.ReadByteSeq(guid[0]);
    recvData.ReadByteSeq(guid[5]);

    LogError("HandleObjectUpdateFailedOpcode : Object update failed for playerguid %u", Arcemu::Util::GUID_LOPART(guid));

    if (_player == nullptr)
    {
        return;
    }

    if (_player->GetGUID() == guid)
    {
        LogoutPlayer(true);
        return;
    }

    //_player->UpdateVisibility();
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

void WorldSession::HandleRequestCemeteryListOpcode(WorldPacket& /*recvData*/)
{
    LOG_DEBUG("Received CMSG_REQUEST_CEMETERY_LIST");

    QueryResult* result = WorldDatabase.Query("SELECT id FROM graveyards WHERE faction = %u OR faction = 3;", _player->GetTeam());
    if (result)
    {
        WorldPacket data(SMSG_REQUEST_CEMETERY_LIST_RESPONSE, 8 * result->GetRowCount());
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

void WorldSession::HandleUpdateAccountData(WorldPacket& recv_data)
{
    if (worldConfig.server.useAccountData == false)
    {
        return;
    }

    uint32_t uiType;
    uint32_t uiTimestamp;
    uint32_t uiDecompressedSize;
    
    recv_data >> uiType;
    recv_data >> uiTimestamp;
    recv_data >> uiDecompressedSize;

    if (uiType > 8)
    {
        LOG_ERROR("WARNING: Accountdata uiType > 8 (%u) was requested to be updated by %s of account %d!",
            uiType, GetPlayer()->GetName(), this->GetAccountId());
        return;
    }

    uLongf uid = uiDecompressedSize;

    if (uiDecompressedSize == 0)
    {
        SetAccountData(uiType, NULL, false, 0);
        WorldPacket rdata(SMSG_UPDATE_ACCOUNT_DATA_COMPLETE, 4 + 4);
        rdata << uint32_t(uiType);
        rdata << uint32_t(0);
        SendPacket(&rdata);
        return;
    }

    if (uiDecompressedSize >= 0xFFFF)
    {
        recv_data.rfinish();
        return;
    }

    size_t ReceivedPackedSize = recv_data.size() - 8;
    char* data = new char[uiDecompressedSize + 1];
    memset(data, 0, uiDecompressedSize + 1);

    if (uiDecompressedSize > ReceivedPackedSize)
    {
        int32_t ZlibResult;

        ZlibResult = uncompress((uint8_t*)data, &uid, recv_data.contents() + 8, (uLong)ReceivedPackedSize);

        switch (ZlibResult)
        {
            case Z_OK:				  //0 no error decompression is OK
            {
                SetAccountData(uiType, data, false, uiDecompressedSize);
                LOG_DETAIL("Successfully decompressed account data %u for %s, and updated storage array.",
                    uiType, GetPlayer()->GetName());
            } break;
            case Z_ERRNO:			    //-1
            case Z_STREAM_ERROR:		//-2
            case Z_DATA_ERROR:		    //-3
            case Z_MEM_ERROR:		    //-4
            case Z_BUF_ERROR:		    //-5
            case Z_VERSION_ERROR:	    //-6
            {
                delete[] data;
                LOG_ERROR("Decompression of account data %u for %s FAILED.", uiType, GetPlayer()->GetName());
            } break;
            default:
            {
                delete[] data;
                LOG_ERROR("Decompression gave a unknown error: %x, of account data %u for %s FAILED.",
                    ZlibResult, uiType, GetPlayer()->GetName());
            } break;
        }
    }
    else
    {
        memcpy(data, recv_data.contents() + 8, uiDecompressedSize);
        SetAccountData(uiType, data, false, uiDecompressedSize);
    }

    WorldPacket rdata(SMSG_UPDATE_ACCOUNT_DATA_COMPLETE, 4 + 4);
    rdata << uint32_t(uiType);
    rdata << uint32_t(0);
    SendPacket(&rdata);
}

void WorldSession::HandleReturnToGraveyardOpcode(WorldPacket& /*recv_data*/)
{
    if (_player->isAlive())
    {
        return;
    }

    _player->RepopAtGraveyard(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetMapId());
}

void WorldSession::HandleBugOpcode(WorldPacket& recv_data)
{
    uint8_t unk1;
    uint8_t unk2;

    recv_data >> unk1;
    recv_data >> unk2;

    uint32_t lenght = 0;
    lenght = unk1 * 16;
    lenght += unk2 / 16;

    std::string bugMessage;
    bugMessage = recv_data.ReadString(lenght);   // message

    LOG_DEBUG("Received CMSG_BUG [Bug Report] lenght: %u message: %s", lenght, bugMessage.c_str());

    uint64_t AccountId = GetAccountId();
    uint32_t TimeStamp = uint32_t(UNIXTIME);
    uint32_t ReportID = objmgr.GenerateReportID();

    std::stringstream ss;

    ss << "INSERT INTO playerbugreports VALUES('";
    ss << ReportID << "','";
    ss << AccountId << "','";
    ss << TimeStamp << "',";
    ss << "'0',";               // 0 = bug
    ss << "'0','";              // 0 = bug
    ss << CharacterDatabase.EscapeString(bugMessage) << "')";

    CharacterDatabase.ExecuteNA(ss.str().c_str());
}

void WorldSession::HandleSuggestionOpcode(WorldPacket& recv_data)
{
    uint8_t unk1;
    uint8_t unk2;

    recv_data >> unk1;
    recv_data >> unk2;

    uint32_t lenght = 0;
    lenght = unk1 * 16;
    lenght += unk2 / 16;

    std::string suggestionMessage;
    suggestionMessage = recv_data.ReadString(lenght);   // message

    LOG_DEBUG("Received CMSG_SUGGESTIONS [Suggestion] lenght: %u message: %s", lenght, suggestionMessage.c_str());

    uint64_t AccountId = GetAccountId();
    uint32_t TimeStamp = uint32_t(UNIXTIME);
    uint32_t ReportID = objmgr.GenerateReportID();

    std::stringstream ss;

    ss << "INSERT INTO playerbugreports VALUES('";
    ss << ReportID << "','";
    ss << AccountId << "','";
    ss << TimeStamp << "',";
    ss << "'1',";               // 1 = suggestion
    ss << "'1','";              // 1 = suggestion
    ss << CharacterDatabase.EscapeString(suggestionMessage) << "')";

    CharacterDatabase.ExecuteNA(ss.str().c_str());
}

void WorldSession::HandleLogDisconnectOpcode(WorldPacket& recv_data)
{
    uint32 disconnectReason;
    recv_data >> disconnectReason;

    // 13 = close window

    LOG_DEBUG("Player %s disconnected on %s - Reason %u", _player->GetName(), Util::GetCurrentDateTimeString().c_str(), disconnectReason);
}
