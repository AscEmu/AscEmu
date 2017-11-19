/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2008 WEmu Team
 * Copyright (C) 2005-2008 Ascent Team
 * Copyright (C) 2007-2008 Moon++ Team
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
 /**********************
 Edits by : FenixGman
 **********************/
#include "Setup.h"


class Lunaclaw : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(Lunaclaw);

    Lunaclaw(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller)
    {
        if (!mKiller->IsPlayer())
            return;

        Player* plr = static_cast<Player*>(mKiller);

        Creature* ct = plr->GetMapMgr()->CreateAndSpawnCreature(12144, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), 0);
        if (ct != nullptr)
            ct->Despawn(1 * 60 * 1000, 0);
    }
};

// Lunaclaw ghost gossip
#define GOSSIP_GHOST_MOONKIN    "You have fought well, spirit. I ask you to grand me the strenght of your body and the strenght of your heart."

class MoonkinGhost_Gossip : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 4714, plr->GetSession()->language);
        if (plr->HasQuest(6002))
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(455), 1);     // You have fought well, spirit. I ask you to grand me the strenght of your body and the strenght of your heart.
        else if (plr->HasQuest(6001))
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(455), 2);     // You have fought well, spirit. I ask you to grand me the strenght of your body and the strenght of your heart.

        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Creature* pCreature = static_cast<Creature*>(pObject);

        switch (Id)
        {
            case 1: //Horde
            {
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 4715, plr);

                QuestLogEntry* qle = plr->GetQuestLogForEntry(6002);
                if (qle == nullptr)
                    return;

                if (qle->CanBeFinished())
                    return;

                qle->Complete();
                qle->SendQuestComplete();
                qle->UpdatePlayerFields();

                pCreature->Emote(EMOTE_ONESHOT_WAVE);
                pCreature->Despawn(240000, 0);
            }
            break;

            case 2: //Ally
            {
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 4715, plr);
 
                QuestLogEntry* qle = plr->GetQuestLogForEntry(6001);
                if (qle == nullptr)
                    return;

                if (qle->CanBeFinished())
                    return;

                qle->Complete();
                qle->SendQuestComplete();
                qle->UpdatePlayerFields();

                pCreature->Emote(EMOTE_ONESHOT_WAVE);
                pCreature->Despawn(240000, 0);
            }
            break;

        }
    }
};


class SCRIPT_DECL BearGhost_Gossip : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* plr) override
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 4719, plr->GetSession()->language);
        if (plr->HasQuest(5930)) // horde
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(456), 1);     // What do you represent, spirit?
        else if (plr->HasQuest(5929)) // ally
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(456), 5);     // What do you represent, spirit?

        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 4721, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(457), 2);     // I seek to understand the importance of strength of the body.
                menu.Send(plr);
                break;
            }
            case 2:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 4733, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(458), 3);     // I seek to understand the importance of strength of the heart.
                menu.Send(plr);
                break;
            }
            case 3:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 4734, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(459), 4);     // I have heard your words, Great Bear Spirit, and I understand. I now...
                menu.Send(plr);
                break;
            }
            case 4:
            {
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 4735, plr);

                QuestLogEntry* qle = plr->GetQuestLogForEntry(5930);
                if (qle == nullptr)
                    return;

                if (qle->CanBeFinished())
                    return;

                qle->Complete();
                qle->SendQuestComplete();
                qle->UpdatePlayerFields();
                break;
            }
            case 5:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 4721, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(457), 6);     // I seek to understand the importance of strength of the body.
                menu.Send(plr);
                break;
            }
            case 6:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 4733, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(458), 7);     // I seek to understand the importance of strength of the heart.
                menu.Send(plr);
                break;
            }
            case 7:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 4734, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(459), 8);     // I have heard your words, Great Bear Spirit, and I understand. I now...
                menu.Send(plr);
                break;
            }
            case 8:
            {
                Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 4735, plr);

                QuestLogEntry* qle = plr->GetQuestLogForEntry(5929);
                if (qle == nullptr)
                    return;

                if (qle->CanBeFinished())
                    return;

                qle->Complete();
                qle->SendQuestComplete();
                qle->UpdatePlayerFields();
                break;
            }
        }
    }
};

class MoongladeQuest : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        if (!mTarget->HasSpell(19027))
            mTarget->CastSpell(mTarget, sSpellCustomizations.GetSpellInfo(19027), true);
    }
};


void SetupDruid(ScriptMgr* mgr)
{
    QuestScript* Moonglade = new MoongladeQuest();
    mgr->register_quest_script(5921, Moonglade);
    mgr->register_quest_script(5922, Moonglade);
    mgr->register_creature_script(12138, &Lunaclaw::Create);

    //Register gossip scripts
    Arcemu::Gossip::Script* MoonkinGhostGossip = new MoonkinGhost_Gossip();
    Arcemu::Gossip::Script* BearGhostGossip = new BearGhost_Gossip();

    mgr->register_creature_gossip(12144, MoonkinGhostGossip); // Ghost of Lunaclaw
    mgr->register_creature_gossip(11956, BearGhostGossip); // Great Bear Spirit
}
