// License: MIT
#pragma once
#include "CommonTypes.hpp"
#include <memory>
#include <set>

class Unit;

namespace AscEmu { namespace World { namespace Units {
    class SERVER_DECL CombatStatus
    {
        // Attackers can be anything, but healers can only be players
        // TODO: Check this, it sounds wrong
        typedef std::set<uint64_t> AttackerSet;
        typedef std::set<uint32_t> HealerSet;

        Unit* m_unit;
        uint64_t m_primaryAttackTarget;

        HealerSet m_healingUs;
        HealerSet m_healedByUs;

        AttackerSet m_attackingUs;
        AttackerSet m_attackedByUs;

        bool m_lastStatus;

    public:
        CombatStatus(Unit* unit);

        bool isInCombat() const;
        void addHealer(Unit* unit);
    };
}}}
