/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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

class AncientMarks : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        uint32_t entry = pObject->getEntry();
        const char* text = "";
        uint32_t TextId = 0;

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

        if (plr->HasFinishedQuest(9785) || plr->hasQuestInQuestLog(9785))
        {
            GossipMenu menu(pObject->getGuid(), TextId, plr->GetSession()->language);
            menu.addItem(GOSSIP_ICON_CHAT, 0, 1, text);
            menu.sendGossipPacket(plr);
        }
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Creature* casta = static_cast<Creature*>(pObject);
        switch (pObject->getEntry())
        {
            case 17900:
            {
                plr->AddQuestKill(9785, 0, 0);

                if (plr->GetStandingRank(942) == 4)
                    casta->castSpell(plr, 31808, true);
                else if (plr->GetStandingRank(942) == 5)
                    casta->castSpell(plr, 31810, true);
                else if (plr->GetStandingRank(942) == 6)
                    casta->castSpell(plr, 31811, true);
                else if (plr->GetStandingRank(942) == 7)
                    casta->castSpell(plr, 31815, true);

            } break;
            case 17901:
            {
                plr->AddQuestKill(9785, 1, 0);

                if (plr->GetStandingRank(942) == 4)
                    casta->castSpell(plr, 31807, true);
                else if (plr->GetStandingRank(942) == 5)
                    casta->castSpell(plr, 31814, true);
                else if (plr->GetStandingRank(942) == 6)
                    casta->castSpell(plr, 31813, true);
                else if (plr->GetStandingRank(942) == 7)
                    casta->castSpell(plr, 31812, true);

            } break;
        }
    }
};

class ElderKuruti : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        if (!plr->getItemInterface()->GetItemCount(24573, true))
        {
            GossipMenu menu(pObject->getGuid(), 9226, plr->GetSession()->language);
            menu.addItem(GOSSIP_ICON_CHAT, 502, 1);     // Offer treat
            menu.sendGossipPacket(plr);
        }
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                GossipMenu menu(pObject->getGuid(), 9227, plr->GetSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, 503, 2); // I'm a messenger for Draenei
                menu.sendGossipPacket(plr);
            }break;
            case 2:
            {
                GossipMenu menu(pObject->getGuid(), 9229, plr->GetSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, 504, 3); // Get message
                menu.sendGossipPacket(plr);
            }break;
            case 3:
            {
                GossipMenu::sendSimpleMenu(pObject->getGuid(), 9231, plr);

                if (!plr->getItemInterface()->GetItemCount(24573, true))
                    plr->getItemInterface()->AddItemById(24573, 1, 0);

            }break;
        }
    }
};

void SetupZangarmarsh(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(17900, new AncientMarks()); // Ashyen Ancient of Lore
    mgr->register_creature_gossip(17901, new AncientMarks()); // Keleth Ancient of War
    mgr->register_creature_gossip(18197, new ElderKuruti());  //
}
