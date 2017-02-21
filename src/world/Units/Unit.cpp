// License: MIT

#include "Unit.h"
#include "Server/Packets/Opcodes.h"
#include "Server/WorldSession.h"
#include "Players/Player.h"
#include "Spell/SpellAuras.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Combat

void Unit::setCombatFlag(bool enabled)
{
    if (enabled)
    {
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT);
    }
    else
    {
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT);
    }
}

bool Unit::isInCombat() const
{
    return m_combatStatus.isInCombat();
}

bool Unit::isAttacking(Unit* target) const
{
    ASSERT(target);

    return m_combatStatus.isAttacking(target);
}

void Unit::enterCombat()
{
    setCombatFlag(true);

    if (!HasUnitStateFlag(UNIT_STATE_ATTACKING))
    {
        AddUnitStateFlag(UNIT_STATE_ATTACKING);
    }
}

void Unit::leaveCombat()
{
    setCombatFlag(false);

    if (HasUnitStateFlag(UNIT_STATE_ATTACKING))
    {
        RemoveUnitStateFlag(UNIT_STATE_ATTACKING);
    }

    if (IsPlayer())
    {
        static_cast<Player*>(this)->UpdatePotionCooldown();
    }
}

void Unit::onDamageDealt(Unit* target)
{
    ASSERT(target);

    m_combatStatus.onDamageDealt(target);
}

void Unit::addHealTarget(Unit* target)
{
    ASSERT(target != nullptr);

    if (target->IsPlayer())
    {
        m_combatStatus.addHealTarget(reinterpret_cast<Player*>(target));
    }
}

void Unit::removeHealTarget(Unit* target)
{
    ASSERT(target != nullptr);

    if (target->IsPlayer())
    {
        m_combatStatus.removeHealTarget(reinterpret_cast<Player*>(target));
    }
}

void Unit::addHealer(Unit* healer)
{
    ASSERT(healer != nullptr);

    if (healer->IsPlayer())
    {
        m_combatStatus.addHealer(reinterpret_cast<Player*>(healer));
    }
}

void Unit::removeHealer(Unit* healer)
{
    ASSERT(healer != nullptr);

    if (healer->IsPlayer())
    {
        m_combatStatus.removeHealer(reinterpret_cast<Player*>(healer));
    }
}

void Unit::addAttacker(Unit* attacker)
{
    ASSERT(attacker);

    m_combatStatus.addAttacker(attacker);
}

bool Unit::hasAttacker(uint64_t guid) const
{
    return m_combatStatus.hasAttacker(guid);
}

void Unit::removeAttacker(Unit* attacker)
{
    ASSERT(attacker != nullptr);
    //ASSERT(IsInWorld());    //Zyres: unit is not in world. remove attack target only for units in world
    if (this->IsInWorld())
    {
        m_combatStatus.removeAttacker(attacker);
    }
}

void Unit::removeAttacker(uint64_t guid)
{
    m_combatStatus.removeAttacker(guid);
}

void Unit::removeAttackTarget(Unit* attackTarget)
{
    ASSERT(attackTarget != nullptr);
    //ASSERT(IsInWorld());    //Zyres: unit is not in world. remove attack target only for units in world
    if (this->IsInWorld())
    {
        m_combatStatus.removeAttackTarget(attackTarget);
    }
}

void Unit::updateCombatStatus()
{
    m_combatStatus.update();
}

void Unit::clearAllCombatTargets()
{
    m_combatStatus.clearAllCombatTargets();
}

uint64_t Unit::getPrimaryAttackTarget() const
{
    return m_combatStatus.getPrimaryAttackTarget();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Movement

void Unit::SetMoveWaterWalk()
{
    AddUnitMovementFlag(MOVEFLAG_WATER_WALK);

    if (IsPlayer())
    {
        WorldPacket data(SMSG_MOVE_WATER_WALK, 12);
        data << GetNewGUID();
        data << uint32(0);
        SendMessageToSet(&data, true);
    }

    if (IsCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_WATER_WALK, 9);
        data << GetNewGUID();
        SendMessageToSet(&data, false);
    }
}

void Unit::SetMoveLandWalk()
{
    RemoveUnitMovementFlag(MOVEFLAG_WATER_WALK);

    if (IsPlayer())
    {
        WorldPacket data(SMSG_MOVE_LAND_WALK, 12);
        data << GetNewGUID();
        data << uint32(0);
        SendMessageToSet(&data, true);
    }

    if (IsCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_LAND_WALK, 9);
        data << GetNewGUID();
        SendMessageToSet(&data, false);
    }
}

void Unit::SetMoveFeatherFall()
{
    AddUnitMovementFlag(MOVEFLAG_FEATHER_FALL);

    if (IsPlayer())
    {
        WorldPacket data(SMSG_MOVE_FEATHER_FALL, 12);
        data << GetNewGUID();
        data << uint32(0);
        SendMessageToSet(&data, true);
    }

    if (IsCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_FEATHER_FALL, 9);
        data << GetNewGUID();
        SendMessageToSet(&data, false);
    }
}

void Unit::SetMoveNormalFall()
{
    RemoveUnitMovementFlag(MOVEFLAG_FEATHER_FALL);

    if (IsPlayer())
    {
        WorldPacket data(SMSG_MOVE_NORMAL_FALL, 12);
        data << GetNewGUID();
        data << uint32(0);
        SendMessageToSet(&data, true);
    }

    if (IsCreature())
    {
        WorldPacket data(SMSG_SPLINE_MOVE_NORMAL_FALL, 9);
        data << GetNewGUID();
        SendMessageToSet(&data, false);
    }
}

void Unit::SetMoveHover(bool set_hover)
{
    if (IsPlayer())
    {
        if (set_hover)
        {
            AddUnitMovementFlag(MOVEFLAG_HOVER);

            WorldPacket data(SMSG_MOVE_SET_HOVER, 13);
            data << GetNewGUID();
            data << uint32(0);
            SendMessageToSet(&data, true);
        }
        else
        {
            RemoveUnitMovementFlag(MOVEFLAG_HOVER);

            WorldPacket data(SMSG_MOVE_UNSET_HOVER, 13);
            data << GetNewGUID();
            data << uint32(0);
            SendMessageToSet(&data, true);
        }
    }

    //\todo spline update
    if (IsCreature())
    {
        if (set_hover)
        {
            AddUnitMovementFlag(MOVEFLAG_HOVER);

            SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);

            WorldPacket data(SMSG_SPLINE_MOVE_SET_HOVER, 10);
            data << GetNewGUID();
            SendMessageToSet(&data, false);
        }
        else
        {
            RemoveUnitMovementFlag(MOVEFLAG_HOVER);

            RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);

            WorldPacket data(SMSG_SPLINE_MOVE_UNSET_HOVER, 10);
            data << GetNewGUID();
            SendMessageToSet(&data, false);
        }
    }
}

void Unit::SetMoveCanFly(bool set_fly)
{
    if (IsPlayer())
    {
        if (set_fly)
        {
            AddUnitMovementFlag(MOVEFLAG_CAN_FLY);

            // Remove falling flag if set
            RemoveUnitMovementFlag(MOVEFLAG_FALLING);

            WorldPacket data(SMSG_MOVE_SET_CAN_FLY, 13);
            data << GetNewGUID();
            data << uint32(0);
            SendMessageToSet(&data, true);
        }
        else
        {
            // Remove all fly related moveflags
            RemoveUnitMovementFlag(MOVEFLAG_CAN_FLY);
            RemoveUnitMovementFlag(MOVEFLAG_DESCENDING);
            RemoveUnitMovementFlag(MOVEFLAG_ASCENDING);

            WorldPacket data(SMSG_MOVE_UNSET_CAN_FLY, 13);
            data << GetNewGUID();
            data << uint32(0);
            SendMessageToSet(&data, true);
        }
    }

    if (IsCreature())
    {
        if (set_fly)
        {
            AddUnitMovementFlag(MOVEFLAG_CAN_FLY);

            // Remove falling flag if set
            RemoveUnitMovementFlag(MOVEFLAG_FALLING);

            WorldPacket data(SMSG_SPLINE_MOVE_SET_FLYING, 10);
            data << GetNewGUID();
            SendMessageToSet(&data, false);
        }
        else
        {
            // Remove all fly related moveflags
            RemoveUnitMovementFlag(MOVEFLAG_CAN_FLY);
            RemoveUnitMovementFlag(MOVEFLAG_DESCENDING);
            RemoveUnitMovementFlag(MOVEFLAG_ASCENDING);

            WorldPacket data(SMSG_SPLINE_MOVE_UNSET_FLYING, 10);
            data << GetNewGUID();
            SendMessageToSet(&data, false);
        }
    }
}

void Unit::SetMoveRoot(bool set_root)
{
    // AIInterface
    //\todo stop movement based on movement flag instead of m_canMove
    if (!IsPlayer())
    {
        if (set_root)
        {
            m_aiInterface->m_canMove = false;
            m_aiInterface->StopMovement(100);
        }
        else
        {
            m_aiInterface->m_canMove = true;
        }
    }

    if (set_root)
    {
        AddUnitMovementFlag(MOVEFLAG_ROOTED);

        WorldPacket data(SMSG_FORCE_MOVE_ROOT, 12);
        data << GetNewGUID();
        data << uint32(0);
        SendMessageToSet(&data, true);
    }
    else
    {
        RemoveUnitMovementFlag(MOVEFLAG_ROOTED);

        WorldPacket data(SMSG_FORCE_MOVE_UNROOT, 12);
        data << GetNewGUID();
        data << uint32(0);
        SendMessageToSet(&data, true);
    }
}

bool Unit::IsRooted() const
{
    return HasUnitMovementFlag(MOVEFLAG_ROOTED);
}

void Unit::SetMoveSwim(bool set_swim)
{
    if (IsCreature())
    {
        if (set_swim)
        {
            AddUnitMovementFlag(MOVEFLAG_SWIMMING);

            WorldPacket data(SMSG_SPLINE_MOVE_START_SWIM, 10);
            data << GetNewGUID();
            SendMessageToSet(&data, false);
        }
        else
        {
            RemoveUnitMovementFlag(MOVEFLAG_SWIMMING);

            WorldPacket data(SMSG_SPLINE_MOVE_STOP_SWIM, 10);
            data << GetNewGUID();
            SendMessageToSet(&data, false);
        }
    }
}

void Unit::SetMoveDisableGravity(bool disable_gravity)
{
    if (IsPlayer())
    {
        if (disable_gravity)
        {
            AddUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);

            WorldPacket data(SMSG_MOVE_GRAVITY_DISABLE, 13);
            data << GetNewGUID();
            data << uint32(0);
            SendMessageToSet(&data, true);
        }
        else
        {
            RemoveUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);

            WorldPacket data(SMSG_MOVE_GRAVITY_ENABLE, 13);
            data << GetNewGUID();
            data << uint32(0);
            SendMessageToSet(&data, true);
        }
    }

    if (IsCreature())
    {
        if (disable_gravity)
        {
            AddUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);

            WorldPacket data(SMSG_SPLINE_MOVE_GRAVITY_DISABLE, 10);
            data << GetNewGUID();
            SendMessageToSet(&data, false);
        }
        else
        {
            RemoveUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY);

            WorldPacket data(SMSG_SPLINE_MOVE_GRAVITY_ENABLE, 10);
            data << GetNewGUID();
            SendMessageToSet(&data, false);
        }
    }
}

//\todo Zyres: call it if creature has MoveFlag in its movement info (set in Object::_BuildMovementUpdate)
//             Unfortunately Movement and object update is a mess.
void Unit::SetMoveWalk(bool set_walk)
{
    if (IsCreature())
    {
        if (set_walk)
        {
            AddUnitMovementFlag(MOVEFLAG_WALK);

            WorldPacket data(SMSG_SPLINE_MOVE_SET_WALK_MODE, 10);
            data << GetNewGUID();
            SendMessageToSet(&data, false);
        }
        else
        {
            RemoveUnitMovementFlag(MOVEFLAG_WALK);

            WorldPacket data(SMSG_SPLINE_MOVE_SET_RUN_MODE, 10);
            data << GetNewGUID();
            SendMessageToSet(&data, false);
        }
    }
}

float Unit::getSpeedForType(UnitSpeedType speed_type, bool get_basic)
{
    switch (speed_type)
    {
        case TYPE_WALK:
            return (get_basic ? m_basicSpeedWalk : m_currentSpeedWalk);
        case TYPE_RUN:
            return (get_basic ? m_basicSpeedRun : m_currentSpeedRun);
        case TYPE_RUN_BACK:
            return (get_basic ? m_basicSpeedRunBack : m_currentSpeedRunBack);
        case TYPE_SWIM:
            return (get_basic ? m_basicSpeedSwim : m_currentSpeedSwim);
        case TYPE_SWIM_BACK:
            return (get_basic ? m_basicSpeedSwimBack : m_currentSpeedSwimBack);
        case TYPE_TURN_RATE:
            return (get_basic ? m_basicTurnRate : m_currentTurnRate);
        case TYPE_FLY:
            return (get_basic ? m_basicSpeedFly : m_currentSpeedFly);
        case TYPE_FLY_BACK:
            return (get_basic ? m_basicSpeedFlyBack : m_currentSpeedFlyBack);
        case TYPE_PITCH_RATE:
            return (get_basic ? m_basicPitchRate : m_currentPitchRate);
    }
}

void Unit::setSpeedForType(UnitSpeedType speed_type, float speed, bool set_basic)
{
    switch (speed_type)
    {
        case TYPE_WALK:
        {
            if (set_basic)
                m_basicSpeedWalk = speed;
            else
                m_currentSpeedWalk = speed;
        } break;
        case TYPE_RUN:
        {
            if (set_basic)
                m_basicSpeedRun = speed;
            else
                m_currentSpeedRun = speed;
        } break;
        case TYPE_RUN_BACK:
        {
            if (set_basic)
                m_basicSpeedRunBack = speed; 
            else
                m_currentSpeedRunBack = speed;
        } break;
        case TYPE_SWIM:
        {
            if (set_basic)
                m_basicSpeedSwim = speed;
            else
                m_currentSpeedSwim = speed;
        } break;
        case TYPE_SWIM_BACK:
        {
            if (set_basic)
                m_basicSpeedSwimBack = speed; 
            else
                m_currentSpeedSwimBack = speed;
        } break;
        case TYPE_TURN_RATE:
        {
            if (set_basic)
                m_basicTurnRate = speed;
            else
                m_currentTurnRate = speed;
        } break;
        case TYPE_FLY:
        {
            if (set_basic)
                 m_basicSpeedFly = speed;
            else
                m_currentSpeedFly = speed;
        } break;
        case TYPE_FLY_BACK:
        {
            if (set_basic)
                m_basicSpeedFlyBack = speed;
            else
                m_currentSpeedFlyBack = speed;
        } break;
        case TYPE_PITCH_RATE:
        {
            if (set_basic)
                m_basicPitchRate = speed;
            else
                m_currentPitchRate = speed;
        } break;
    }
}

void Unit::resetCurrentSpeed()
{
    m_currentSpeedWalk = m_basicSpeedWalk;
    m_currentSpeedRun = m_basicSpeedRun;
    m_currentSpeedRunBack = m_basicSpeedRunBack;
    m_currentSpeedSwim = m_basicSpeedSwim;
    m_currentSpeedSwimBack = m_basicSpeedSwimBack;
    m_currentTurnRate = m_basicTurnRate;
    m_currentSpeedFly = m_basicSpeedFly;
    m_currentSpeedFlyBack = m_basicSpeedFlyBack;
    m_currentPitchRate = m_basicPitchRate;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Spells

void Unit::PlaySpellVisual(uint64_t guid, uint32_t spell_id)
{
    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 12);
    data << uint64_t(guid);
    data << uint32_t(spell_id);

    if (IsPlayer())
        static_cast<Player*>(this)->SendMessageToSet(&data, true);
    else
        SendMessageToSet(&data, false);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Aura

Aura* Unit::GetAuraWithId(uint32_t spell_id)
{
    for (uint32_t i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        Aura* aura = m_auras[i];
        if (aura != nullptr)
        {
            if (aura->GetSpellId() == spell_id)
                return aura;
        }
    }

    return nullptr;
}

Aura* Unit::GetAuraWithIdForGuid(uint32_t spell_id, uint64_t target_guid)
{
    for (uint32_t i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        Aura* aura = m_auras[i];
        if (aura != nullptr)
        {
            if (GetAuraWithId(spell_id) && aura->m_casterGuid == target_guid)
                return aura;
        }
    }

    return nullptr;
}

Aura* Unit::GetAuraWithAuraEffect(uint32_t aura_effect)
{
    for (uint32_t i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        Aura* aura = m_auras[i];
        if (aura != nullptr && aura->GetSpellInfo()->HasEffectApplyAuraName(aura_effect))
            return aura;
    }

    return nullptr;
}
