// License: MIT

#include "Unit.h"
#include "Server/Packets/Opcodes.h"
#include "Players/Player.h"

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
