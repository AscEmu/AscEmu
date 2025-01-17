/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_BlackfathomDeeps.h"

#include "Setup.h"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class BlackfathomDeepsInstanceScript : public InstanceScript
{
public:
    explicit BlackfathomDeepsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new BlackfathomDeepsInstanceScript(pMapMgr); }
};

class FathomStone : public GameObjectAIScript
{
public:
    explicit FathomStone(GameObject* goinstance) : GameObjectAIScript(goinstance)
    {
        SpawnBaronAquanis = true;
    }

    static GameObjectAIScript* Create(GameObject* GO) { return new FathomStone(GO); }

    void OnActivate(Player* pPlayer) override
    {
        if (pPlayer->isTeamHorde() && SpawnBaronAquanis == true) // Horde
        {
            // Spawn Baron Aquanis
            _gameobject->getWorldMap()->getInterface()->spawnCreature(BlackfathomDeeps::CN_BARON_AQUANIS, LocationVector(-782.021f, -63.5876f, -45.0935f, -2.44346f), true, false, 0, 0);
            SpawnBaronAquanis = false;
        }
    }

protected:
    bool SpawnBaronAquanis;
};

class MorriduneGossip : public GossipScript
{
    void onHello(Object* pObject, Player* pPlayer) override
    {
        GossipMenu menu(pObject->getGuid(), BlackfathomDeeps::MORRIDUNE_ON_HELLO, 0);
        if (pPlayer->isTeamAlliance())
            menu.addItem(GOSSIP_ICON_CHAT, BlackfathomDeeps::MORRIDUNE_OPTION_1, 1);
        else
            menu.addItem(GOSSIP_ICON_CHAT, BlackfathomDeeps::MORRIDUNE_OPTION_2, 2);

        menu.sendGossipPacket(pPlayer);
    }

    void onSelectOption(Object* /*pObject*/, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
                pPlayer->safeTeleport(1, 0, LocationVector(9951.52f, 2280.32f, 1341.39f, 0));
                break;
            case 2:
                pPlayer->safeTeleport(1, 0, LocationVector(4247.74f, 745.879f, -24.2967f, 4.36996f));
                break;
        }

        GossipMenu::senGossipComplete(pPlayer);
    }
};

void SetupBlackfathomDeeps(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACKFATHOM_DEEPS, &BlackfathomDeepsInstanceScript::Create);

    mgr->register_creature_gossip(BlackfathomDeeps::CN_MORRIDUNE, new MorriduneGossip());

    mgr->register_gameobject_script(BlackfathomDeeps::GO_FATHOM_STONE, &FathomStone::Create);
}
