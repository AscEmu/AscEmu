/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Packets/SmsgDuelCountdown.h"
#include "Server/Packets/SmsgDuelComplete.h"
#include "Server/WorldSession.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"

using namespace AscEmu::Packets;

void WorldSession::handleDuelAccepted(WorldPacket& /*recvPacket*/)
{
    const auto duelPlayer = _player->m_duelPlayer;
    if (duelPlayer == nullptr)
        return;

    if (_player->m_duelState != DUEL_STATE_FINISHED)
        return;

    if (_player->m_duelCountdownTimer > 0)
        return;

    _player->m_duelStatus = DUEL_STATUS_INBOUNDS;
    _player->m_duelState = DUEL_STATE_STARTED;

    duelPlayer->m_duelStatus = DUEL_STATUS_INBOUNDS;
    duelPlayer->m_duelState = DUEL_STATE_STARTED;

    const uint32_t defaultDuelCountdown = 3000;

    SendPacket(SmsgDuelCountdown(defaultDuelCountdown).serialise().get());
    duelPlayer->sendPacket(SmsgDuelCountdown(defaultDuelCountdown).serialise().get());

    _player->m_duelCountdownTimer = defaultDuelCountdown;

    sEventMgr.AddEvent(_player, &Player::handleDuelCountdown, EVENT_PLAYER_DUEL_COUNTDOWN, 1000, 3, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void WorldSession::handleDuelCancelled(WorldPacket& /*recvPacket*/)
{
    const auto duelPlayer = _player->m_duelPlayer;
    if (duelPlayer == nullptr)
        return;

    if (_player->m_duelState == DUEL_STATE_STARTED)
    {
        duelPlayer->endDuel(DUEL_WINNER_KNOCKOUT);
        return;
    }

    SendPacket(SmsgDuelComplete(1).serialise().get());
    duelPlayer->sendPacket(SmsgDuelComplete(1).serialise().get());

    _player->cancelDuel();
}
