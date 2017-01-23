// License: MIT
#include "CombatStatus.h"
#include "Errors.h"
#include "Unit.h"
#include "Map/MapMgr.h"
#include "Spell/SpellAuras.h"

using namespace std;

namespace AscEmu { namespace World { namespace Units {

    bool CombatStatus::isReallyInCombat() const
    {
        if (m_unit->IsPlayer())
        {
            auto map_mgr = m_unit->GetMapMgr();
            if (map_mgr != nullptr && map_mgr->IsCombatInProgress())
            {
                return true;
            }
        }

        return m_healTargets.size() > 0 || m_attackTargets.size() > 0 || m_attackers.size() > 0;
    }

    CombatStatus::CombatStatus(Unit* unit) : m_unit(unit), m_primaryAttackTarget(0), m_lastStatus(false)
    {
    }

    void CombatStatus::update()
    {
        auto currentStatus = isReallyInCombat();
        if (currentStatus != m_lastStatus)
        {
            m_lastStatus = currentStatus;
            if (currentStatus)
            {
                m_unit->enterCombat();
            }
            else
            {
                removeMyHealers();
                m_unit->leaveCombat();
            }
        }
    }

    void CombatStatus::onRemoveFromWorld()
    {
        clearAllCombatTargets();
    }

    void CombatStatus::clearAttackTargets()
    {
        auto map_mgr = m_unit->GetMapMgr();
        ASSERT(map_mgr);

        if (m_unit->IsPlayer())
        {
            reinterpret_cast<Player*>(m_unit)->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_CONT_PVP);
        }

        for (auto targetGuid: m_attackTargets)
        {
            auto target = map_mgr->GetUnit(targetGuid);
            if (target == nullptr)
            {
                m_attackTargets.erase(targetGuid);
                continue;
            }

            removeAttackTarget(target);
            target->removeAttacker(m_unit);
        }
    }

    void CombatStatus::clearAllCombatTargets()
    {
        removeAllAttackersAndAttackTargets();
        removeAllHealersAndHealTargets();
    }

    bool CombatStatus::isInCombat() const
    {
        ASSERT(m_unit != nullptr);
        if (!m_unit->IsInWorld())
        {
            return false;
        }

        if (m_unit->GetTypeId() == TYPEID_UNIT)
        {
            if (m_unit->IsPet())
            {
                Pet* me = reinterpret_cast<Pet*>(m_unit);

                if (me->GetPetAction() == PET_ACTION_ATTACK)
                {
                    return true;
                }

                return m_lastStatus;
            }

            ASSERT(m_unit->GetAIInterface() != nullptr);

            return m_unit->GetAIInterface()->getAITargetsCount() > 0;
        }

        if (m_unit->GetTypeId() == TYPEID_PLAYER)
        {
            Player* me = reinterpret_cast<Player*>(m_unit);

            auto summons = me->GetSummons();
            for (auto summon: summons)
            {
                if (summon->GetPetOwner() == me && summon->isInCombat())
                {
                    return true;
                }
            }

            return m_lastStatus;
        }

        return false;
    }

    bool CombatStatus::isAttacking(Unit* target) const
    {
        ASSERT(target);

        for (uint32_t i = MAX_NEGATIVE_AURAS_EXTEDED_START; i < MAX_NEGATIVE_AURAS_EXTEDED_END; ++i)
        {
            auto aura = target->m_auras[i];
            if (aura != nullptr)
            {
                if (m_unit->GetGUID() == aura->m_casterGuid && aura->IsCombatStateAffecting())
                {
                    return true;
                }
            }
        }

        return false;
    }

    void CombatStatus::removeMyHealers()
    {
        auto map_mgr = m_unit->GetMapMgr();
        ASSERT(map_mgr);

        for (auto healerGuid: m_healers)
        {
            auto healer = map_mgr->GetPlayer(healerGuid);
            if (healer != nullptr)
            {
                healer->removeHealTarget(m_unit);
            }
        }

        m_healers.clear();
    }

    void CombatStatus::removeAllHealersAndHealTargets()
    {
        auto map_mgr = m_unit->GetMapMgr();
        ASSERT(map_mgr != nullptr);

        for (auto healTargetGuid: m_healTargets)
        {
            auto player = map_mgr->GetPlayer(healTargetGuid);
            if (player != nullptr)
            {
                player->removeHealer(m_unit);
            }
        }

        for (auto healerGuid: m_healers)
        {
            auto player = map_mgr->GetPlayer(healerGuid);
            if (player != nullptr)
            {
                player->removeHealTarget(m_unit);
            }
        }

        m_healTargets.clear();
        m_healers.clear();
        update();
    }

    void CombatStatus::removeAllAttackersAndAttackTargets()
    {
        auto map_mgr = m_unit->GetMapMgr();
        ASSERT(map_mgr != nullptr);

        for (auto attackTargetGuid: m_attackTargets)
        {
            auto attackTarget = map_mgr->GetUnit(attackTargetGuid);
            if (attackTarget != nullptr)
            {
                attackTarget->removeAttacker(m_unit);
            }
        }

        for (auto attackerGuid: m_attackers)
        {
            auto attacker = map_mgr->GetUnit(attackerGuid);
            if (attacker != nullptr)
            {
                attacker->removeAttackTarget(m_unit);
            }
        }

        m_attackers.clear();
        m_attackTargets.clear();
        clearPrimaryAttackTarget();
        update();
    }

    void CombatStatus::clearPrimaryAttackTarget()
    {
        auto map_mgr = m_unit->GetMapMgr();
        ASSERT(map_mgr);

        if (m_primaryAttackTarget != 0)
        {
            auto target = map_mgr->GetUnit(m_primaryAttackTarget);
            if (target != nullptr)
            {
                if (!isAttacking(target))
                {
                    target->removeAttacker(m_unit);
                    m_attackTargets.erase(m_primaryAttackTarget);
                }
            }
            else
            {
                m_attackTargets.erase(m_primaryAttackTarget);
            }

            m_primaryAttackTarget = 0;
        }

        update();
    }

    void CombatStatus::addAttackTarget(uint64_t guid)
    {
        ASSERT(m_unit->IsInWorld());

        if (guid == m_unit->GetGUID())
            return;

        m_attackTargets.insert(guid);
        if (m_unit->IsPlayer())
        {
            // Players can only have one primary attack target
            // TODO: This should be true for creatures too...
            if (m_primaryAttackTarget != guid)
            {
                if (m_primaryAttackTarget != 0)
                    clearPrimaryAttackTarget();

                m_primaryAttackTarget = guid;
            }
        }

        update();
    }

    void CombatStatus::removeHealTarget(Player* target)
    {
        ASSERT(target != nullptr);

        m_healTargets.erase(target->GetLowGUID());
        update();
    }

    void CombatStatus::removeHealer(Player* healer)
    {
        ASSERT(healer != nullptr);

        m_healers.erase(healer->GetLowGUID());
        update();
    }

    void CombatStatus::addHealer(Player* healer)
    {
        ASSERT(healer != nullptr);

        if (!m_unit->IsPlayer() || m_unit == healer)
        {
            return;
        }

        m_healers.insert(healer->GetLowGUID());
    }

    void CombatStatus::addAttacker(Unit* attacker)
    {
        ASSERT(attacker);

        auto guid = attacker->GetGUID();

        if (hasAttacker(guid))
            return;

        m_attackers.insert(guid);
        update();
    }

    bool CombatStatus::hasAttacker(uint64_t guid) const
    {
        return find(begin(m_attackers), end(m_attackers), guid) != end(m_attackers);
    }

    void CombatStatus::onDamageDealt(Unit* target)
    {
        ASSERT(target);

        if (target == m_unit)
        {
            return;
        }

        if (!target->isAlive() || !m_unit->isAlive())
        {
            return;
        }

        auto attackTarget = find(begin(m_attackTargets), end(m_attackTargets), target->GetGUID());
        if (attackTarget == end(m_attackTargets))
        {
            addAttackTarget(target->GetGUID());
        }

        if (!target->hasAttacker(m_unit->GetGUID()))
        {
            target->addAttacker(m_unit);
        }

        m_unit->CombatStatusHandler_ResetPvPTimeout();
    }

    void CombatStatus::removeAttacker(Unit* attacker)
    {
        ASSERT(attacker != nullptr);

        if (attacker->isAttacking(m_unit))
        {
            removeAttacker(attacker->GetGUID());
        }
    }

    void CombatStatus::removeAttacker(uint64_t guid)
    {
        ASSERT(m_unit->IsInWorld());
        
        m_attackers.erase(guid);
        update();
    }

    void CombatStatus::removeAttackTarget(Unit* target)
    {
        ASSERT(target);

        if (!isAttacking(target))
        {
            auto guid = target->GetGUID();
            m_attackTargets.erase(guid);
            if (m_primaryAttackTarget == guid)
                m_primaryAttackTarget = 0;

            update();
        }
    }

    uint64_t CombatStatus::getPrimaryAttackTarget() const
    {
        return m_primaryAttackTarget;
    }

    void CombatStatus::addHealTarget(Player* target)
    {
        ASSERT(target != nullptr);

        if (!m_unit->IsPlayer() || m_unit == target)
        {
            return;
        }

        if (target->isInCombat())
        {
            m_healTargets.insert(target->GetLowGUID());
            target->addHealer(m_unit);
        }

        update();
    }
}}}
