/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_ForgeOfSouls.h"

class InstanceForgeOfSoulsScript : public InstanceScript
{
public:

    explicit InstanceForgeOfSoulsScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {}

    static InstanceScript* Create(MapMgr* pMapMgr) { return new InstanceForgeOfSoulsScript(pMapMgr); }

    void OnPlayerEnter(Player* player) override
    {
        if (!spawnsCreated())
        {
            if (player->getTeam() == TEAM_ALLIANCE)
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
    mgr->register_instance_script(MAP_FORGE_OF_SOULS, &InstanceForgeOfSoulsScript::Create);
}