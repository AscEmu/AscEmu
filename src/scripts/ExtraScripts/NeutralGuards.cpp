/*
 Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 This file is released under the MIT license. See README-MIT for more information.
 */

#include "Setup.h"
#include "Map/MapMgr.h"
#include "Map/MapScriptInterface.h"
#include "Units/Creatures/Creature.h"
#include "Server/Script/ScriptMgr.h"

class ShatteredSunSpawner : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(ShatteredSunSpawner);
    ShatteredSunSpawner(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* mTarget)
    {
        if (mTarget != NULL && mTarget->isPlayer())
        {
            for (uint8 i = 0; i < 3; ++i)
            {
                float x = mTarget->GetPositionX() + Util::getRandomUInt(20) - 10;
                float y = mTarget->GetPositionY() + Util::getRandomUInt(20) - 10;
                float z = mTarget->GetPositionZ();
                Creature* guard = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(26253, x, y, z, 0, true, false, getCreature()->getUInt32Value(UNIT_FIELD_FACTIONTEMPLATE), 50);

                if (guard != NULL)
                {
                    guard->SetGuardWaypoints();
                    guard->GetAIInterface()->AttackReaction(mTarget, 1, 0);
                    getCreature()->Despawn(60000, 0);
                }
            }
        }
    }
};

void SetupNeutralGuards(ScriptMgr* mgr)
{
    mgr->register_creature_script(24994, &ShatteredSunSpawner::Create); // Shat Sun Sentry
}
