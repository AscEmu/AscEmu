/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Player.h"
#include "Server/Packets/Opcode.h"
#include "Chat/ChatDefines.hpp"
#include "Server/World.h"


//////////////////////////////////////////////////////////////////////////////////////////
// Movement

#if VERSION_STRING != Cata
void Player::sendForceMovePacket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data(50);

    switch (speed_type)
    {
        case TYPE_WALK:
            data.Initialize(SMSG_FORCE_WALK_SPEED_CHANGE);
            break;
        case TYPE_RUN:
            data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
            break;
        case TYPE_RUN_BACK:
            data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE);
            break;
        case TYPE_SWIM:
            data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE);
            break;
        case TYPE_SWIM_BACK:
            data.Initialize(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE);
            break;
        case TYPE_TURN_RATE:
            data.Initialize(SMSG_FORCE_TURN_RATE_CHANGE);
            break;
        case TYPE_FLY:
            data.Initialize(SMSG_FORCE_FLIGHT_SPEED_CHANGE);
            break;
        case TYPE_FLY_BACK:
            data.Initialize(SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE);
            break;
#if VERSION_STRING > TBC
        case TYPE_PITCH_RATE:
            data.Initialize(SMSG_FORCE_PITCH_RATE_CHANGE);
            break;
#endif
    }

    data << GetNewGUID();
    data << m_speedChangeCounter++;

    if (speed_type == TYPE_RUN)
        data << uint8_t(1);

    data << float(speed);

    SendMessageToSet(&data, true);
}

void Player::sendMoveSetSpeedPaket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data(45);

    switch (speed_type)
    {
        case TYPE_WALK:
            data.Initialize(MSG_MOVE_SET_WALK_SPEED);
            break;
        case TYPE_RUN:
            data.Initialize(MSG_MOVE_SET_RUN_SPEED);
            break;
        case TYPE_RUN_BACK:
            data.Initialize(MSG_MOVE_SET_RUN_BACK_SPEED);
            break;
        case TYPE_SWIM:
            data.Initialize(MSG_MOVE_SET_SWIM_SPEED);
            break;
        case TYPE_SWIM_BACK:
            data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED);
            break;
        case TYPE_TURN_RATE:
            data.Initialize(MSG_MOVE_SET_TURN_RATE);
            break;
        case TYPE_FLY:
            data.Initialize(MSG_MOVE_SET_FLIGHT_SPEED);
            break;
        case TYPE_FLY_BACK:
            data.Initialize(MSG_MOVE_SET_FLIGHT_BACK_SPEED);
            break;
#if VERSION_STRING > TBC
        case TYPE_PITCH_RATE:
            data.Initialize(MSG_MOVE_SET_PITCH_RATE);
            break;
#endif
    }

    data << GetNewGUID();
    BuildMovementPacket(&data);
    data << float(speed);

    SendMessageToSet(&data, false);
}

void Player::handleFall(MovementInfo const& /*movement_info*/)
{
}

bool Player::isPlayerJumping(MovementInfo const& /*movement_info*/, uint16_t /*opcode*/)
{
    return false;
}

void Player::handleBreathing(MovementInfo const& /*movement_info*/, WorldSession* /*session*/)
{
}

bool Player::isSpellFitByClassAndRace(uint32_t /*spell_id*/)
{
    return false;
}

void Player::sendAuctionCommandResult(Auction* /*auction*/, uint32_t /*action*/, uint32_t /*errorCode*/, uint32_t /*bidError*/)
{
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Spells
void Player::updateAutoRepeatSpell()
{
    // Get the autorepeat spell
    Spell* autoRepeatSpell = getCurrentSpell(CURRENT_AUTOREPEAT_SPELL);

    // Check is target valid
    // Note for self: remove this check when new Spell::canCast() is ready -Appled
    Unit* target = GetMapMgr()->GetUnit(autoRepeatSpell->m_targets.m_unitTarget);
    if (target == nullptr)
    {
        m_AutoShotAttackTimer = 0;
        interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
        return;
    }

    // If player is moving or casting a spell, interrupt wand casting and delay auto shot
    const bool isAutoShot = autoRepeatSpell->GetSpellInfo()->getId() == 75;
    if (m_isMoving || isCastingNonMeleeSpell(true, false, true, isAutoShot))
    {
        if (!isAutoShot)
        {
            interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
        }
        m_FirstCastAutoRepeat = true;
        return;
    }

    // Apply delay to wand shooting
    if (m_FirstCastAutoRepeat && m_AutoShotAttackTimer < 500 && !isAutoShot)
    {
        m_AutoShotAttackTimer = 500;
    }
    m_FirstCastAutoRepeat = false;

    if (m_AutoShotAttackTimer == 0)
    {
        // TODO: implement ::CanShootRangedWeapon() into new Spell::canCast()
        // also currently if target gets too far away, your autorepeat spell will get interrupted
        // it's related most likely to ::CanShootRangedWeapon()
        const int32_t canCastAutoRepeatSpell = CanShootRangedWeapon(autoRepeatSpell->GetSpellInfo()->getId(), target, isAutoShot);
        if (canCastAutoRepeatSpell != SPELL_CANCAST_OK)
        {
            if (!isAutoShot)
            {
                interruptSpellWithSpellType(CURRENT_AUTOREPEAT_SPELL);
            }
            return;
        }

        m_AutoShotAttackTimer = m_uint32Values[UNIT_FIELD_RANGEDATTACKTIME];

        // Cast the spell with triggered flag
        Spell* newAutoRepeatSpell = sSpellFactoryMgr.NewSpell(this, autoRepeatSpell->GetSpellInfo(), true, nullptr);
        newAutoRepeatSpell->prepare(&(autoRepeatSpell->m_targets));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Messages
void Player::sendReportToGmMessage(std::string playerName, std::string damageLog)
{
    std::string gm_ann(MSG_COLOR_GREEN);

    gm_ann += "|HPlayer:";
    gm_ann += playerName;
    gm_ann += "|h[";
    gm_ann += playerName;
    gm_ann += "]|h: ";
    gm_ann += MSG_COLOR_YELLOW;
    gm_ann += damageLog;

    sWorld.sendMessageToOnlineGms(gm_ann.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
bool Player::isGMFlagSet()
{
    return HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);
}

void Player::sendMovie(uint32_t movieId)
{
    WorldPacket data(SMSG_TRIGGER_MOVIE, 4);
    data << uint32_t(movieId);
    m_session->SendPacket(&data);
}
