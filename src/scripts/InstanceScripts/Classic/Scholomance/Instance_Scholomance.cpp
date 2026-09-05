/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Scholomance.h"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum ScholomanceData
{
    DATA_MALICIA = 0,
    DATA_THEOLEN = 1,
    DATA_POLKELT = 2,
    DATA_RAVENIAN = 3,
    DATA_ALEXEI_BAROV = 4,
    DATA_ILLUCIA_BAROV = 5,
    DATA_GANDLING = 6
};

enum ScholomanceCreatures
{
    NPC_INSTRUCTOR_MALICIA = 10505,
    NPC_THEOLEN_KRASTINOV = 11261,
    NPC_LOREKEEPER_POLKELT = 10901,
    NPC_RAVENIAN = 10507,
    NPC_ALEXEI_BAROV = 10504,
    NPC_ILLUCIA_BAROV = 10502,
    NPC_DARKMASTER_GANDLING = 1853
};

class ScholomanceInstanceScript : public InstanceScript
{
public:
    explicit ScholomanceInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(7);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ScholomanceInstanceScript(pMapMgr); }

    void OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/) override
    {
        switch (pVictim->getEntry())
        {
            case NPC_INSTRUCTOR_MALICIA:
                setBossState(DATA_MALICIA, Performed);
                break;
            case NPC_THEOLEN_KRASTINOV:
                setBossState(DATA_THEOLEN, Performed);
                break;
            case NPC_LOREKEEPER_POLKELT:
                setBossState(DATA_POLKELT, Performed);
                break;
            case NPC_RAVENIAN:
                setBossState(DATA_RAVENIAN, Performed);
                break;
            case NPC_ALEXEI_BAROV:
                setBossState(DATA_ALEXEI_BAROV, Performed);
                break;
            case NPC_ILLUCIA_BAROV:
                setBossState(DATA_ILLUCIA_BAROV, Performed);
                break;
            case NPC_DARKMASTER_GANDLING:
                setBossState(DATA_GANDLING, Performed);
                return;
            default:
                return;
        }

        trySpawnGandling();
    }

    void OnPlayerEnter(Player* /*pPlayer*/) override
    {
        trySpawnGandling();
    }

private:
    void trySpawnGandling()
    {
        if (getBossState(DATA_GANDLING) == Performed)
            return;

        if (!getCreatureSetForEntry(NPC_DARKMASTER_GANDLING).empty())
            return;

        if (getBossState(DATA_MALICIA) != Performed || getBossState(DATA_THEOLEN) != Performed ||
            getBossState(DATA_POLKELT) != Performed || getBossState(DATA_RAVENIAN) != Performed ||
            getBossState(DATA_ALEXEI_BAROV) != Performed || getBossState(DATA_ILLUCIA_BAROV) != Performed)
            return;

        spawnCreature(NPC_DARKMASTER_GANDLING, 180.771f, -5.4286f, 75.5702f, 1.29154f);
    }
};

void SetupScholomance(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SCHOLOMANCE, &ScholomanceInstanceScript::Create);
}
