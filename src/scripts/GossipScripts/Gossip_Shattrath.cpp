/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Storage/WorldStrings.h"
#include "Server/WorldSession.h"
#include "Spell/SpellMgr.h"
#include "Units/Creatures/Creature.h"
#include "Management/Gossip/GossipScript.h"
#include "Server/Script/ScriptMgr.h"
#include "Management/Gossip/GossipMenu.h"

class ZephyrGossipScript : public GossipScript
{
public:

    void onHello(Object* pObject, Player* Plr) override
    {
        GossipMenu::sendQuickMenu(pObject->getGuid(), 1, Plr, 1, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_SHATT_ZEPH_COT));
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        if (plr->GetStanding(989) >= 21000)
        //plr->SafeTeleport( 1, 0, -8170.441406f, -4751.321777f, 33.457771f, 5.136f);
            static_cast<Creature*>(pObject)->castSpell(plr, sSpellMgr.getSpellInfo(37778), true);
        else
            plr->BroadcastMessage(plr->GetSession()->LocalizedWorldSrv(ServerString::SHATT_ZEPH_KOT)); // Dunno what the correct text is ^^
        GossipMenu::senGossipComplete(plr);
    }

    void destroy() override { delete this; }
};

void SetupShattrathGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(25967, new ZephyrGossipScript);  // Zephyr
}
