/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Icecrown_Citadel.h"

class IceCrownCitadel : public InstanceScript
{
public:

    explicit IceCrownCitadel(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new IceCrownCitadel(pMapMgr); }
};

class ICCTeleporterGossip : public GossipScript
{
public:

    void onHello(Object* object, Player* player) override
    {
        IceCrownCitadel* pInstance = (IceCrownCitadel*)object->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        GossipMenu menu(object->getGuid(), 15221, player->GetSession()->language);
        menu.addItem(GOSSIP_ICON_CHAT, 515, 0);          // Teleport to Light's Hammer.

        if (pInstance->isDataStateFinished(CN_LORD_MARROWGAR))
            menu.addItem(GOSSIP_ICON_CHAT, 516, 1);      // Teleport to Oratory of The Damned.

        if (pInstance->isDataStateFinished(CN_LADY_DEATHWHISPER))
            menu.addItem(GOSSIP_ICON_CHAT, 517, 2);      // Teleport to Rampart of Skulls.

        // GunshipBattle has to be finished...
        //menu.addItem(GOSSIP_ICON_CHAT, 518, 3);        // Teleport to Deathbringer's Rise.

        if (pInstance->isDataStateFinished(CN_VALITHRIA_DREAMWALKER))
            menu.addItem(GOSSIP_ICON_CHAT, 519, 4);      // Teleport to the Upper Spire.

        if (pInstance->isDataStateFinished(CN_COLDFLAME))
            menu.addItem(GOSSIP_ICON_CHAT, 520, 5);      // Teleport to Sindragosa's Lair.

        menu.sendGossipPacket(player);
    }

    void onSelectOption(Object* /*object*/, Player* player, uint32 Id, const char* /*enteredcode*/, uint32 /*gossipId*/) override
    {
        switch (Id)
        {
            case 0:
                player->castSpell(player, 70781, true);     // Light's Hammer
                break;
            case 1:
                player->castSpell(player, 70856, true);     // Oratory of The Damned
                break;
            case 2:
                player->castSpell(player, 70857, true);     // Rampart of Skulls
                break;
            case 3:
                player->castSpell(player, 70858, true);     // Deathbringer's Rise
                break;
            case 4:
                player->castSpell(player, 70859, true);     // Upper Spire
                break;
            case 5:
                player->castSpell(player, 70861, true);     // Sindragosa's Lair
                break;
        }
        GossipMenu::senGossipComplete(player);
    }
};

class IcecrownCitadelTeleport : public GameObjectAIScript
{
public:

    explicit IcecrownCitadelTeleport(GameObject* go) : GameObjectAIScript(go)
    {
    }

    static GameObjectAIScript* Create(GameObject* go) { return new IcecrownCitadelTeleport(go); }

    void OnCreate() override
    {
        _gameobject->setFlags(GO_FLAG_NEVER_DESPAWN);
    }

    void OnActivate(Player* player) override
    {
        ICCTeleporterGossip gossip;
        gossip.onHello(_gameobject, player);
    }

};

#ifdef UseNewMapScriptsProject
void IcecrownCitadel(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(631, &IceCrownCitadel::Create);

    scriptMgr->register_gameobject_script(GO_TELE_1, &IcecrownCitadelTeleport::Create);
    scriptMgr->register_go_gossip(GO_TELE_1, new ICCTeleporterGossip());

    scriptMgr->register_gameobject_script(GO_TELE_2, &IcecrownCitadelTeleport::Create);
    scriptMgr->register_go_gossip(GO_TELE_2, new ICCTeleporterGossip());

    scriptMgr->register_gameobject_script(GO_TELE_3, &IcecrownCitadelTeleport::Create);
    scriptMgr->register_go_gossip(GO_TELE_3, new ICCTeleporterGossip());

    scriptMgr->register_gameobject_script(GO_TELE_4, &IcecrownCitadelTeleport::Create);
    scriptMgr->register_go_gossip(GO_TELE_4, new ICCTeleporterGossip());

    scriptMgr->register_gameobject_script(GO_TELE_5, &IcecrownCitadelTeleport::Create);
    scriptMgr->register_go_gossip(GO_TELE_5, new ICCTeleporterGossip());
}
#endif
