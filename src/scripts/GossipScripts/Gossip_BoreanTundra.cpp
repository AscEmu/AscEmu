/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Objects/ObjectMgr.h"
#include <Spell/Customization/SpellCustomizations.hpp>

enum UnorderedEntry
{
    GT_TIARE    = 13022,        // Gossip text "How can I help you, child?"
    CN_TIARE    = 30051,        // Tiare
    GI_TELE_AMBER_LEDGE = 350   // "Teleport me to Amber Ledge!"
};

class TiareGossipScript : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* Plr) override
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), GT_TIARE, 0);
            menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_TELE_AMBER_LEDGE), 1);
            menu.Send(Plr);
        }

        void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
        {
            switch (Id)
            {
                case 1:
                {
                    static_cast<Creature*>(pObject)->CastSpell(Plr, sSpellCustomizations.GetSpellInfo(50135), true);
                    Arcemu::Gossip::Menu::Complete(Plr);
                } break;
            }
        }
};

void SetupBoreanTundraGossip(ScriptMgr* mgr)
{
    Arcemu::Gossip::Script* gs = new TiareGossipScript();
    mgr->register_creature_gossip(CN_TIARE, gs);
}
