/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Map/MapMgr.h"
#include "Spell/SpellAuras.h"
#include "Server/WorldSession.h"
#include "Objects/GameObject.h"
#include "Units/Creatures/Pet.h"

void WorldSession::HandleDuelAccepted(WorldPacket& recvData)
{
    uint64_t guid;
    recvData >> guid;

    if (_player->DuelingWith == nullptr)
        return;

    if (_player->m_duelState != DUEL_STATE_FINISHED || _player->m_duelCountdownTimer > 0)
        return;

    Player* player_target = _player->DuelingWith;

    _player->m_duelCountdownTimer = 3000;

    _player->m_duelStatus = DUEL_STATUS_INBOUNDS;
    player_target->m_duelStatus = DUEL_STATUS_INBOUNDS;

    _player->m_duelState = DUEL_STATE_STARTED;
    player_target->m_duelState = DUEL_STATE_STARTED;

    SendDuelCountdown();
    player_target->GetSession()->SendDuelCountdown();

    sEventMgr.AddEvent(_player, &Player::DuelCountdown, EVENT_PLAYER_DUEL_COUNTDOWN, 1000, 3, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void WorldSession::SendDuelCountdown(uint32_t time /*3000*/)
{
    WorldPacket data(SMSG_DUEL_COUNTDOWN, 4);
    data << uint32_t(time);
    SendPacket(&data);
}

void WorldSession::SendDuelComplete(uint8_t type)
{
    WorldPacket data(SMSG_DUEL_COMPLETE, 1);
    data << uint8_t(type);                      //bool 1 = true
    SendPacket(&data);
}

void WorldSession::HandleDuelCancelled(WorldPacket& recvData)
{
    uint64_t guid;
    recvData >> guid;

    if (_player->DuelingWith == nullptr)
        return;

    if (_player->m_duelState == DUEL_STATE_STARTED)
    {
        _player->DuelingWith->EndDuel(DUEL_WINNER_KNOCKOUT);
        return;
    }

    Player* player_target = _player->DuelingWith;

    SendDuelComplete(1);
    player_target->GetSession()->SendDuelComplete(1);

    GameObject* arbiter = _player->GetMapMgr() ? _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(_player->GetDuelArbiter())) : nullptr;
    if (arbiter != nullptr)
    {
        arbiter->RemoveFromWorld(true);
        delete arbiter;
    }

    player_target->SetDuelArbiter(0);
    player_target->SetDuelTeam(0);
    player_target->m_duelState = DUEL_STATE_FINISHED;
    player_target->m_duelCountdownTimer = 0;
    player_target->DuelingWith = nullptr;

    _player->SetDuelArbiter(0);
    _player->SetDuelTeam(0);
    _player->m_duelState = DUEL_STATE_FINISHED;
    _player->m_duelCountdownTimer = 0;
    _player->DuelingWith = nullptr;

    for (uint32_t i = MAX_NEGATIVE_AURAS_EXTEDED_START; i < MAX_NEGATIVE_AURAS_EXTEDED_END; ++i)
    {
        if (_player->m_auras[i])
            _player->m_auras[i]->Remove();
    }

    std::list<Pet*> summons = _player->GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        if ((*itr)->isAlive())
        {
            (*itr)->SetPetAction(PET_ACTION_STAY);
            (*itr)->SetPetAction(PET_ACTION_FOLLOW);
        }
    }
}
