/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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

#include "../world/Gossip.h"
#include "Setup.h"

class TiareGossipScript : public Arcemu::Gossip::Script
{
    public:
        void OnHello(Object* pObject, Player* Plr)
        {
            Arcemu::Gossip::Menu::SendQuickMenu(pObject->GetGUID(), 1, Plr, 1, 0, Plr->GetSession()->LocalizedGossipOption(GI_TELE_AMBER_LEDGE));
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* EnteredCode)
        {
            Arcemu::Gossip::Menu::Complete(plr);
            TO_CREATURE(pObject)->CastSpell(plr, dbcSpell.LookupEntry(50135), true);
        }
        void Destroy() { delete this; }

};

void SetupBoreanTundraGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(30051, new TiareGossipScript);        // Tiare
}