/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_HellfireRamparts.hpp"
#include "WatchkeeperGargolmar.hpp"
#include "Omor.hpp"
#include "NazanAndVazruden.hpp"
#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Hellfire Ramparts Instance
HellfireRampartsInstanceScript::HellfireRampartsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
{

}

InstanceScript* HellfireRampartsInstanceScript::Create(WorldMap* pMapMgr) { return new HellfireRampartsInstanceScript(pMapMgr); }

void HellfireRampartsInstanceScript::OnCreaturePushToWorld(Creature* pCreature)
{
    WoWGuid guid = pCreature->getGuid();

    switch (pCreature->getEntry())
    {
        case NPC_VAZRUDEN_HERALD:
            vazrudenHeraldGUID = guid.getGuidLowPart();
            break;
    }
}

void HellfireRampartsInstanceScript::OnGameObjectPushToWorld(GameObject* pGameObject)
{
    switch (pGameObject->getEntry())
    {
        case GO_FEL_IRON_CHEST_NORMAL:
        case GO_FEL_IRON_CHEST_HEROIC:
            felIronChestGUID = pGameObject->getGuidLow();
            break;
    }
}

void HellfireRampartsInstanceScript::OnEncounterStateChange(uint32_t entry, uint32_t /*state*/)
{
    switch (entry)
    {
        case DATA_VAZRUDEN:
        case DATA_NAZAN:
        {
            if (getBossState(DATA_VAZRUDEN) == Performed && getBossState(DATA_NAZAN) == Performed)
                if (GameObject* chest = GetGameObjectByGuid(felIronChestGUID))
                    chest->removeFlags(GO_FLAG_NOT_SELECTABLE);
        } break;
        default:
            break;
    }
}

void HellfireRampartsInstanceScript::OnSpawnGroupKilled(uint32_t groupId)
{
    if (groupId == 1)
    {
        if (Creature* vazruden = GetCreatureByGuid(vazrudenHeraldGUID))
            if (vazruden->GetScript())
                vazruden->GetScript()->DoAction(0);
    }
}

void SetupHellfireRamparts(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HC_RAMPARTS, &HellfireRampartsInstanceScript::Create);
    
    mgr->register_creature_script(NPC_WATCHKEEPER_GARGOLMAR, &WatchkeeperGargolmarAI::Create);
    mgr->register_creature_script(NPC_OMOR_THE_UNSCARRED, &OmorTheUnscarredAI::Create);
    mgr->register_creature_script(NPC_NAZAN, &NazanAI::Create);
    mgr->register_creature_script(NPC_VAZRUDEN, &VazrudenAI::Create);
    mgr->register_creature_script(NPC_VAZRUDEN_HERALD, &VazrudenTheHeraldAI::Create);
}
