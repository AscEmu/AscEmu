/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

class TiareGossipScript : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* Plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->getGuid(), GT_TIARE, 0);
        menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_TELE_AMBER_LEDGE), 1);
        menu.Send(Plr);
    }

    void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                static_cast<Creature*>(pObject)->castSpell(Plr, sSpellMgr.getSpellInfo(50135), true);
                Arcemu::Gossip::Menu::Complete(Plr);
            } break;
        }
    }
};

void SetupBoreanTundraGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(CN_TIARE, new TiareGossipScript());
}
