/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Map/Maps/MapScriptInterface.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Utilities/Random.hpp"

class ShatteredSunSpawner : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShatteredSunSpawner(c); }
    explicit ShatteredSunSpawner(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* mTarget) override
    {
        if (mTarget != nullptr && mTarget->isPlayer())
        {
            for (uint8_t i = 0; i < 3; ++i)
            {
                float x = mTarget->GetPositionX() + Util::getRandomUInt(20) - 10;
                float y = mTarget->GetPositionY() + Util::getRandomUInt(20) - 10;
                float z = mTarget->GetPositionZ();
                Creature* guard = getCreature()->getWorldMap()->getInterface()->spawnCreature(26253, LocationVector(x, y, z), true, false, getCreature()->getFactionTemplate(), 50);

                if (guard != nullptr)
                {
                    setGuardWaypoints();
                    guard->getAIInterface()->onHostileAction(mTarget);
                    getCreature()->Despawn(60000, 0);
                }
            }
        }
    }

    // Generates 3 random waypoints around the NPC
    void setGuardWaypoints()
    {
        if (!getCreature()->getWorldMap())
            return;

        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float o = 0.0f;

        for (uint8_t i = 1; i <= 4; i++)
        {
            float ang = Util::getRandomFloat(100.0f) / 100.0f;
            float ran = Util::getRandomFloat(100.0f) / 10.0f;
            while (ran < 1)
                ran = Util::getRandomFloat(100.0f) / 10.0f;

            x = getCreature()->GetSpawnX() + ran * sin(ang);
            y = getCreature()->GetSpawnY() + ran * cos(ang);
            z = getCreature()->getMapHeight(LocationVector(x, y, getCreature()->GetSpawnZ() + 2));

            addWaypoint(1, createWaypoint(i, 800, WAYPOINT_MOVE_TYPE_WALK, LocationVector(x, y, z, o)));
        }
    }
};

void SetupNeutralGuards(ScriptMgr* mgr)
{
    mgr->register_creature_script(24994, &ShatteredSunSpawner::Create); // Shat Sun Sentry
}
