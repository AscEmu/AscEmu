/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2008 WEmu Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"

class Veronia : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            if(plr->HasQuest(10652))
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 1, plr);
                Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(470), 1);     // I'm ready
                Menu->SendTo(plr);
            }
        }

        void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char* Code)
        {
            Creature* creat = static_cast<Creature*>(pObject);
            switch(IntId)
            {
                case 1:
                    creat->CastSpell(plr, dbcSpell.LookupEntry(34905), true);
                    break;
            }
        }

};



void SetupNetherstorm(ScriptMgr* mgr)
{
    mgr->register_gossip_script(20162, new Veronia());

}
