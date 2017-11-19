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

class BeatenCorpse : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* plr) override
    {
        if (plr->HasQuest(4921))
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 3557, plr->GetSession()->language);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(498), 1);     // I inspect the body further.
            menu.Send(plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Arcemu::Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), 3558, plr);

        QuestLogEntry* qle = plr->GetQuestLogForEntry(4921);
        if (qle == nullptr)
            return;

        if (qle->GetMobCount(0) != 0)
            return;

        qle->SetMobCount(0, 1);
        qle->SendUpdateAddKill(0);
        qle->UpdatePlayerFields();
    }
};

class Wizzlecranks_Shredder : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(Wizzlecranks_Shredder);
    Wizzlecranks_Shredder(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == 195)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Thank you Young warior!");
            getCreature()->Despawn(5000, 1000);
            getCreature()->DeleteWaypoints();
            if (getCreature()->m_escorter == NULL)
                return;
            Player* plr = getCreature()->m_escorter;
            getCreature()->m_escorter = NULL;

            auto quest_entry = plr->GetQuestLogForEntry(863);
            if (quest_entry == nullptr)
                return;
            quest_entry->SendQuestComplete();
        }
    }
};

class Gilthares_Firebough : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(Gilthares_Firebough);
    Gilthares_Firebough(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == 100)
        {
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Finally, I am rescued");
            getCreature()->Despawn(5000, 1000);
            getCreature()->DeleteWaypoints();
            if (getCreature()->m_escorter == NULL)
                return;
            Player* plr = getCreature()->m_escorter;
            getCreature()->m_escorter = NULL;

            auto quest_entry = plr->GetQuestLogForEntry(898);
            if (quest_entry == nullptr)
                return;
            quest_entry->SendQuestComplete();
        }
    }
};

int kolkarskilled = 0;
class VerogtheDervish : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(VerogtheDervish);
    VerogtheDervish(Creature* pCreature) : CreatureAIScript(pCreature) {}
    void OnDied(Unit* mKiller) override
    {
        kolkarskilled++;
        if (mKiller->IsPlayer())
        {
            Player* mPlayer = static_cast<Player*>(mKiller);

            if (kolkarskilled > 8 && mPlayer->HasQuest(851))
            {
                getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(3395, -1209.8f, -2729.84f, 106.33f, 4.8f, true, false, 0, 0)->Despawn(600000, 0);
                kolkarskilled = 0;
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "I am slain! Summon Verog!");
            }
        }
    }

};

void SetupBarrens(ScriptMgr* mgr)
{
    Arcemu::Gossip::Script* gos = new BeatenCorpse();
    mgr->register_creature_gossip(10668, gos);

    mgr->register_creature_script(3439, &Wizzlecranks_Shredder::Create);
    mgr->register_creature_script(3465, &Gilthares_Firebough::Create);
    mgr->register_creature_script(3275, &VerogtheDervish::Create);
    mgr->register_creature_script(3274, &VerogtheDervish::Create);
    mgr->register_creature_script(3397, &VerogtheDervish::Create);
    mgr->register_creature_script(4316, &VerogtheDervish::Create);
}
