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

class ErelasAmbersky_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu::sendQuickMenu(pObject->getGuid(), 2153, plr, 1, GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELDSASSIL_HIPPO));
    }

    void onSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        GossipMenu::sendSimpleMenu(pObject->getGuid(), 2154, plr);
    }

    void destroy() override { delete this; }
};

void SetupTeldrassilGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(7916, new ErelasAmbersky_Gossip); // Erelas Ambersky
}
