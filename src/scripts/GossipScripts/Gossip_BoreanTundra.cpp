/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Objects/ObjectMgr.h"
#include <Spell/SpellMgr.h>

enum UnorderedEntry
{
    GT_TIARE = 13022,           // Gossip text "How can I help you, child?"
    CN_TIARE = 30051,           // Tiare
    GI_TELE_AMBER_LEDGE = 350   // "Teleport me to Amber Ledge!"
};

class TiareGossipScript : public GossipScript
{
public:

    void onHello(Object* pObject, Player* Plr) override
    {
        GossipMenu menu(pObject->getGuid(), GT_TIARE, 0);
        menu.addItem(GOSSIP_ICON_CHAT, GI_TELE_AMBER_LEDGE, 1);
        menu.sendGossipPacket(Plr);
    }

    void onSelectOption(Object* pObject, Player* Plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                static_cast<Creature*>(pObject)->castSpell(Plr, sSpellMgr.getSpellInfo(50135), true);
                GossipMenu::senGossipComplete(Plr);
            } break;
        }
    }
};

void SetupBoreanTundraGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(CN_TIARE, new TiareGossipScript());
}
