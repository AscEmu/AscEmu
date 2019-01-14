/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/WorldSession.h"
#include "Management/Gossip/Gossip.h"
#include "Units/Players/Player.h"
#include "Server/Script/ScriptMgr.h"

class CurgleCranklehop_Gossip : public Arcemu::Gossip::Script
{
public:

    uint32_t definedGossipMenu = 1519;
    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), definedGossipMenu);
        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TANARIS_CRANK_HIPPO), 1);
        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TANARIS_CRANK_GORDUNNI), 2);
        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), 0);
        if (1 == Id)
            menu.setTextID(1521);
        else
            menu.setTextID(1646);
        menu.Send(plr);
    }

    void Destroy() override { delete this; }

};

class TrentonLighthammer_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu::SendQuickMenu(pObject->getGuid(), 1758, plr, 1, GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_TANARIS_TELL_TRENTON));
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), 0);
        menu.setTextID(1759);
        menu.Send(plr);
    }

    void Destroy() override { delete this; }
};

void SetupTanarisGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(7763, new CurgleCranklehop_Gossip);   // Curgle Cranklehop
    mgr->register_creature_gossip(7804, new TrentonLighthammer_Gossip); // Trenton Lighthammer
}
