/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_RagefireChasm.h"

#include "Server/Script/CreatureAIScript.h"
#include "Macros/ScriptMacros.hpp"

class RagefireChasmInstanceScript : public InstanceScript
{
public:

    explicit RagefireChasmInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new RagefireChasmInstanceScript(pMapMgr); }
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
            pPlayer->GetSession()->SendNotification("Request quest `The Path of the Adept`.");
            return;
        }
        Creature* Zelemar = NULL;
        Zelemar = _gameobject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-370.133f, 162.519f, -21.1299f, CN_ZELMAR);
        if (Zelemar)
            return;

        // Spawn Zelemar the Wrathful
        Zelemar = _gameobject->GetMapMgr()->GetInterface()->SpawnCreature(17830, -370.133f, 162.519f, -21.1299f, -1.29154f, true, false, 0, 0);
        if (Zelemar)
        {
            Zelemar->m_noRespawn = true;
            Zelemar = NULL;
        }
    }
};

void SetupRagefireChasm(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_RAGEFIRE_CHASM, &RagefireChasmInstanceScript::Create);

    mgr->register_gameobject_script(GO_BLOOD_FILLED_ORB, &BloodFilledOrb::Create);
}
