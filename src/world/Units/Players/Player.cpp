/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Player.h"
#include "Server/Packets/Opcode.h"


//////////////////////////////////////////////////////////////////////////////////////////
// Movement

#if VERSION_STRING != Cata
void Player::sendForceMovePaket(UnitSpeedType speed_type, float speed)
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
#else
void Player::sendForceMovePaket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data(60);
    switch (speed_type)
    {
        case TYPE_WALK:
        {
            data.Initialize(SMSG_FORCE_WALK_SPEED_CHANGE);
            movement_info.Write(data, SMSG_FORCE_WALK_SPEED_CHANGE, speed);
        } break;
        case TYPE_RUN:
        {
            data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE);
            movement_info.Write(data, SMSG_FORCE_RUN_SPEED_CHANGE, speed);
        } break;
        case TYPE_RUN_BACK:
        {
            data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE);
            movement_info.Write(data, SMSG_FORCE_RUN_BACK_SPEED_CHANGE, speed);
        } break;
        case TYPE_SWIM:
        {
            data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE);
            movement_info.Write(data, SMSG_FORCE_SWIM_SPEED_CHANGE, speed);
        } break;
        case TYPE_SWIM_BACK:
        {
            data.Initialize(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE);
            movement_info.Write(data, SMSG_FORCE_SWIM_BACK_SPEED_CHANGE, speed);
        } break;
        case TYPE_TURN_RATE:
        {
            data.Initialize(SMSG_FORCE_TURN_RATE_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_TURN_RATE_CHANGE, speed);
        } break;
        case TYPE_FLY:
        {
            data.Initialize(SMSG_FORCE_FLIGHT_SPEED_CHANGE);
            movement_info.Write(data, SMSG_FORCE_FLIGHT_SPEED_CHANGE, speed);
        } break;
        case TYPE_FLY_BACK:
        {
            data.Initialize(SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE, speed);
        } break;
        case TYPE_PITCH_RATE:
        {
            data.Initialize(SMSG_FORCE_PITCH_RATE_CHANGE);
            //movement_info.Write(data, SMSG_FORCE_PITCH_RATE_CHANGE, speed);
        } break;
    }

    SendMessageToSet(&data, true);
}
#endif

#if VERSION_STRING != Cata
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
#else
void Player::sendMoveSetSpeedPaket(UnitSpeedType speed_type, float speed)
{
    WorldPacket data;
    ObjectGuid guid = GetGUID();

    switch (speed_type)
    {
        case TYPE_WALK:
        {
            data.Initialize(MSG_MOVE_SET_WALK_SPEED, 1 + 8 + 4 + 4);
            data.WriteByteMask(guid[0]);
            data.WriteByteMask(guid[4]);
            data.WriteByteMask(guid[5]);
            data.WriteByteMask(guid[2]);
            data.WriteByteMask(guid[3]);
            data.WriteByteMask(guid[1]);
            data.WriteByteMask(guid[6]);
            data.WriteByteMask(guid[7]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[5]);
            data << float(speed);
            data.WriteByteSeq(guid[2]);
            data << uint32(0);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[3]);
        } break;
        case TYPE_RUN:
        {
            data.Initialize(MSG_MOVE_SET_RUN_SPEED, 1 + 8 + 4 + 4);
            data.WriteByteMask(guid[6]);
            data.WriteByteMask(guid[1]);
            data.WriteByteMask(guid[5]);
            data.WriteByteMask(guid[2]);
            data.WriteByteMask(guid[7]);
            data.WriteByteMask(guid[0]);
            data.WriteByteMask(guid[3]);
            data.WriteByteMask(guid[4]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[4]);
            data << uint32(0);
            data << float(speed);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[2]);
        } break;
        case TYPE_RUN_BACK:
        { 
            data.Initialize(MSG_MOVE_SET_RUN_BACK_SPEED, 1 + 8 + 4 + 4);
            data.WriteByteMask(guid[0]);
            data.WriteByteMask(guid[6]);
            data.WriteByteMask(guid[2]);
            data.WriteByteMask(guid[1]);
            data.WriteByteMask(guid[3]);
            data.WriteByteMask(guid[5]);
            data.WriteByteMask(guid[4]);
            data.WriteByteMask(guid[7]);
            data.WriteByteSeq(guid[5]);
            data << uint32(0);
            data << float(speed);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[6]);
        } break;
        case TYPE_SWIM:
        { 
            data.Initialize(MSG_MOVE_SET_SWIM_SPEED, 1 + 8 + 4 + 4);
            data.WriteByteMask(guid[5]);
            data.WriteByteMask(guid[4]);
            data.WriteByteMask(guid[7]);
            data.WriteByteMask(guid[3]);
            data.WriteByteMask(guid[2]);
            data.WriteByteMask(guid[0]);
            data.WriteByteMask(guid[1]);
            data.WriteByteMask(guid[6]);
            data.WriteByteSeq(guid[0]);
            data << uint32(0);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[2]);
            data << float(speed);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[4]);
        } break;
        case TYPE_SWIM_BACK:
        {
            data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED, 1 + 8 + 4 + 4);
            data.WriteByteMask(guid[4]);
            data.WriteByteMask(guid[2]);
            data.WriteByteMask(guid[3]);
            data.WriteByteMask(guid[6]);
            data.WriteByteMask(guid[5]);
            data.WriteByteMask(guid[1]);
            data.WriteByteMask(guid[0]);
            data.WriteByteMask(guid[7]);
            data << uint32(0);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[1]);
            data << float(speed);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[7]);
        } break;
        case TYPE_TURN_RATE:
        { 
            data.Initialize(MSG_MOVE_SET_TURN_RATE, 1 + 8 + 4 + 4);
            data.WriteByteMask(guid[7]);
            data.WriteByteMask(guid[2]);
            data.WriteByteMask(guid[1]);
            data.WriteByteMask(guid[0]);
            data.WriteByteMask(guid[4]);
            data.WriteByteMask(guid[5]);
            data.WriteByteMask(guid[6]);
            data.WriteByteMask(guid[3]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[2]);
            data << float(speed);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[0]);
            data << uint32(0);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[4]);
        } break;
        case TYPE_FLY:
        {
            data.Initialize(MSG_MOVE_SET_FLIGHT_SPEED, 1 + 8 + 4 + 4);
            data.WriteByteMask(guid[0]);
            data.WriteByteMask(guid[5]);
            data.WriteByteMask(guid[1]);
            data.WriteByteMask(guid[6]);
            data.WriteByteMask(guid[3]);
            data.WriteByteMask(guid[2]);
            data.WriteByteMask(guid[7]);
            data.WriteByteMask(guid[4]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[5]);
            data << float(speed);
            data << uint32(0);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[4]);
        } break;
        case TYPE_FLY_BACK:
        { 
            data.Initialize(MSG_MOVE_SET_FLIGHT_BACK_SPEED, 1 + 8 + 4 + 4);
            data.WriteByteMask(guid[1]);
            data.WriteByteMask(guid[2]);
            data.WriteByteMask(guid[6]);
            data.WriteByteMask(guid[4]);
            data.WriteByteMask(guid[7]);
            data.WriteByteMask(guid[3]);
            data.WriteByteMask(guid[0]);
            data.WriteByteMask(guid[4]);
            data.WriteByteSeq(guid[3]);
            data << uint32(0);
            data.WriteByteSeq(guid[6]);
            data << float(speed);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[0]);
            data.WriteByteSeq(guid[5]);
            data.WriteByteSeq(guid[7]);
        } break;
        case TYPE_PITCH_RATE:
        {
            data.Initialize(MSG_MOVE_SET_PITCH_RATE, 1 + 8 + 4 + 4);
            data.WriteByteMask(guid[1]);
            data.WriteByteMask(guid[2]);
            data.WriteByteMask(guid[6]);
            data.WriteByteMask(guid[7]);
            data.WriteByteMask(guid[0]);
            data.WriteByteMask(guid[3]);
            data.WriteByteMask(guid[5]);
            data.WriteByteMask(guid[4]);
            data << float(speed);
            data.WriteByteSeq(guid[6]);
            data.WriteByteSeq(guid[4]);
            data.WriteByteSeq(guid[0]);
            data << uint32(0);
            data.WriteByteSeq(guid[1]);
            data.WriteByteSeq(guid[2]);
            data.WriteByteSeq(guid[7]);
            data.WriteByteSeq(guid[3]);
            data.WriteByteSeq(guid[5]);
        } break;
    }

    SendMessageToSet(&data, false);
}
#endif

void Player::handleFall(MovementInfo const& movement_info)
{
#if VERSION_STRING == Cata
    if (!z_axisposition)
        z_axisposition = movement_info.position.z;

    uint32 falldistance = float2int32(z_axisposition - movement_info.position.z);
    if (z_axisposition <= movement_info.position.z)
        falldistance = 1;

    if ((int)falldistance > m_safeFall)
        falldistance -= m_safeFall;
    else
        falldistance = 1;


    if (isAlive() && !bInvincible && (falldistance > 12) && !m_noFallDamage && ((!GodModeCheat && (UNIXTIME >= m_fallDisabledUntil))))
    {
        uint32_t health_loss = static_cast<uint32_t>(GetHealth() * (falldistance - 12) * 0.017f);

        if (health_loss >= GetHealth())
            health_loss = GetHealth();
        else if ((falldistance >= 65))
        {
            GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING, falldistance, Player::GetDrunkenstateByValue(GetDrunkValue()), 0);
        }

        SendEnvironmentalDamageLog(GetGUID(), DAMAGE_FALL, health_loss);
        DealDamage(this, health_loss, 0, 0, 0);
    }
    z_axisposition = 0.0f;
#endif
}

bool Player::isPlayerJumping(MovementInfo const& movement_info, uint16_t opcode)
{
#if VERSION_STRING == Cata
    if (opcode == MSG_MOVE_FALL_LAND || movement_info.HasMovementFlag(MOVEFLAG_SWIMMING))
    {
        jumping = false;
        return false;
    }

    if (!jumping && (opcode == MSG_MOVE_JUMP || movement_info.HasMovementFlag(MOVEFLAG_FALLING)))
    {
        jumping = true;
        return true;
    }
#endif
    return false;
}

void Player::handleBreathing(MovementInfo& movement_info, WorldSession* session)
{
#if VERSION_STRING == Cata
    if (!sWorld.BreathingEnabled || FlyCheat || m_bUnlimitedBreath || !isAlive() || GodModeCheat)
    {
        if (m_UnderwaterState & UNDERWATERSTATE_SWIMMING)
            m_UnderwaterState &= ~UNDERWATERSTATE_SWIMMING;

        if (m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
        {
            m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;

            SendMirrorTimer(MIRROR_TYPE_BREATH, m_UnderwaterTime, m_UnderwaterMaxTime, -1);
        }

        if (session->m_bIsWLevelSet)
        {
            if ((movement_info.position.z + m_noseLevel) > session->m_wLevel)
            {
                RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_LEAVE_WATER);

                session->m_bIsWLevelSet = false;
            }
        }

        return;
    }

    if (movement_info.HasMovementFlag(MOVEFLAG_SWIMMING) && !(m_UnderwaterState & UNDERWATERSTATE_SWIMMING))
    {
        RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_ENTER_WATER);

        if (!session->m_bIsWLevelSet)
        {
            session->m_wLevel = movement_info.position.z + m_noseLevel * 0.95f;
            session->m_bIsWLevelSet = true;
        }

        m_UnderwaterState |= UNDERWATERSTATE_SWIMMING;
    }

    if (!(movement_info.HasMovementFlag(MOVEFLAG_SWIMMING)) && (movement_info.HasMovementFlag(MOVEFLAG_NONE)) && (m_UnderwaterState & UNDERWATERSTATE_SWIMMING))
    {
        if ((movement_info.position.z + m_noseLevel) > session->m_wLevel)
        {
            RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_LEAVE_WATER);

            session->m_bIsWLevelSet = false;

            m_UnderwaterState &= ~UNDERWATERSTATE_SWIMMING;
        }
    }

    if (m_UnderwaterState & UNDERWATERSTATE_SWIMMING && !(m_UnderwaterState & UNDERWATERSTATE_UNDERWATER))
    {
        if ((movement_info.position.z + m_noseLevel) < session->m_wLevel)
        {
            m_UnderwaterState |= UNDERWATERSTATE_UNDERWATER;
            SendMirrorTimer(MIRROR_TYPE_BREATH, m_UnderwaterTime, m_UnderwaterMaxTime, -1);
        }
    }

    if (m_UnderwaterState & UNDERWATERSTATE_SWIMMING && m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        if ((movement_info.position.z + m_noseLevel) > session->m_wLevel)
        {
            m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            SendMirrorTimer(MIRROR_TYPE_BREATH, m_UnderwaterTime, m_UnderwaterMaxTime, 10);
        }
    }

    if (!(m_UnderwaterState & UNDERWATERSTATE_SWIMMING) && m_UnderwaterState & UNDERWATERSTATE_UNDERWATER)
    {
        if ((movement_info.position.z + m_noseLevel) > session->m_wLevel)
        {
            m_UnderwaterState &= ~UNDERWATERSTATE_UNDERWATER;
            SendMirrorTimer(MIRROR_TYPE_BREATH, m_UnderwaterTime, m_UnderwaterMaxTime, 10);
        }
    }
#endif
}

bool Player::isSpellFitByClassAndRace(uint32_t spell_id)
{
    uint32_t racemask = getRaceMask();
    uint32_t classmask = getClassMask();

    SkillLineAbilityMapBounds bounds = objmgr.GetSkillLineAbilityMapBounds(spell_id);
    if (bounds.first == bounds.second)
        return true;

    for (SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        // skip wrong race skills
        if (_spell_idx->second->race_mask && (_spell_idx->second->race_mask & racemask) == 0)
            continue;

        // skip wrong class skills
        if (_spell_idx->second->class_mask && (_spell_idx->second->class_mask & classmask) == 0)
            continue;

        return true;
    }

    return false;
}
