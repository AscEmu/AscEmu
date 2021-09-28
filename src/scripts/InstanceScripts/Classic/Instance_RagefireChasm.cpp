/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_RagefireChasm.h"
#include "Server/Script/CreatureAIScript.h"

class RagefireChasmInstanceScript : public InstanceScript
{
public:
    explicit RagefireChasmInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new RagefireChasmInstanceScript(pMapMgr); }
};

class BloodFilledOrb : public GameObjectAIScript
{
public:
    explicit BloodFilledOrb(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new BloodFilledOrb(GO); }

    void OnActivate(Player* plr) override
    {
        // Make sure player has the quest and Zelemar isn't spawned yet
        if (!plr->hasQuestInQuestLog(9692)) // The Path of the Adept
        {
            plr->GetSession()->SendNotification("Request quest `The Path of the Adept`.");
            return;
        }

        Creature* pCreature = plr->MAP_CREATURE_NEAREST_COORDS(-370.133f, 162.519f, -21.1299f, RagefireChasm::CN_ZELMAR);
        if (pCreature)
            return;

        // Spawn Zelemar the Wrathful
        pCreature = plr->GetMapMgr()->GetInterface()->SpawnCreature(17830, -370.133f, 162.519f, -21.1299f, -1.29154f, true, false, 0, 0);
        if (pCreature)
        {
            pCreature->m_noRespawn = true;
            pCreature = nullptr;
        }
    }
};

void SetupRagefireChasm(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_RAGEFIRE_CHASM, &RagefireChasmInstanceScript::Create);

    mgr->register_gameobject_script(RagefireChasm::GO_BLOOD_FILLED_ORB, &BloodFilledOrb::Create);
}
