/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2008 WEmu Team
 * Copyright (C) 2009 WhyScripts Team <http://www.whydb.org/>
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

class Lady_Jaina : public GossipScript
{
    public:

        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            if (plr->HasQuest(558))
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 7012, plr);
                Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(505), 1);     // Lady Jaina, this may sound like an odd request... but I have a young ward who is quite shy. You are a hero to him, and he asked me to get your autograph.
                Menu->SendTo(plr);
            }
        }

        void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char* Code)
        {
            switch (IntId)
            {
                case 0: // Return to start
                    GossipHello(pObject, plr);
                    break;
                case 1: // Give Item
                {
                    plr->CastSpell(plr, dbcSpell.LookupEntry(23122), true);
                    plr->Gossip_Complete();
                    break;
                }
                break;
            }
        }
};

class Cairne : public GossipScript
{
    public:

        void GossipHello(Object* pObject, Player* plr)
        {
            GossipMenu* Menu;
            if (plr->HasQuest(925))
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 7013, plr);
                Menu->AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(506), 1);     // Give me hoofprint.
                Menu->SendTo(plr);
            }
        }

        void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char* Code)
        {
            GossipMenu* Menu;
            switch (IntId)
            {
                case 0: // Return to start
                    GossipHello(pObject, plr);
                    break;
                case 1: // Give Item
                {
                    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 7014, plr);
                    Menu->SendTo(plr);
                    plr->CastSpell(plr, dbcSpell.LookupEntry(23123), true);
                    break;
                }
                break;
            }
        }
};


#define DALARAN_TELEPORT_SPELL 68328

class TeleportQ_Gossip : public GossipScript
{
    public:

        void GossipHello(Object* pObject, Player* plr)
        {
            uint32 Text = objmgr.GetGossipTextForNpc(static_cast<Creature*>(pObject)->GetEntry());

            // check if there is a entry in the db
            if (NpcTextStorage.LookupEntry(Text) == NULL)
                return;

            Arcemu::Gossip::Menu menu(pObject->GetGUID(), Text, plr->GetSession()->language);
            sQuestMgr.FillQuestMenu(static_cast<Creature*>(pObject), plr, menu);

            // Requirements:
            // one of these quests: 12791, 12794, 12796
            // and item 39740: Kirin Tor Signet
            if ((plr->HasQuest(12791) || plr->HasQuest(12794) || plr->HasQuest(12796)) && plr->HasItemCount(39740, 1, false))
            {
                menu.AddItem(ICON_CHAT, plr->GetSession()->LocalizedGossipOption(514), 1);        // Teleport me to Dalaran.
            }
            menu.Send(plr);
        }

        void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char* Code)
        {
            if (IntId == 1)
            {
                plr->CastSpell(plr, DALARAN_TELEPORT_SPELL, true);
            }
            plr->Gossip_Complete();
        }
};

void SetupQuestGossip(ScriptMgr* mgr)
{
    mgr->register_gossip_script(4968, new Lady_Jaina());
    mgr->register_gossip_script(3057, new Cairne());

    // **** Dalaran quests start **** //
    // Horde
    mgr->register_gossip_script(26471, new TeleportQ_Gossip);
    mgr->register_gossip_script(29155, new TeleportQ_Gossip);
    mgr->register_gossip_script(29159, new TeleportQ_Gossip);
    mgr->register_gossip_script(29160, new TeleportQ_Gossip);
    mgr->register_gossip_script(29162, new TeleportQ_Gossip);
    // Alliance
    mgr->register_gossip_script(23729, new TeleportQ_Gossip);
    mgr->register_gossip_script(26673, new TeleportQ_Gossip);
    mgr->register_gossip_script(27158, new TeleportQ_Gossip);
    mgr->register_gossip_script(29158, new TeleportQ_Gossip);
    mgr->register_gossip_script(29161, new TeleportQ_Gossip);
    // Both
    mgr->register_gossip_script(29169, new TeleportQ_Gossip);
}
