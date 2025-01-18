/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CombatHandler.hpp"
#include "Players/Player.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Unit.hpp"
#include "UnitDefines.hpp"
#include "Creatures/AIInterface.h"
#include "Server/Script/HookInterface.hpp"
#include "Utilities/Util.hpp"

CombatHandler::CombatHandler(Unit* owner) : m_owner(owner)
{}

void CombatHandler::onHostileAction(Unit* victim, bool skipInitialDelay/* = false*/)
{
    if (victim == nullptr || !victim->IsInWorld())
        return;

    std::lock_guard<std::mutex> guard(m_mutexCombat);
    _initiateCombatWith(victim);

    if (skipInitialDelay)
    {
        const auto msTime = Util::getMSTime();
        _checkPvpFlags(victim, false);
        _combatAction(victim, msTime, false, true);
        return;
    }

    m_combatBatchAttackingTargets.insert(std::make_pair(victim->getGuid(), BatchTargetInfo(victim, false)));
}

void CombatHandler::onFriendlyAction(Unit* target, bool skipInitialDelay/* = false*/)
{
    if (target == nullptr || !target->IsInWorld())
        return;

    // If target is not in combat, unit should not get in combat either
    if (!target->getCombatHandler().isInCombat())
        return;

    std::lock_guard<std::mutex> guard(m_mutexCombat);
    _initiateCombatWith(target);

    if (skipInitialDelay)
    {
        const auto msTime = Util::getMSTime();
        _checkPvpFlags(target, true);
        _combatAction(target, msTime, true, true);
        return;
    }

    m_combatBatchAttackingTargets.insert(std::make_pair(target->getGuid(), BatchTargetInfo(target, true)));
}

void CombatHandler::takeCombatAction(Unit* initiater, bool friendlyCombat/* = false*/, bool skipInitialDelay/* = false*/)
{
    if (initiater == nullptr || !initiater->IsInWorld())
        return;

    std::lock_guard<std::mutex> guard(m_mutexCombat);
    initiater->getCombatHandler()._notifyOnCombatInitiated(getOwner());

    if (!friendlyCombat)
    {
        if (skipInitialDelay)
        {
            const auto msTime = Util::getMSTime();
            _combatAction(initiater, msTime, false, false);
            return;
        }

        m_combatBatchAttackedByTargets.insert(std::make_pair(initiater->getGuid(), BatchTargetInfo(initiater, false)));
    }
}

void CombatHandler::clearCombat()
{
    std::lock_guard<std::mutex> guard(m_mutexCombat);

    _leaveCombat();
    m_combatBatchAttackingTargets.clear();
    m_combatBatchAttackedByTargets.clear();
    m_combatInitiatedWith.clear();
}

void CombatHandler::onRemoveFromWorld()
{
    // Clear this unit from all in range units combat map to avoid crashes when removed from world
    for (const auto& obj : getOwner()->getInRangeObjectsSet())
    {
        if (obj == nullptr || !obj->isCreatureOrPlayer())
            continue;

        dynamic_cast<Unit*>(obj)->getCombatHandler().onUnitRemovedFromWorld(getOwner()->getGuid());
    }

    clearCombat();
}

void CombatHandler::onUnitRemovedFromWorld(uint64_t guid)
{
    std::lock_guard<std::mutex> guard(m_mutexCombat);

    // No need to remove from player combat map
    m_combatBatchAttackingTargets.erase(guid);
    m_combatBatchAttackedByTargets.erase(guid);
    m_combatInitiatedWith.erase(guid);
}

bool CombatHandler::isInCombat() const
{
    return m_inCombat;
}

bool CombatHandler::isInCombatWithPlayer(Player const* player) const
{
    if (!isInCombat())
        return false;

    std::lock_guard<std::mutex> guard(m_mutexPlayerCombat);
    return m_combatPlayerTargets.find(player->getGuid()) != m_combatPlayerTargets.end();
}

bool CombatHandler::isInPreCombatWithUnit(Unit const* player) const
{
    std::lock_guard<std::mutex> guard(m_mutexCombat);
    return m_combatBatchAttackingTargets.find(player->getGuid()) != m_combatBatchAttackingTargets.end();
}

void CombatHandler::updateCombat(uint32_t msTime)
{
    auto diff = msTime - m_lastCombatUpdateTime;
    if (diff >= COMBAT_BATCH_INTERVAL)
    {
        m_lastCombatUpdateTime = msTime;

        // Make sure unit cannot leave combat in the same update tick he got it
        auto hadCombatAction = false;

        {
            std::lock_guard<std::mutex> guard(m_mutexCombat);

            // Unit is attacking or healing these targets
            if (!m_combatBatchAttackingTargets.empty())
            {
                for (auto itr = m_combatBatchAttackingTargets.cbegin(); itr != m_combatBatchAttackingTargets.cend();)
                {
                    // Make sure target still exists
                    auto* const target = itr->second.unit;
                    if (target == nullptr || !target->IsInWorld())
                    {
                        itr = m_combatBatchAttackingTargets.erase(itr);
                        continue;
                    }

                    _checkPvpFlags(target, itr->second.isFriendly);
                    _combatAction(target, msTime, itr->second.isFriendly, true);
                    hadCombatAction = true;

                    itr = m_combatBatchAttackingTargets.erase(itr);
                }
            }

            // Unit is being attacked or healed by these targets
            if (!m_combatBatchAttackedByTargets.empty())
            {
                for (auto itr = m_combatBatchAttackedByTargets.cbegin(); itr != m_combatBatchAttackedByTargets.cend();)
                {
                    // Make sure attacker still exists
                    auto* const attacker = itr->second.unit;
                    if (attacker == nullptr || !attacker->IsInWorld())
                    {
                        itr = m_combatBatchAttackedByTargets.erase(itr);
                        continue;
                    }

                    if (getOwner()->isPlayer() && attacker->isPlayer())
                    {
                        // Special pvp case - neither attacker or victim should enter in combat
                        // if attacker managed to clear combat right after attack
                        // i.e. if rogue casts vanish right after ambush but before combat batch,
                        // neither rogue or victim should enter in combat and rogue should be able to sap target
                        if (!attacker->getCombatHandler().isInPreCombatWithUnit(getOwner()) &&
                            !attacker->getCombatHandler().isInCombatWithPlayer(dynamic_cast<Player*>(getOwner())))
                        {
                            itr = m_combatBatchAttackedByTargets.erase(itr);
                            continue;
                        }
                    }

                    _combatAction(attacker, msTime, false, false);
                    hadCombatAction = true;

                    itr = m_combatBatchAttackedByTargets.erase(itr);
                }
            }
        }

        if (m_justEnteredInCombat)
        {
            m_justEnteredInCombat = false;

            if (getOwner()->isPlayer())
                sHookInterface.OnEnterCombat(dynamic_cast<Player*>(getOwner()), m_enterCombatInfo.enteringCombatWith);

            if (getOwner()->isCreature() && getOwner()->getAIInterface())
                getOwner()->getAIInterface()->justEnteredCombat(m_enterCombatInfo.enteringCombatWith);

            // If summons get in combat master will also get in combat
            // However if master gets in combat summons won't get in combat if kept on passive
            _notifyOwner(m_enterCombatInfo.isFriendly, m_enterCombatInfo.enteringCombatWith, m_enterCombatInfo.initiatingCombat);
        }

        if (!isInCombat() || hadCombatAction)
            return;

        std::lock_guard<std::mutex> guard(m_mutexCombat);
        if (!m_combatInitiatedWith.empty())
        {
            for (auto itr = m_combatInitiatedWith.begin(); itr != m_combatInitiatedWith.end();)
            {
                // First check, set time
                if (itr->second == 0)
                {
                    itr->second = msTime;
                    ++itr;
                    continue;
                }

                // If creature is in this container for over 4.5 seconds it can be assumed that the combat is bugged
                // Remove creature from map and try clear combat in next update
                diff = msTime - itr->second;
                if (diff >= PVP_COMBAT_LEAVE_MIN_TIME)
                {
                    itr = m_combatInitiatedWith.erase(itr);
                    continue;
                }
                else
                {
                    ++itr;
                }
            }

            return;
        }

        // Creatures are simple; if threat list is empty then leave combat, else stay in combat
        if (getOwner()->isCreature() && getOwner()->getPlayerOwner() == nullptr)
        {
            if (getOwner()->getThreatManager().isThreatListEmpty())
                _leaveCombat();

            return;
        }

        // At this point owner is either player or player controlled creature (pet, summon etc)
        // If player or pet has threat with something => stay in combat
        if (!getOwner()->getThreatManager().getThreatenedByMeList().empty())
            return;

        // Owner is not in combat with creatures
        // Check if owner is still in combat with other players or pets
        if (!m_combatPlayerTargets.empty())
            return;

        // Owner can leave combat
        _leaveCombat();
    }

    // Update pvp combat
    diff = msTime - m_lastPvPCombatUpdateTime;
    if (diff >= PVP_COMBAT_TIMER_PULSE)
    {
        std::lock_guard<std::mutex> guard(m_mutexCombat);

        // Timer must keep going even if unit is not in combat
        _updatePvpCombat(msTime);
        m_lastPvPCombatUpdateTime = msTime;
    }
}

Unit* CombatHandler::getOwner() const
{
    return m_owner;
}

void CombatHandler::_enterCombat(bool initiatingCombat, Unit* enteringCombatWith, bool friendlyTarget)
{
    if (m_inCombat)
        return;

    getOwner()->addUnitFlags(UNIT_FLAG_COMBAT);
    m_inCombat = true;

    m_enterCombatInfo.enteringCombatWith = enteringCombatWith;
    m_enterCombatInfo.isFriendly = friendlyTarget;
    m_enterCombatInfo.initiatingCombat = initiatingCombat;
    m_justEnteredInCombat = true;
}

void CombatHandler::_leaveCombat()
{
    if (!m_inCombat)
        return;

    getOwner()->removeUnitFlags(UNIT_FLAG_COMBAT);
    m_inCombat = false;

    // Potion cooldown starts when combat ends
    if (getOwner()->isPlayer())
        dynamic_cast<Player*>(getOwner())->updatePotionCooldown();

    std::lock_guard<std::mutex> guard(m_mutexPlayerCombat);
    m_combatPlayerTargets.clear();
}

void CombatHandler::_combatAction(Unit* target, uint32_t msTime, bool friendlyAction, bool initiatingCombat)
{
    _resetPvpCombatTimer(msTime, target);

    // Set owner in combat
    // Victim is set in combat when spell lands on target or damage is done
    if (!isInCombat())
        _enterCombat(initiatingCombat, target, friendlyAction);
    else
        _notifyOwner(friendlyAction, target, initiatingCombat);
}

void CombatHandler::_initiateCombatWith(Unit* enteringCombatWith)
{
    if (enteringCombatWith->getPlayerOwnerOrSelf() != nullptr)
        return;

    m_combatInitiatedWith.insert(std::make_pair(enteringCombatWith->getGuid(), 0U));
}

void CombatHandler::_notifyOnCombatInitiated(Unit* initiatedCombatWith)
{
    if (initiatedCombatWith->getPlayerOwnerOrSelf() != nullptr)
        return;

    m_combatInitiatedWith.erase(initiatedCombatWith->getGuid());
}

void CombatHandler::_notifyOwner(bool friendlyAction, Unit* enteringCombatWith, bool initiatingCombat)
{
    // If unit has an owner, the owner should also get in combat
    auto* const owner = getOwner()->getUnitOwner();
    if (owner == nullptr || owner == getOwner())
        return;

    if (enteringCombatWith->isPlayer() && owner->hasUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT))
        return;

    if (enteringCombatWith->isCreature() && enteringCombatWith->getPlayerOwner() == nullptr)
    {
        if (owner->hasUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT))
            return;
    }

    if (owner->hasUnitFlags(UNIT_FLAG_NON_ATTACKABLE))
        return;

    // Skip combat delay for owner as it was already delayed for this unit
    if (initiatingCombat)
    {
        if (friendlyAction)
            owner->getCombatHandler().onFriendlyAction(enteringCombatWith, true);
        else
            owner->getCombatHandler().onHostileAction(enteringCombatWith, true);
    }
    else
    {
        owner->getCombatHandler().takeCombatAction(enteringCombatWith, false, true);
    }
}

void CombatHandler::_checkPvpFlags(Unit* target, bool friendlyAction)
{
    const auto playerOwner = getOwner()->getPlayerOwnerOrSelf();
    if (playerOwner == nullptr)
        return;

    if (target->isPvpFlagSet())
    {
        if (getOwner()->isPet())
        {
            if (!getOwner()->isPvpFlagSet())
                playerOwner->togglePvP();

            if (!friendlyAction)
                playerOwner->aggroPvPGuards();
        }
        else
        {
            if (!playerOwner->isPvpFlagSet())
                playerOwner->togglePvP();

            if (!friendlyAction)
                playerOwner->aggroPvPGuards();
        }
    }
}

void CombatHandler::_updatePvpCombat(uint32_t msTime)
{
    if (!isInCombat())
        return;

    std::lock_guard<std::mutex> guard(m_mutexPlayerCombat);
    for (auto itr = m_combatPlayerTargets.cbegin(); itr != m_combatPlayerTargets.cend();)
    {
        const auto combatDiff = msTime - itr->second;
        if (combatDiff >= PVP_COMBAT_LEAVE_MIN_TIME)
        {
            // Last hostile action with this target was over 4.5 seconds ago
            itr = m_combatPlayerTargets.erase(itr);
        }
        else
        {
            ++itr;
        }
    }
}

void CombatHandler::_resetPvpCombatTimer(uint32_t msTime, Unit* victim)
{
    // No need to check if unit is also creature
    if (victim->getPlayerOwnerOrSelf() == nullptr)
        return;

    std::lock_guard<std::mutex> guard(m_mutexPlayerCombat);
    auto itr = m_combatPlayerTargets.find(victim->getGuid());
    if (itr != m_combatPlayerTargets.end())
    {
        itr->second = msTime;
        return;
    }

    m_combatPlayerTargets.insert(std::make_pair(victim->getGuid(), msTime));
}
