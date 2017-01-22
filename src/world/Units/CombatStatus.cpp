// License: MIT
#include "CombatStatus.h"
#include "Errors.h"
#include "Unit.h"

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

    void CombatStatus::removeMyHealers()
    {
        auto map_mgr = m_unit->GetMapMgr();
        ASSERT(map_mgr != nullptr);

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
        m_primaryAttackTarget = 0;
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

    void CombatStatus::removeAttacker(Unit* attacker)
    {
        ASSERT(m_unit->IsInWorld());
        ASSERT(attacker != nullptr);

        m_attackers.erase(attacker->GetGUID());
        update();
    }

    void CombatStatus::removeAttackTarget(Unit* attackTarget)
    {
        ASSERT(attackTarget);

        m_attackTargets.erase(attackTarget->GetGUID());
        update();
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
