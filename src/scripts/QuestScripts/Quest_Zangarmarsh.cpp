/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
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

class AncientMarks : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* plr) override
    {
        uint32 entry = pObject->GetEntry();
        const char* text = "";
        uint32 TextId = 0;

        if (entry == 17900)
        {
            text = "Grant me your mark, wise ancient.";
            TextId = 9176;
        }
        else if (entry == 17901)
        {
            text = "Grant me your mark, mighty ancient.";
            TextId = 9177;
        }

        if (plr->HasFinishedQuest(9785) || plr->HasQuest(9785))
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), TextId, plr->GetSession()->language);
            menu.AddItem(GOSSIP_ICON_CHAT, text, 1);
            menu.Send(plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Creature* casta = (static_cast<Creature*>(pObject));
        switch (pObject->GetEntry())
        {
            case 17900:
            {
                plr->AddQuestKill(9785, 0, 0);

                if (plr->GetStandingRank(942) == 4)
                    casta->CastSpell(plr, 31808, true);
                else if (plr->GetStandingRank(942) == 5)
                    casta->CastSpell(plr, 31810, true);
                else if (plr->GetStandingRank(942) == 6)
                    casta->CastSpell(plr, 31811, true);
                else if (plr->GetStandingRank(942) == 7)
                    casta->CastSpell(plr, 31815, true);

            } break;
            case 17901:
            {
                plr->AddQuestKill(9785, 1, 0);

                if (plr->GetStandingRank(942) == 4)
                    casta->CastSpell(plr, 31807, true);
                else if (plr->GetStandingRank(942) == 5)
                    casta->CastSpell(plr, 31814, true);
                else if (plr->GetStandingRank(942) == 6)
                    casta->CastSpell(plr, 31813, true);
                else if (plr->GetStandingRank(942) == 7)
                    casta->CastSpell(plr, 31812, true);

            } break;
        }
    }
};

class ElderKuruti : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* plr) override
    {
        if (!plr->GetItemInterface()->GetItemCount(24573, true))
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 9226, plr->GetSession()->language);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(502), 1);     // Offer treat
            menu.Send(plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 9227, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(503), 2);     // I'm a messenger for Draenei
                menu.Send(plr);
            }break;
            case 2:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 9229, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(504), 3);     // Get message
                menu.Send(plr);
            }break;
            case 3:
            {
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 9231, plr);

                if (!plr->GetItemInterface()->GetItemCount(24573, true))
                    plr->GetItemInterface()->AddItemById(24573, 1, 0);

            }break;
        }
    }
};


void SetupZangarmarsh(ScriptMgr* mgr)
{
    Arcemu::Gossip::Script* AMark = new AncientMarks();
    mgr->register_creature_gossip(17900, AMark);    // Ashyen Ancient of Lore
    mgr->register_creature_gossip(17901, AMark);    // Keleth Ancient of War

    mgr->register_creature_gossip(18197, new ElderKuruti());
}
