/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Objects/GameObject.h"

enum UnorderedEntrys
{
    GO_DEDICATION_OF_HONOR = 202443,
    GT_DEDICATION_OF_HONOR = 15921,    // "Dedicated to those that fell to the Scourge during the war in the frozen wastes."
    GI_SEE_FALL_LICH_KING = 351        // "See the fall of the Lich King."
};

class DedicationOfHonorGossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* object, Player* player) override
    {
        Arcemu::Gossip::Menu::SendQuickMenu(object->GetGUID(), GT_DEDICATION_OF_HONOR, player, 1, GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(GI_SEE_FALL_LICH_KING));
    }

    void OnSelectOption(Object* /*object*/, Player* player, uint32_t /*id*/, const char* /*enteredCode*/, uint32_t /*gossipId*/) override
    {
        player->sendMovie(16);
        Arcemu::Gossip::Menu::Complete(player);
    }
};

class DedicationOfHonorAI : public GameObjectAIScript
{
public:

    DedicationOfHonorAI(GameObject* go) : GameObjectAIScript(go) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new DedicationOfHonorAI(GO); };

    void OnActivate(Player* player)
    {
        DedicationOfHonorGossip gossip;
        gossip.OnHello(_gameobject, player);
    }
};


void SetupDalaranGossip(ScriptMgr* mgr)
{
    mgr->register_gameobject_script(GO_DEDICATION_OF_HONOR, &DedicationOfHonorAI::Create);
    mgr->register_go_gossip(GO_DEDICATION_OF_HONOR, new DedicationOfHonorGossip);
}
