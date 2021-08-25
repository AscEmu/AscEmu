/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_BlackfathomDeeps.h"

#include "Server/Script/CreatureAIScript.h"
#include "Macros/ScriptMacros.hpp"

class BlackfathomDeepsInstanceScript : public InstanceScript
{
public:

    explicit BlackfathomDeepsInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new BlackfathomDeepsInstanceScript(pMapMgr); }
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
            _gameobject->GetMapMgr()->GetInterface()->SpawnCreature(CN_BARON_AQUANIS, -782.021f, -63.5876f, -45.0935f, -2.44346f, true, false, 0, 0);
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
        GossipMenu menu(pObject->getGuid(), MORRIDUNE_ON_HELLO, 0);
        if (pPlayer->isTeamAlliance())
            menu.addItem(GOSSIP_ICON_CHAT, MORRIDUNE_OPTION_1, 1);
        else
            menu.addItem(GOSSIP_ICON_CHAT, MORRIDUNE_OPTION_2, 2);

        menu.sendGossipPacket(pPlayer);
    }

    void onSelectOption(Object* /*pObject*/, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
                pPlayer->SafeTeleport(1, 0, 9951.52f, 2280.32f, 1341.39f, 0);
                break;
            case 2:
                pPlayer->SafeTeleport(1, 0, 4247.74f, 745.879f, -24.2967f, 4.36996f);
                break;
        }

        GossipMenu::senGossipComplete(pPlayer);
    }
};

void SetupBlackfathomDeeps(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACKFATHOM_DEEPS, &BlackfathomDeepsInstanceScript::Create);

    mgr->register_creature_gossip(CN_MORRIDUNE, new MorriduneGossip());

    mgr->register_gameobject_script(GO_FATHOM_STONE, &FathomStone::Create);
}
