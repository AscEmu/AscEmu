/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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

//enum 
//{
//    // Lunaclaw ghost gossip
//    GOSSIP_GHOST_MOONKIN "You have fought well, spirit. I ask you to grand me the strenght of your body and the strenght of your heart."
//};

class Lunaclaw : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Lunaclaw)
    explicit Lunaclaw(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller)
    {
        if (!mKiller->isPlayer())
            return;

        Player* plr = static_cast<Player*>(mKiller);

        Creature* ct = plr->GetMapMgr()->CreateAndSpawnCreature(12144, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), 0);
        if (ct != nullptr)
            ct->Despawn(1 * 60 * 1000, 0);
    }
};

class MoonkinGhost_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 4714, plr->GetSession()->language);
        if (plr->hasQuestInQuestLog(6002))
            menu.addItem(GOSSIP_ICON_CHAT, 455, 1);     // You have fought well, spirit. I ask you to grand me the strenght of your body and the strenght of your heart.
        else if (plr->hasQuestInQuestLog(6001))
            menu.addItem(GOSSIP_ICON_CHAT, 455, 2);     // You have fought well, spirit. I ask you to grand me the strenght of your body and the strenght of your heart.

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        Creature* pCreature = static_cast<Creature*>(pObject);

        switch (Id)
        {
            case 1: //Horde
            {
                GossipMenu::sendSimpleMenu(pObject->getGuid(), 4715, plr);

                if (auto* questLog = plr->getQuestLogByQuestId(6002))
                {
                    if (questLog->canBeFinished())
                        return;

                    questLog->setStateComplete();
                    questLog->sendQuestComplete();
                    questLog->updatePlayerFields();

                    pCreature->emote(EMOTE_ONESHOT_WAVE);
                    pCreature->Despawn(240000, 0);
                }
            }
            break;

            case 2: //Ally
            {
                GossipMenu::sendSimpleMenu(pObject->getGuid(), 4715, plr);
 
                if (auto* questLog = plr->getQuestLogByQuestId(6001))
                {
                    if (questLog->canBeFinished())
                        return;

                    questLog->setStateComplete();
                    questLog->sendQuestComplete();
                    questLog->updatePlayerFields();

                    pCreature->emote(EMOTE_ONESHOT_WAVE);
                    pCreature->Despawn(240000, 0);
                }
            }
            break;

        }
    }
};

class SCRIPT_DECL BearGhost_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 4719, plr->GetSession()->language);
        if (plr->hasQuestInQuestLog(5930)) // horde
            menu.addItem(GOSSIP_ICON_CHAT, 456, 1);     // What do you represent, spirit?
        else if (plr->hasQuestInQuestLog(5929)) // ally
            menu.addItem(GOSSIP_ICON_CHAT, 456, 5);     // What do you represent, spirit?

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                GossipMenu menu(pObject->getGuid(), 4721, plr->GetSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, 457, 2);     // I seek to understand the importance of strength of the body.
                menu.sendGossipPacket(plr);
                break;
            }
            case 2:
            {
                GossipMenu menu(pObject->getGuid(), 4733, plr->GetSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, 458, 3);     // I seek to understand the importance of strength of the heart.
                menu.sendGossipPacket(plr);
                break;
            }
            case 3:
            {
                GossipMenu menu(pObject->getGuid(), 4734, plr->GetSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, 459, 4);     // I have heard your words, Great Bear Spirit, and I understand. I now...
                menu.sendGossipPacket(plr);
                break;
            }
            case 4:
            {
                GossipMenu::sendSimpleMenu(pObject->getGuid(), 4735, plr);

                if (auto* questLog = plr->getQuestLogByQuestId(5930))
                {
                    if (questLog->canBeFinished())
                        return;

                    questLog->setStateComplete();
                    questLog->sendQuestComplete();
                    questLog->updatePlayerFields();
                }
                break;
            }
            case 5:
            {
                GossipMenu menu(pObject->getGuid(), 4721, plr->GetSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, 457, 6);     // I seek to understand the importance of strength of the body.
                menu.sendGossipPacket(plr);
                break;
            }
            case 6:
            {
                GossipMenu menu(pObject->getGuid(), 4733, plr->GetSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, 458, 7);     // I seek to understand the importance of strength of the heart.
                menu.sendGossipPacket(plr);
                break;
            }
            case 7:
            {
                GossipMenu menu(pObject->getGuid(), 4734, plr->GetSession()->language);
                menu.addItem(GOSSIP_ICON_CHAT, 459, 8);     // I have heard your words, Great Bear Spirit, and I understand. I now...
                menu.sendGossipPacket(plr);
                break;
            }
            case 8:
            {
                GossipMenu::sendSimpleMenu(pObject->getGuid(), 4735, plr);

                if (auto* questLog = plr->getQuestLogByQuestId(5929))
                {
                    if (questLog->canBeFinished())
                        return;

                    questLog->setStateComplete();
                    questLog->sendQuestComplete();
                    questLog->updatePlayerFields();
                }
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
            mTarget->castSpell(mTarget, sSpellMgr.getSpellInfo(19027), true);
    }
};

void SetupDruid(ScriptMgr* mgr)
{
    QuestScript* Moonglade = new MoongladeQuest();
    mgr->register_quest_script(5921, Moonglade);
    mgr->register_quest_script(5922, Moonglade);
    mgr->register_creature_script(12138, &Lunaclaw::Create);

    mgr->register_creature_gossip(12144, new MoonkinGhost_Gossip()); // Ghost of Lunaclaw
    mgr->register_creature_gossip(11956, new BearGhost_Gossip()); // Great Bear Spirit
}
