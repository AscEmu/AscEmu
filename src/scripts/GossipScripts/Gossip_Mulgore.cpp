/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/WorldSession.h"
#include "Management/Gossip/Gossip.h"
#include "Units/Players/Player.h"
#include "Server/Script/ScriptMgr.h"

class SkornWhitecloud_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu::SendQuickMenu(pObject->getGuid(), 522, plr, 1, GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_MULGORE_STORY_SKORN));
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        if(!pObject->isCreature())
            return;
        Arcemu::Gossip::Menu::SendSimpleMenu(pObject->getGuid(), 523, plr);
    }

    void Destroy() override { delete this; }
};

void SetupMulgoreGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(3052, new SkornWhitecloud_Gossip); // Skorn Whitecloud
}
