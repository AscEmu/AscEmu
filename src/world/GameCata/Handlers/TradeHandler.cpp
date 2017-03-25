/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/Container.h"
#include "Management/ItemInterface.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Server/WorldSession.h"
#include "Server/World.h"
#include "Objects/ObjectMgr.h"


void WorldSession::HandleInitiateTradeOpcode(WorldPacket& recv_data)
{
    ObjectGuid target_guid;

    target_guid[0] = recv_data.readBit();
    target_guid[3] = recv_data.readBit();
    target_guid[5] = recv_data.readBit();
    target_guid[1] = recv_data.readBit();
    target_guid[4] = recv_data.readBit();
    target_guid[6] = recv_data.readBit();
    target_guid[7] = recv_data.readBit();
    target_guid[2] = recv_data.readBit();

    recv_data.ReadByteSeq(target_guid[7]);
    recv_data.ReadByteSeq(target_guid[4]);
    recv_data.ReadByteSeq(target_guid[3]);
    recv_data.ReadByteSeq(target_guid[5]);
    recv_data.ReadByteSeq(target_guid[1]);
    recv_data.ReadByteSeq(target_guid[2]);
    recv_data.ReadByteSeq(target_guid[6]);
    recv_data.ReadByteSeq(target_guid[0]);


    Player* player_target = _player->GetMapMgr()->GetPlayer((uint32_t)target_guid);
    uint32_t trade_status = TRADE_STATUS_PROPOSED;
    if (player_target == nullptr)
    {
        trade_status = TRADE_STATUS_PLAYER_NOT_FOUND;

        OutPacket(SMSG_TRADE_STATUS, 4, &trade_status);
        return;
    }

    // errors
    if (player_target->CalcDistance(_player) > 10.0f)
        trade_status = TRADE_STATUS_TOO_FAR_AWAY;
    else if (player_target->IsDead())
        trade_status = TRADE_STATUS_DEAD;
    else if (player_target->mTradeTarget != 0)
        trade_status = TRADE_STATUS_ALREADY_TRADING;
    else if (player_target->GetTeam() != _player->GetTeam() && GetPermissionCount() == 0 && !sWorld.interfaction_trade)
        trade_status = TRADE_STATUS_WRONG_FACTION;

    if (trade_status == TRADE_STATUS_PROPOSED)
    {
        _player->ResetTradeVariables();
        player_target->ResetTradeVariables();

        player_target->mTradeTarget = _player->GetLowGUID();
        _player->mTradeTarget = player_target->GetLowGUID();

        player_target->mTradeStatus = trade_status;
        _player->mTradeStatus = trade_status;
    }

    ObjectGuid source_guid = _player->GetGUID();

    WorldPacket data(SMSG_TRADE_STATUS, 12);
    data.writeBit(false);
    data.writeBits(12, 5);                      // 12 = status success
    data.WriteByteMask(source_guid[2]);
    data.WriteByteMask(source_guid[4]);
    data.WriteByteMask(source_guid[6]);
    data.WriteByteMask(source_guid[0]);
    data.WriteByteMask(source_guid[1]);
    data.WriteByteMask(source_guid[3]);
    data.WriteByteMask(source_guid[7]);
    data.WriteByteMask(source_guid[5]);

    data.WriteByteSeq(source_guid[4]);
    data.WriteByteSeq(source_guid[1]);
    data.WriteByteSeq(source_guid[2]);
    data.WriteByteSeq(source_guid[3]);
    data.WriteByteSeq(source_guid[0]);
    data.WriteByteSeq(source_guid[7]);
    data.WriteByteSeq(source_guid[6]);
    data.WriteByteSeq(source_guid[5]);

    data << uint32_t(0);              // unk

    player_target->m_session->SendPacket(&data);
}

void WorldSession::HandleBeginTradeOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint32_t trade_status = TRADE_STATUS_INITIATED;

    Player* target_player = _player->GetTradeTarget();
    if (_player->mTradeTarget == 0 || target_player == nullptr)
    {
        trade_status = TRADE_STATUS_PLAYER_NOT_FOUND;
        OutPacket(SMSG_TRADE_STATUS, 4, &trade_status);
        return;
    }

    if (_player->CalcDistance(objmgr.GetPlayer(_player->mTradeTarget)) > 10.0f)
        trade_status = TRADE_STATUS_TOO_FAR_AWAY;

    WorldPacket data(SMSG_TRADE_STATUS, 8);
    data << uint32_t(trade_status);
    data << uint32_t(0x19);

    target_player->m_session->SendPacket(&data);
    _player->SendPacket(&data);

    target_player->mTradeStatus = trade_status;
    _player->mTradeStatus = trade_status;
}
