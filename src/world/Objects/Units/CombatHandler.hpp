/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#include <map>
#include <mutex>

class Unit;
class Player;

class SERVER_DECL CombatHandler
{
public:
    CombatHandler(Unit* owner);
    ~CombatHandler() = default;

    // Damage dealt, applied aura or casted spell on hostile target
    void onHostileAction(Unit* victim, bool skipInitialDelay = false);
    // Healed, applied aura or casted spell on friendly target
    void onFriendlyAction(Unit* target, bool skipInitialDelay = false);
    // Damage taken, healed, applied aura or casted spell by attacker or healer
    void takeCombatAction(Unit* initiater, bool friendlyCombat = false, bool skipInitialDelay = false);

    void clearCombat();

    // If removed from world, clear this unit from nearby units combat map to avoid crashes
    void onRemoveFromWorld();
    void onUnitRemovedFromWorld(uint64_t guid);

    bool isInCombat() const;
    bool isInCombatWithPlayer(Player const* player) const;
    // Needed to check in special pvp case
    bool isInPreCombatWithUnit(Unit const* unit) const;
    void updateCombat(uint32_t msTime);

    Unit* getOwner() const;

private:
    struct BatchTargetInfo
    {
        BatchTargetInfo(Unit* unit, bool isFriendly) : unit(unit), isFriendly(isFriendly) {}

        Unit* unit = nullptr;
        bool isFriendly = false;
    };

    struct EnterCombatInfo
    {
        Unit* enteringCombatWith = nullptr;
        bool isFriendly = false;
        bool initiatingCombat = false;
    };

    // <Guid, BatchTargetInfo>
    typedef std::map<uint64_t, BatchTargetInfo> BatchTargetMap;
    // <Guid, last hostile action>
    typedef std::map<uint64_t, uint32_t> PlayerCombatMap;
    // <Guid, time since combat was inititated>
    typedef std::map<uint64_t, uint32_t> CombatInitiatedMap;

    void _enterCombat(bool initiatingCombat, Unit* enteringCombatWith, bool friendlyTarget);
    void _leaveCombat();

    // Delay enter combat script calls abit
    EnterCombatInfo m_enterCombatInfo;
    bool m_justEnteredInCombat = false;

    void _combatAction(Unit* target, uint32_t msTime, bool friendlyAction, bool initiatingCombat);

    // Only pure creatures are handled with these to avoid leave combat before threat is applied
    void _initiateCombatWith(Unit* enteringCombatWith);
    void _notifyOnCombatInitiated(Unit* initiatedCombatWith);

    void _notifyOwner(bool friendlyAction, Unit* enteringCombatWith, bool initiatingCombat);
    void _checkPvpFlags(Unit* target, bool friendlyAction);
    void _updatePvpCombat(uint32_t msTime);
    void _resetPvpCombatTimer(uint32_t msTime, Unit* victim);

    uint32_t m_lastCombatUpdateTime = 0;
    uint32_t m_lastPvPCombatUpdateTime = 0;
    bool m_inCombat = false;

    // Unit is attacking these targets
    BatchTargetMap m_combatBatchAttackingTargets;
    // Unit is being attacked by these targets
    BatchTargetMap m_combatBatchAttackedByTargets;
    // Contains players and player owned creatures
    PlayerCombatMap m_combatPlayerTargets;
    // Containts all creature targets owner has initiated combat with
    // Target will be cleared from map when target has got in combat with owner
    CombatInitiatedMap m_combatInitiatedWith;

    Unit* m_owner = nullptr;

    mutable std::mutex m_mutexCombat;
    mutable std::mutex m_mutexPlayerCombat;
};
