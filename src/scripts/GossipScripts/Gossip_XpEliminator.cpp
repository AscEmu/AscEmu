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

// XpEleminatorGossip
//  GossipScript subclass for turning on/off Player experience gain
class XpEliminatorGossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 14736);
        if (plr->CanGainXp())
            menu.addItem(GOSSIP_ICON_CHAT, GI_DISABLE_XP_GAIN, 1, "", 100000, plr->GetSession()->LocalizedGossipOption(GI_BOXMSG_DISABLE_XP_GAIN));
        else
            menu.addItem(GOSSIP_ICON_CHAT, GI_ENABLE_XP_GAIN, 1, "", 100000, plr->GetSession()->LocalizedGossipOption(GI_BOXMSG_ENABLE_XP_GAIN));

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* /*pObject*/, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        // turning xp gains on/off costs 10g each time
        if (plr->hasEnoughCoinage(100000))
        {
            plr->modCoinage(-100000);
            plr->ToggleXpGain();
        }
        GossipMenu::senGossipComplete(plr);
    }

    void destroy() override { delete this; }

};

void SetupXpEliminatorGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(35364, new XpEliminatorGossip);    // Slahtzt the Horde NPC
    mgr->register_creature_gossip(35365, new XpEliminatorGossip);    // Besten the Alliance NPC
}
