// License: MIT

#include "Unit.h"
#include "Server/Packets/Opcodes.h"
#include "Players/Player.h"

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

    if (!hasStateFlag(UF_ATTACKING))
    {
        addStateFlag(UF_ATTACKING);
    }
}

void Unit::leaveCombat()
{
    setCombatFlag(false);

    if (hasStateFlag(UF_ATTACKING))
    {
        clearStateFlag(UF_ATTACKING);
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

void Unit::SetWaterWalk()
{
    AddUnitMovementFlag(MOVEFLAG_WATER_WALK);

    WorldPacket data(SMSG_MOVE_WATER_WALK, 12);
    data << GetNewGUID();
    data << uint32(0);
    SendMessageToSet(&data, true);
}

void Unit::SetLandWalk()
{
    RemoveUnitMovementFlag(MOVEFLAG_WATER_WALK);

    WorldPacket data(SMSG_MOVE_LAND_WALK, 12);
    data << GetNewGUID();
    data << uint32(0);
    SendMessageToSet(&data, true);
}

void Unit::SetFeatherFall()
{
    AddUnitMovementFlag(MOVEFLAG_FEATHER_FALL);

    WorldPacket data(SMSG_MOVE_FEATHER_FALL, 12);
    data << GetNewGUID();
    data << uint32(0);
    SendMessageToSet(&data, true);
}

void Unit::SetNormalFall()
{
    RemoveUnitMovementFlag(MOVEFLAG_FEATHER_FALL);

    WorldPacket data(SMSG_MOVE_NORMAL_FALL, 12);
    data << GetNewGUID();
    data << uint32(0);
    SendMessageToSet(&data, true);
}

void Unit::SetHover(bool set_hover)
{
    if (set_hover)
    {
        WorldPacket data(SMSG_MOVE_SET_HOVER, 13);
        data << GetNewGUID();
        data << uint32(0);
        SendMessageToSet(&data, false);
    }
    else
    {
        WorldPacket data(SMSG_MOVE_UNSET_HOVER, 13);
        data << GetNewGUID();
        data << uint32(0);
        SendMessageToSet(&data, false);
    }
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
