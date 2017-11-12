/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/WorldSession.h"

void WorldSession::HandleRequestRatedBgInfoOpcode(WorldPacket & recv_data)
{
    uint8_t unk_type;
    recv_data >> unk_type;

    LOG_DEBUG("CMSG_REQUEST_RATED_BG_INFO received with unk_type = %u", unk_type);

    WorldPacket data(SMSG_RATED_BG_INFO, 72);
    for (int i = 0; i < 18; ++i)
    {
        data << uint32_t(0);    // unknown
    }

    SendPacket(&data);
}

void WorldSession::HandleRequestRatedBgStatsOpcode(WorldPacket& /*recv_data*/)
{
    LOG_DEBUG("CMSG_REQUEST_RATED_BG_STATS received");

    WorldPacket data(SMSG_RATED_BG_STATS, 29);
    data << uint32_t(0);    // unknown
    data << uint8_t(3);     // unknown - always 3?... type?
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown

    SendPacket(&data);
}

void WorldSession::HandleRequestPvPRewardsOpcode(WorldPacket& /*recv_data*/)
{
    LOG_DEBUG("CMSG_REQUEST_RATED_BG_STATS received");

    WorldPacket packet(SMSG_REQUEST_PVP_REWARDS_RESPONSE, 24);
    packet << uint32_t(0);    // unknown currency week cap conquest points
    packet << uint32_t(0);    // unknown currency on week conquest points
    packet << uint32_t(0);    // unknown currency week cap conquest arena
    packet << uint32_t(0);    // unknown currency on week conquest random baattleground
    packet << uint32_t(0);    // unknown currency on week conquest arena
    packet << uint32_t(0);    // unknown currency week cap conquest points

    SendPacket(&packet);
}

void WorldSession::HandleRequestPvpOptionsOpcode(WorldPacket& /*recv_data*/)
{
    LOG_DEBUG("CMSG_REQUEST_RATED_BG_STATS received");

    WorldPacket data(SMSG_PVP_OPTIONS_ENABLED, 1);
    data.writeBit(1);       // unknown 
    data.writeBit(1);       // unknown wargames enabled
    data.writeBit(1);       // unknown 
    data.writeBit(1);       // unknown rated battlegrounds enabled
    data.writeBit(1);       // unknown rated arenas enabled

    data.flushBits();

    SendPacket(&data);
}
