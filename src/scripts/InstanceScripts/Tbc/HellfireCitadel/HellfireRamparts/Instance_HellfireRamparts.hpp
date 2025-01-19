/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Server/Script/InstanceScript.hpp"

enum Encounters
{
    DATA_WATCHKEEPER_GARGOLMAR      = 0,
    DATA_OMOR_THE_UNSCARRED         = 1,
    DATA_VAZRUDEN                   = 2,
    DATA_NAZAN                      = 3
};

enum CreatureEntry
{
    NPC_WATCHKEEPER_GARGOLMAR       = 17306,
    NPC_VAZRUDEN_HERALD             = 17307,
    NPC_OMOR_THE_UNSCARRED          = 17308,
    NPC_VAZRUDEN                    = 17537,
    NPC_NAZAN                       = 17536,
    NPC_LIQUID_FIRE                 = 22515

};

enum GameobjectEntry
{
    GO_FEL_IRON_CHEST_NORMAL        = 185168,
    GO_FEL_IRON_CHEST_HEROIC        = 185169
};

class HellfireRampartsInstanceScript : public InstanceScript
{
public:
    explicit HellfireRampartsInstanceScript(WorldMap* pMapMgr);
    static InstanceScript* Create(WorldMap* pMapMgr);

    void OnCreaturePushToWorld(Creature* /*pCreature*/) override;
    void OnGameObjectPushToWorld(GameObject* /*pGameObject*/) override;
    void OnEncounterStateChange(uint32_t /*entry*/, uint32_t /*state*/) override;
    void OnSpawnGroupKilled(uint32_t /*groupId*/) override;

protected:
    uint32_t felIronChestGUID = 0;
    uint32_t vazrudenHeraldGUID = 0;
};
