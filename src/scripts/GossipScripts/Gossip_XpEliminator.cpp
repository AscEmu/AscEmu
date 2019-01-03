/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/WorldSession.h"
#include "Management/Gossip/Gossip.h"
#include "Units/Players/Player.h"
#include "Server/Script/ScriptMgr.h"

// XpEleminatorGossip
//  GossipScript subclass for turning on/off Player experience gain
class XpEliminatorGossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), 14736);
        if (plr->CanGainXp())
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_DISABLE_XP_GAIN), 1, 100000, plr->GetSession()->LocalizedGossipOption(GI_BOXMSG_DISABLE_XP_GAIN));
        else
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(GI_ENABLE_XP_GAIN), 1, 100000, plr->GetSession()->LocalizedGossipOption(GI_BOXMSG_ENABLE_XP_GAIN));

        menu.Send(plr);
    }

    void OnSelectOption(Object* /*pObject*/, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        // turning xp gains on/off costs 10g each time
        if (plr->hasEnoughCoinage(100000))
        {
            plr->modCoinage(-100000);
            plr->ToggleXpGain();
        }
        Arcemu::Gossip::Menu::Complete(plr);
    }

    void Destroy() override { delete this; }

};

void SetupXpEliminatorGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(35364, new XpEliminatorGossip);    // Slahtzt the Horde NPC
    mgr->register_creature_gossip(35365, new XpEliminatorGossip);    // Besten the Alliance NPC
}
