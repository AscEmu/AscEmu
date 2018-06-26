/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/SmsgDuelCountdown.h"
#include "Server/Packets/SmsgDuelComplete.h"
#include "Server/WorldSession.h"
#include "Units/Players/Player.h"

using namespace AscEmu::Packets;

void WorldSession::handleDuelAccepted(WorldPacket& /*recvPacket*/)
{
    const auto duelPlayer = GetPlayer()->DuelingWith;
    if (duelPlayer == nullptr)
        return;

    if (GetPlayer()->m_duelState != DUEL_STATE_FINISHED)
        return;

    if (GetPlayer()->m_duelCountdownTimer > 0)
        return;

    GetPlayer()->m_duelStatus = DUEL_STATUS_INBOUNDS;
    GetPlayer()->m_duelState = DUEL_STATE_STARTED;

    duelPlayer->m_duelStatus = DUEL_STATUS_INBOUNDS;
    duelPlayer->m_duelState = DUEL_STATE_STARTED;

    const uint32_t defaultDuelCountdown = 3000;

    SendPacket(SmsgDuelCountdown(defaultDuelCountdown).serialise().get());
    duelPlayer->SendPacket(SmsgDuelCountdown(defaultDuelCountdown).serialise().get());

    GetPlayer()->m_duelCountdownTimer = defaultDuelCountdown;

    sEventMgr.AddEvent(_player, &Player::DuelCountdown, EVENT_PLAYER_DUEL_COUNTDOWN, 1000, 3, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void WorldSession::handleDuelCancelled(WorldPacket& /*recvPacket*/)
{
    const auto duelPlayer = GetPlayer()->DuelingWith;
    if (duelPlayer == nullptr)
        return;

    if (GetPlayer()->m_duelState == DUEL_STATE_STARTED)
    {
        duelPlayer->EndDuel(DUEL_WINNER_KNOCKOUT);
        return;
    }

    SendPacket(SmsgDuelComplete(1).serialise().get());
    duelPlayer->SendPacket(SmsgDuelComplete(1).serialise().get());

    GetPlayer()->cancelDuel();
}
