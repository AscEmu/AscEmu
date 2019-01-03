/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/WorldSession.h"
#include "Management/Gossip/Gossip.h"
#include "Units/Players/Player.h"
#include "Server/Script/ScriptMgr.h"

class ErelasAmbersky_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu::SendQuickMenu(pObject->getGuid(), 2153, plr, 1, GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TELDSASSIL_HIPPO));
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Arcemu::Gossip::Menu::SendSimpleMenu(pObject->getGuid(), 2154, plr);
    }

    void Destroy() override { delete this; }
};

void SetupTeldrassilGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(7916, new ErelasAmbersky_Gossip); // Erelas Ambersky
}
