/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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

// Alliance guard
// Cast spell 54028 on horde player if he is in the alliance area
class SilverCovenantMageGuard : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(SilverCovenantMageGuard, MoonScriptCreatureAI);
        SilverCovenantMageGuard(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {}
};

// Horde guard
// Cast spell 54029 on alliance player if he is in the horde area
class SunreaversMageGuard : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(SunreaversMageGuard, MoonScriptCreatureAI);
        SunreaversMageGuard(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {}
};

void SetupCityDalaran(ScriptMgr* mgr)
{
    mgr->register_creature_script(29254, &SilverCovenantMageGuard::Create);
    mgr->register_creature_script(29255, &SunreaversMageGuard::Create);
}
