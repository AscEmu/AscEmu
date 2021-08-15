/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/Script/CreatureAIScript.h"
#include "Units/Creatures/Creature.h"
#include "Macros/ScriptMacros.hpp"

class ShatteredSunSpawner : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShatteredSunSpawner)
    explicit ShatteredSunSpawner(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* mTarget) override
    {
        if (mTarget != NULL && mTarget->isPlayer())
        {
            for (uint8_t i = 0; i < 3; ++i)
            {
                float x = mTarget->GetPositionX() + Util::getRandomUInt(20) - 10;
                float y = mTarget->GetPositionY() + Util::getRandomUInt(20) - 10;
                float z = mTarget->GetPositionZ();
                Creature* guard = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(26253, x, y, z, 0, true, false, getCreature()->getFactionTemplate(), 50);

                if (guard != NULL)
                {
                    setGuardWaypoints();
                    guard->GetAIInterface()->onHostileAction(mTarget);
                    getCreature()->Despawn(60000, 0);
                }
            }
        }
    }

    // Generates 3 random waypoints around the NPC
    void setGuardWaypoints()
    {
        if (!getCreature()->GetMapMgr())
            return;

        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float o = 0.0f;

        for (uint8 i = 1; i <= 4; i++)
        {
            float ang = Util::getRandomFloat(100.0f) / 100.0f;
            float ran = Util::getRandomFloat(100.0f) / 10.0f;
            while (ran < 1)
                ran = Util::getRandomFloat(100.0f) / 10.0f;

            x = getCreature()->GetSpawnX() + ran * sin(ang);
            y = getCreature()->GetSpawnY() + ran * cos(ang);
            z = getCreature()->GetMapMgr()->GetLandHeight(x, y, getCreature()->GetSpawnZ() + 2);

            addWaypoint(1, createWaypoint(i, 800, WAYPOINT_MOVE_TYPE_WALK, LocationVector(x, y, z, o)));
        }
    }
};

void SetupNeutralGuards(ScriptMgr* mgr)
{
    mgr->register_creature_script(24994, &ShatteredSunSpawner::Create); // Shat Sun Sentry
}
