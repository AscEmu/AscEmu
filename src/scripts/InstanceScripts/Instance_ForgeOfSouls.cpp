 /* AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "Setup.h"
#include "Instance_ForgeOfSouls.h"

class InstanceForgeOfSoulsScript : public InstanceScript
{
public:

	InstanceForgeOfSoulsScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
	{}

    static InstanceScript* Create(MapMgr* pMapMgr) { return new InstanceForgeOfSoulsScript(pMapMgr); }

	void OnPlayerEnter(Player* player) override
	{
        if (!spawnsCreated())
        {
            if (player->GetTeam() == TEAM_ALLIANCE)
            {
                spawnCreature(CN_JAINA_PROUDMOORE, 4900.08f, 2208.45f, 638.73f, 5.48f, 35);
                spawnCreature(CN_ARCHMAGE_ELANDRA, 4899.95f, 2206.149f, 638.73f, 5.37f, 35);
                spawnCreature(CN_ARCHMAGE_KORELN, 4902.95f, 2212.69f, 638.73f, 35);
            }
            else // TEAM_HORDE
            {
                spawnCreature(CN_SYLVANAS_WINDRUNNER, 4900.08f, 2208.45f, 638.73f, 5.48f, 35);
                spawnCreature(CN_DARK_RANGER_LORALEN, 4899.95f, 2206.149f, 638.73f, 5.37f, 35);
                spawnCreature(CN_DARK_RANGER_KALIRA, 4902.95f, 2212.69f, 638.73f, 35);
            }

            setSpawnsCreated();
        }
	}
};

void SetupForgeOfSouls(ScriptMgr* mgr)
{
#ifndef UseNewMapScriptsProject
    mgr->register_instance_script(MAP_FORGE_OF_SOULS, &InstanceForgeOfSoulsScript::Create);
#endif
}