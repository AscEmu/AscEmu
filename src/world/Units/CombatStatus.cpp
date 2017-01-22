// License: MIT
#include "CombatStatus.h"
#include "Errors.h"
#include "Unit.h"

using namespace std;

namespace AscEmu { namespace World { namespace Units {

    CombatStatus::CombatStatus(Unit* unit) : m_unit(unit), m_primaryAttackTarget(0), m_lastStatus(false)
    {
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
                if (summon->GetPetOwner() == me && summon->getCombatStatus().isInCombat())
                {
                    return true;
                }

                return m_lastStatus;
            }
        }

        return false;
    }
}}}
