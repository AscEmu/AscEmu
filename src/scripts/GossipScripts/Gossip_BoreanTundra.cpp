/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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

#include "Management/Gossip/Gossip.h"
#include "Setup.h"

enum UnorderedEntry
{
    GT_TIARE    = 13022,        // Gossip text "How can I help you, child?"
    CN_TIARE    = 30051,        // Tiare
    GI_TELE_AMBER_LEDGE = 350   // "Teleport me to Amber Ledge!"
};

class TiareGossipScript : public GossipScript
{
    public:

        void GossipHello(Object* pObject, Player* Plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), GT_TIARE, Plr);
            Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(GI_TELE_AMBER_LEDGE), 1);
            Menu->SendTo(Plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            static_cast<Creature*>(pObject)->CastSpell(Plr, sSpellCustomizations.GetSpellInfo(50135), true);
        }
};

void SetupBoreanTundraGossip(ScriptMgr* mgr)
{
    mgr->register_gossip_script(CN_TIARE, new TiareGossipScript);
}
