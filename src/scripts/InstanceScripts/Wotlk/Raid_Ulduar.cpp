/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Ulduar.h"

class UlduarInstanceScript : public InstanceScript
{
public:

    explicit UlduarInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new UlduarInstanceScript(pMapMgr); }


};

//////////////////////////////////////////////////////////////////////////////////////////
// Ulduar Teleporter
class UlduarTeleporterGossip : public GossipScript
{
public:

    //void OnHello(Object* object, Player* player)
    //{
    //    UlduarScript* pInstance = (UlduarScript*)player->GetMapMgr()->GetScript();
    //    if (!pInstance)
    //        return;

    //    Menu menu(object->getGuid(), 14424, player->GetSession()->language);
    //    menu.addItem(GOSSIP_ICON_CHAT, 521, 0);          // Expedition Base Camp.

    //    // Unlock after engaging Flame Leviathan
    //    menu.addItem(GOSSIP_ICON_CHAT, (522), 1);          // Formation Grounds

    //    if (pInstance->GetInstanceData(Data_EncounterState, CN_FLAME_LEVIATHAN) == State_Finished)
    //        menu.addItem(GOSSIP_ICON_CHAT, (523), 2);      // Colossal Forge

    //    if (pInstance->GetInstanceData(Data_EncounterState, CN_XT_002_DECONSTRUCTOR) == State_Finished)
    //    {
    //        menu.addItem(GOSSIP_ICON_CHAT, (524), 3);      // Scrapyard
    //        menu.addItem(GOSSIP_ICON_CHAT, (525), 4);      // Antechamber of Ulduar
    //    }

    //    if (pInstance->GetInstanceData(Data_EncounterState, CN_KOLOGARN) == State_Finished)
    //        menu.addItem(GOSSIP_ICON_CHAT, (526), 5);      // Shattered Walkway

    //    if (pInstance->GetInstanceData(Data_EncounterState, CN_AURIAYA) == State_Finished)
    //        menu.addItem(GOSSIP_ICON_CHAT, (527), 6);      // Conservatory of Life

    //    if (pInstance->GetInstanceData(Data_EncounterState, CN_MIMIRON) == State_Finished)
    //        menu.addItem(GOSSIP_ICON_CHAT, (528), 7);      // Spark of Imagination

    //    if (pInstance->GetInstanceData(Data_EncounterState, CN_GENERAL_VEZAX) == State_Finished)
    //        menu.addItem(GOSSIP_ICON_CHAT, (529), 8);      // Prison of Yogg-Saron

    //    menu.sendGossipPacket(player);
    //}

    //void OnSelectOption(Object* object, Player* player, uint32_t Id, const char* enteredcode, uint32_t gossipId)
    //{
    //    Menu::senGossipComplete(player);

    //    if (Id >= 9)
    //        return;
    //    
    //    switch (Id)
    //    {
    //        case 0:
    //            player->castSpell(player, 64014, true);     // Expedition Base Camp
    //            break;
    //        case 1:
    //            player->castSpell(player, 64032, true);     // Formation Grounds
    //            break;
    //        case 2:
    //            player->castSpell(player, 64028, true);     // Colossal Forge
    //            break;
    //        case 3:
    //            player->castSpell(player, 64031, true);     // Scrapyard
    //            break;
    //        case 4:
    //            player->castSpell(player, 64030, true);     // Antechamber of Ulduar
    //            break;
    //        case 5:
    //            player->castSpell(player, 64029, true);     // Shattered Walkway
    //            break;
    //        case 6:
    //            player->castSpell(player, 64024, true);     // Conservatory of Life
    //            break;
    //        case 7:
    //            player->castSpell(player, 64025, true);     // Spark of Imagination
    //            break;
    //        case 8:
    //            player->castSpell(player, 65042, true);     // Prison of Yogg-Saron
    //            break;
    //    }
    //}
};

class UlduarTeleporterAI : public GameObjectAIScript
{
public:

    explicit UlduarTeleporterAI(GameObject* go) : GameObjectAIScript(go) {}
    ~UlduarTeleporterAI() {}

    static GameObjectAIScript* Create(GameObject* go) { return new UlduarTeleporterAI(go); }

    void OnActivate(Player* /*player*/) override
    {
        /*UlduarTeleporterGossip gossip;
        gossip.OnHello(_gameobject, player);*/
    }
};

void SetupUlduar(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ULDUAR, &UlduarInstanceScript::Create);

    //Teleporter
    mgr->register_gameobject_script(194569, &UlduarTeleporterAI::Create);
    //mgr->register_go_gossip(194569, new UlduarTeleporterGossip());
};
