/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Spell/SpellAuras.h"

enum
{
    HEMET = 27986,
    HADRIUS = 28047,
    TAMARA = 28568,
};

//////////////////////////////////////////////////////////////////////////////////////////
// Quest 12532 - Flown the Coop!
// Quest 12702 - Chicken Party!

class ChickenEscapee : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ChickenEscapee);
    explicit ChickenEscapee(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad()
    {
        RegisterAIUpdateEvent(1000);
    }

    void AIUpdate()
    {
        // Let's see if we are netted
        Aura* a = getCreature()->getAuraWithId(51959);
        if (a != NULL)
        {
            Unit* Caster = a->GetUnitCaster();
            if (Caster == nullptr)
                return;
            if (Caster->isPlayer())
            {

                QuestLogEntry* qle = static_cast<Player*>(Caster)->GetQuestLogForEntry(12532);
                if (qle == NULL)
                    qle = static_cast<Player*>(Caster)->GetQuestLogForEntry(12702);

                if (qle != NULL)
                {
                    // casting the spell that will create the item for the player
                    getCreature()->castSpell(Caster, 51037, true);
                    getCreature()->Despawn(1000, 360000);
                }
            }
        }
    }
};

class SCRIPT_DECL HemetTasteTest : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        if (plr->HasQuest(12645))
        {
            Arcemu::Gossip::Menu menu(pObject->getGuid(), 40002, plr->GetSession()->language);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(476), 1);     // Care to try Grimbooze Thunderbrew's Jungle punch?
            menu.Send(plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Creature* pCreature = static_cast<Creature*>(pObject);

        std::string msg = "Aye, I'll try it.";
        pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg.c_str());
        std::string msg2 = "That's exactly what I needed!";
        std::string msg3 = "It's got my vote! That'll put hair on your chest like nothing else will.";
        pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg2.c_str(), 2000);
        pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg3.c_str(), 4000);

        QuestLogEntry* qle = plr->GetQuestLogForEntry(12645);
        if (qle == nullptr)
            return;

        if (qle->GetMobCount(0) != 0)
            return;

        qle->SetMobCount(0, 1);
        qle->SendUpdateAddKill(0);
        qle->UpdatePlayerFields();
    }
};

class SCRIPT_DECL HadriusTasteTest : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        if (plr->HasQuest(12645))
        {
            Arcemu::Gossip::Menu menu(pObject->getGuid(), 40002, plr->GetSession()->language);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(476), 1);     // Care to try Grimbooze Thunderbrew's Jungle punch?
            menu.Send(plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Creature* pCreature = static_cast<Creature*>(pObject);

        std::string msg = "I'm always up for something of Grimbooze's.";
        pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg.c_str());
        std::string msg2 = "Well, so far, it tastes like something my wife would drink...";
        std::string msg3 = "Now, there's the kick I've come to expect from Grimbooze's drinks! I like it!";
        pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg2.c_str(), 2000);
        pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg3.c_str(), 4000);
        QuestLogEntry* qle = plr->GetQuestLogForEntry(12645);
        if (qle == nullptr)
            return;

        if (qle->GetMobCount(0) != 0)
            return;

        qle->SetMobCount(1, 1);
        qle->SendUpdateAddKill(1);
        qle->UpdatePlayerFields();
    }
};

class SCRIPT_DECL TamaraTasteTest : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        if (plr->HasQuest(12645))
        {
            Arcemu::Gossip::Menu menu(pObject->getGuid(), 40002, plr->GetSession()->language);
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(476), 1);     // Care to try Grimbooze Thunderbrew's Jungle punch?
            menu.Send(plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 /*Id*/, const char* /*Code*/, uint32 /*gossipId*/) override
    {
        Creature* pCreature = static_cast<Creature*>(pObject);

        std::string msg = " Sure!";
        pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg.c_str());
        std::string msg2 = "Oh my...";
        std::string msg3 = "Tastes like I'm drinking... engine degreaser!";
        pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg2.c_str(), 2000);
        pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg3.c_str(), 4000);

        QuestLogEntry* qle = plr->GetQuestLogForEntry(12645);
        if (qle == nullptr)
            return;

        if (qle->GetMobCount(0) != 0)
            return;

        qle->SetMobCount(2, 1);
        qle->SendUpdateAddKill(2);
        qle->UpdatePlayerFields();
    }
};

void SetupSholazarBasin(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(HEMET, new HemetTasteTest());
    mgr->register_creature_gossip(HADRIUS, new HadriusTasteTest());
    mgr->register_creature_gossip(TAMARA, new TamaraTasteTest());

    mgr->register_creature_script(28161, &ChickenEscapee::Create);
}