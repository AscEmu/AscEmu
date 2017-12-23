/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 <http://www.arcemu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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
            if (mTarget != NULL && mTarget->IsPlayer())
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
    mgr->register_creature_script(24994, &ShatteredSunSpawner::Create); //Shat Sun Sentry
}
