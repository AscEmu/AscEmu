/*
 * Yelly Database Scripts
 * Copyright (C) 2006-2008 U2 Team <http://www.undzwei.eu/>
 * Copyright (C) 2007-2008 Yelly Team
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

 // quest #8304 - Dearest Natalia
class DearestNatalia1 : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* Plr) override
    {
        QuestLogEntry* en = Plr->GetQuestLogForEntry(8304);

        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7736, Plr->GetSession()->language);

        if (en && en->GetMobCount(1) < en->GetQuest()->required_mob_or_go_count[1])
            menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(477), 3);     // Hello, Rutgar. The Commander has sent me here to gather some information about his missing wife.

        menu.Send(Plr);
    }

    void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 3:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7755, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(478), 4);     // That sounds dangerous.
                menu.Send(Plr);
            }break;
            case 4:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7756, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(479), 5);     // What happened to her after that?
                menu.Send(Plr);
            }break;
            case 5:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7757, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(480), 6);     // Natalia?
                menu.Send(Plr);
            }break;
            case 6:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7758, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(481), 7);     // What demands?
                menu.Send(Plr);
            }break;
            case 7:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7759, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(482), 8);     // Lost it? What do you mean?
                menu.Send(Plr);
            }break;
            case 8:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7760, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(483), 9);     // Possessed by what?
                menu.Send(Plr);
            }break;
            case 9:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7761, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(484), 10);     // I'll be back once I straighten this mess out.
                menu.Send(Plr);
            }break;
            case 10:
            {
                Plr->AddQuestKill(8304, 0, 0);
            }break;
        }
    }
};

class DearestNatalia2 : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* Plr) override
    {
        QuestLogEntry* en = Plr->GetQuestLogForEntry(8304);
        if (en && (en->GetMobCount(0) < en->GetQuest()->required_mob_or_go_count[0]) && (en->GetMobCount(1) == 1))
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7735, Plr->GetSession()->language);
            menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(485), 3);   // Hello, Frankal. I've heard that you might have some information as to the whe
            menu.Send(Plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* Plr, uint32 IntId, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (IntId)
        {
            case 3:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7762, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(508), 2);      // That's what I like to hear.
                menu.Send(Plr);
            }break;
            case 4:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7763, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(487), 5);      // That's odd.
                menu.Send(Plr);
            }break;
            case 5:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7764, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(488), 6);      // You couldn't handle a lone night elf priestess?
                menu.Send(Plr);
            }break;
            case 6:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7765, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(489), 7);      // I've been meaning to ask you about that monkey.
                menu.Send(Plr);
            }break;
            case 7:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7766, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(490), 8);      // Then what?
                menu.Send(Plr);
            }break;
            case 8:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7767, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(491), 9);      // What a story! So she went into Hive'Regal and that was the last you saw of her?
                menu.Send(Plr);
            }break;
            case 9:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 7768, Plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(492), 10);      // Thanks for the information, Frankal.
                menu.Send(Plr);
            }break;
            case 10:
            {
                Plr->AddQuestKill(8304, 0, 0);
            }break;
        }
    }
};


class highlord_demitrianGossip : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* pPlayer) override
    {
        //Send quests and gossip menu.
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 6812, pPlayer->GetSession()->language);
        sQuestMgr.FillQuestMenu(static_cast<Creature*>(pObject), pPlayer, menu);
        if (pPlayer->HasItemCount(18563, 1, false) && pPlayer->HasItemCount(18564, 1, false))
        {
            if (pPlayer->HasItemCount(19016, 0, false))
            {
                menu.AddItem(GOSSIP_ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(507), 1);  // What do you know of it
            }
        }
        menu.Send(pPlayer);
    };

    void OnSelectOption(Object* object, Player* player, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                Arcemu::Gossip::Menu menu(object->GetGUID(), 6842, player->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(508), 2);      // I am listening, Demitrian.
                menu.Send(player);
            } break;
            case 2:
            {
                Arcemu::Gossip::Menu menu(object->GetGUID(), 6843, player->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(509), 3);        // Continue, please.
                menu.Send(player);
            } break;
            case 3:
            {
                Arcemu::Gossip::Menu menu(object->GetGUID(), 6844, player->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(510), 4);        // A battle?
                menu.Send(player);
            } break;
            case 4:
            {
                Arcemu::Gossip::Menu menu(object->GetGUID(), 6867, player->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(511), 5);        // <Nod>
                menu.Send(player);
            } break;
            case 5:
            {
                Arcemu::Gossip::Menu menu(object->GetGUID(), 6868, player->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(512), 6);        // Caught unaware? How?
                menu.Send(player);
            } break;
            case 6:
            {
                Arcemu::Gossip::Menu menu(object->GetGUID(), 6869, player->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, player->GetSession()->LocalizedGossipOption(513), 7);        // Oh what did Ragnaros do next?
                menu.Send(player);
            } break;
            case 7:
            {
                player->GetItemInterface()->AddItemById(19016, 1, 0);
                Arcemu::Gossip::Menu::Complete(player);
            } break;
        }
    }
};

class Thunderan : public QuestScript
{
public:
    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        mTarget->GetMapMgr()->CreateAndSpawnCreature(14435, -6241.0f, 1715.0f, 4.8f, 0.605017f);
    }
};


void SetupSilithus(ScriptMgr* mgr)
{
    Arcemu::Gossip::Script* dearestNatalia1 = new DearestNatalia1();
    Arcemu::Gossip::Script* dearestNatalia2 = new DearestNatalia2();
    mgr->register_creature_gossip(15170, dearestNatalia1);
    mgr->register_creature_gossip(15171, dearestNatalia2);
    mgr->register_creature_gossip(14347, new highlord_demitrianGossip);

    mgr->register_quest_script(7786, new Thunderan);
}
