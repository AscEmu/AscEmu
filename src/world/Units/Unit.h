// License: MIT

#include "Objects/Object.h"

#pragma once

// Prevent compiler from trying to compile duplicate class definitions
#define UNIT_CLASS_PARTIAL

// AGPL Start
#include "Unit.Legacy.Partial1.h"
// AGPL End

class SERVER_DECL Unit : public Object
{
    AscEmu::World::Units::CombatStatus m_combatStatus;
    void setCombatFlag(bool enabled);

public:
    bool isInCombat() const;
    bool isAttacking(Unit* target) const;

    void enterCombat();
    void leaveCombat();
    void onDamageDealt(Unit* target);

    void addHealTarget(Unit* target);
    void removeHealTarget(Unit* target);

    void addHealer(Unit* healer);
    void removeHealer(Unit* healer);

    void addAttacker(Unit* attacker);
    bool hasAttacker(uint64_t guid) const;
    void removeAttacker(Unit* attacker);
    void removeAttacker(uint64_t guid);

    void removeAttackTarget(Unit* attackTarget);

    void updateCombatStatus();
    void clearAllCombatTargets();

    uint64_t getPrimaryAttackTarget() const;

    // Do not alter anything below this line
    // -------------------------------------
private:
    // AGPL Start
    #include "Unit.Legacy.Partial2.h"
    // AGPL End
};