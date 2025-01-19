/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_RagefireChasm.h"

#include "Setup.h"
#include "Map/Maps/MapScriptInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class RagefireChasmInstanceScript : public InstanceScript
{
public:
    explicit RagefireChasmInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new RagefireChasmInstanceScript(pMapMgr); }
};

class BloodFilledOrb : public GameObjectAIScript
{
public:
    explicit BloodFilledOrb(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new BloodFilledOrb(GO); }

    void OnActivate(Player* pPlayer) override
    {
        // Make sure player has the quest and Zelemar isn't spawned yet
        if (!pPlayer->hasQuestInQuestLog(9692)) // The Path of the Adept
        {
            pPlayer->getSession()->SendNotification("Request quest `The Path of the Adept`.");
            return;
        }

        Creature* Zelemar = _gameobject->getWorldMap()->getInterface()->getCreatureNearestCoords(-370.133f, 162.519f, -21.1299f, RagefireChasm::CN_ZELMAR);
        if (Zelemar)
            return;

        // Spawn Zelemar the Wrathful
        Zelemar = _gameobject->getWorldMap()->getInterface()->spawnCreature(17830, LocationVector(-370.133f, 162.519f, -21.1299f, -1.29154f), true, false, 0, 0);
        if (Zelemar)
        {
            Zelemar->m_noRespawn = true;
            Zelemar = nullptr;
        }
    }
};

void SetupRagefireChasm(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_RAGEFIRE_CHASM, &RagefireChasmInstanceScript::Create);

    mgr->register_gameobject_script(RagefireChasm::GO_BLOOD_FILLED_ORB, &BloodFilledOrb::Create);
}
