/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Icecrown_Citadel.h"

class IceCrownCitadel : public InstanceScript
{
public:

    IceCrownCitadel(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new IceCrownCitadel(pMapMgr); }
};

class ICCTeleporterGossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* object, Player* player) override
    {
        IceCrownCitadel* pInstance = (IceCrownCitadel*)object->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        Arcemu::Gossip::Menu menu(object->GetGUID(), 15221, player->GetSession()->language);
        menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(515), 0);          // Teleport to Light's Hammer.

        if (pInstance->isDataStateFinished(CN_LORD_MARROWGAR))
            menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(516), 1);      // Teleport to Oratory of The Damned.

        if (pInstance->isDataStateFinished(CN_LADY_DEATHWHISPER))
            menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(517), 2);      // Teleport to Rampart of Skulls.

        // GunshipBattle has to be finished...
        //menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(518), 3);        // Teleport to Deathbringer's Rise.

        if (pInstance->isDataStateFinished(CN_VALITHRIA_DREAMWALKER))
            menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(519), 4);      // Teleport to the Upper Spire.

        if (pInstance->isDataStateFinished(CN_COLDFLAME))
            menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(520), 5);      // Teleport to Sindragosa's Lair.

        menu.Send(player);
    }

    void OnSelectOption(Object* /*object*/, Player* player, uint32 Id, const char* /*enteredcode*/, uint32 /*gossipId*/) override
    {
        switch (Id)
        {
            case 0:
                player->CastSpell(player, 70781, true);     // Light's Hammer
                break;
            case 1:
                player->CastSpell(player, 70856, true);     // Oratory of The Damned
                break;
            case 2:
                player->CastSpell(player, 70857, true);     // Rampart of Skulls
                break;
            case 3:
                player->CastSpell(player, 70858, true);     // Deathbringer's Rise
                break;
            case 4:
                player->CastSpell(player, 70859, true);     // Upper Spire
                break;
            case 5:
                player->CastSpell(player, 70861, true);     // Sindragosa's Lair
                break;
        }
        Arcemu::Gossip::Menu::Complete(player);
    }
};

class IcecrownCitadelTeleport : public GameObjectAIScript
{
public:

    IcecrownCitadelTeleport(GameObject* go) : GameObjectAIScript(go)
    {
    }

    static GameObjectAIScript* Create(GameObject* go) { return new IcecrownCitadelTeleport(go); }

    void OnCreate() override
    {
        _gameobject->SetFlags(32);
    }

    void OnActivate(Player* player) override
    {
        ICCTeleporterGossip gossip;
        gossip.OnHello(_gameobject, player);
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
