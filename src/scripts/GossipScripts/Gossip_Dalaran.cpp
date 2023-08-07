/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/Gossip/GossipMenu.hpp"
#include "Server/WorldSession.h"

enum UnorderedEntrys
{
    GO_DEDICATION_OF_HONOR = 202443,
    GT_DEDICATION_OF_HONOR = 15921,    // "Dedicated to those that fell to the Scourge during the war in the frozen wastes."
    GI_SEE_FALL_LICH_KING = 351        // "See the fall of the Lich King."
};

class DedicationOfHonorGossip : public GossipScript
{
public:
    void onHello(Object* object, Player* player) override
    {
        GossipMenu::sendQuickMenu(object->getGuid(), GT_DEDICATION_OF_HONOR, player, 1, GOSSIP_ICON_CHAT, player->getSession()->LocalizedGossipOption(GI_SEE_FALL_LICH_KING));
    }

    void onSelectOption(Object* /*object*/, Player* player, uint32_t /*id*/, const char* /*enteredCode*/, uint32_t /*gossipId*/) override
    {
        player->sendMovie(16);
        GossipMenu::senGossipComplete(player);
    }
};

class DedicationOfHonorAI : public GameObjectAIScript
{
public:
    explicit DedicationOfHonorAI(GameObject* go) : GameObjectAIScript(go) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new DedicationOfHonorAI(GO); };

    void OnActivate(Player* player) override
    {
        DedicationOfHonorGossip gossip;
        gossip.onHello(_gameobject, player);
    }
};

void SetupDalaranGossip(ScriptMgr* mgr)
{
    mgr->register_gameobject_script(GO_DEDICATION_OF_HONOR, &DedicationOfHonorAI::Create);
    mgr->register_go_gossip(GO_DEDICATION_OF_HONOR, new DedicationOfHonorGossip);
}
