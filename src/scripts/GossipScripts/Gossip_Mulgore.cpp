/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/WorldSession.h"
#include "Management/Gossip/GossipScript.h"
#include "Units/Players/Player.h"
#include "Server/Script/ScriptMgr.h"
#include "Management/Gossip/GossipMenu.h"

class SkornWhitecloud_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu::sendQuickMenu(pObject->getGuid(), 522, plr, 1, GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MULGORE_STORY_SKORN));
    }

    void onSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        if(!pObject->isCreature())
            return;
        GossipMenu::sendSimpleMenu(pObject->getGuid(), 523, plr);
    }

    void destroy() override { delete this; }
};

void SetupMulgoreGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(3052, new SkornWhitecloud_Gossip); // Skorn Whitecloud
}
