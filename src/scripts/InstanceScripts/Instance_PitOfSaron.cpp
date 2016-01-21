 /* AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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
#include "Instance_PitOfSaron.h"

class InstancePitOfSaronScript : public MoonInstanceScript
{
public:

	MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstancePitOfSaronScript, MoonInstanceScript);
	InstancePitOfSaronScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
	{
		// Way to select bosses
		BuildEncounterMap();
		if (mEncounters.size() == 0)
			return;

		for (EncounterMap::iterator Iter = mEncounters.begin(); Iter != mEncounters.end(); ++Iter)
		{
			if ((*Iter).second.mState != State_Finished)
				continue;
		}
	}

	void OnGameObjectPushToWorld(GameObject* pGameObject) { }

	void SetInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
	{
		if (pType != Data_EncounterState || pIndex == 0)
			return;

		EncounterMap::iterator Iter = mEncounters.find(pIndex);
		if (Iter == mEncounters.end())
			return;

		(*Iter).second.mState = (EncounterState)pData;
	}

	uint32 GetInstanceData(uint32 pType, uint32 pIndex)
	{
		if (pType != Data_EncounterState || pIndex == 0)
			return 0;

		EncounterMap::iterator Iter = mEncounters.find(pIndex);
		if (Iter == mEncounters.end())
			return 0;

		return (*Iter).second.mState;
	}

	void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
	{
		EncounterMap::iterator Iter = mEncounters.find(pCreature->GetEntry());
		if (Iter == mEncounters.end())
			return;

		(*Iter).second.mState = State_Finished;
	}

	void OnPlayerEnter(Player* player)
	{
		if (player->GetTeam() == TEAM_ALLIANCE)
		{
			PushCreature(CN_JAINA_PROUDMOORE, 441.39f, 213.32f, 528.71f, 0.10f, 35);
			PushCreature(CN_ARCHMAGE_ELANDRA, 439.26f, 215.89f, 528.71f, 0.02f, 35);
			PushCreature(CN_ARCHMAGE_KORELN, 440.35f, 211.154f, 528.71f, 6.15f, 35);
		}
		else // TEAM_HORDE
		{
			PushCreature(CN_SYLVANAS_WINDRUNNER, 441.39f, 213.32f, 528.71f, 0.10f, 35);
			PushCreature(CN_DARK_RANGER_LORALEN, 440.35f, 211.154f, 528.71f, 6.15f, 35);
			PushCreature(CN_DARK_RANGER_KALIRA, 439.26f, 215.89f, 528.71f, 0.02f, 35);
		}
	};
};

void SetupPitOfSaron(ScriptMgr* mgr)
{
	mgr->register_instance_script(MAP_PIT_OF_SARON, &InstancePitOfSaronScript::Create);
}