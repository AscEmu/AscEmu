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

class CurgleCranklehop_Gossip : public GossipScript
{
public:

    uint32_t definedGossipMenu = 1519;
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), definedGossipMenu);
        menu.addItem(GOSSIP_ICON_CHAT, GI_TANARIS_CRANK_HIPPO, 1);
        menu.addItem(GOSSIP_ICON_CHAT, GI_TANARIS_CRANK_GORDUNNI, 2);
        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        GossipMenu menu(pObject->getGuid(), 0);
        if (1 == Id)
            menu.setTextID(1521);
        else
            menu.setTextID(1646);
        menu.sendGossipPacket(plr);
    }

    void destroy() override { delete this; }

};

class TrentonLighthammer_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu::sendQuickMenu(pObject->getGuid(), 1758, plr, 1, GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TANARIS_TELL_TRENTON));
    }

    void onSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        GossipMenu menu(pObject->getGuid(), 0);
        menu.setTextID(1759);
        menu.sendGossipPacket(plr);
    }

    void destroy() override { delete this; }
};

void SetupTanarisGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(7763, new CurgleCranklehop_Gossip);   // Curgle Cranklehop
    mgr->register_creature_gossip(7804, new TrentonLighthammer_Gossip); // Trenton Lighthammer
}
