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
#include "Server/Script/CreatureAIScript.h"

class Quest_The_Ring_of_Blood_The_Final_Challenge : public QuestScript
{
public:
    void OnQuestStart(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* pCreature = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18069);
        if (pCreature != nullptr)
        {
            pCreature->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Prepare yourselves!");
            Unit* Qgiver = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18471);
            if (Qgiver != nullptr)
            {
                Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Mogor has challenged you. You have to accept! Get in the right of blood if you are ready to fight.");
                Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "For the first time in the Ring of Bloods history. Mogor has chosen to exercise his right of the battle! On this wartorn ground, $N will face Mogor, hero of the Warmaul!", 32000);
            }

            pCreature->GetAIInterface()->setAllowedToEnterCombat(true);
            pCreature->GetAIInterface()->moveTo(-704.669f, 7871.08f, 45.0387f);
            pCreature->SetOrientation(1.59531f);
            pCreature->SetFacing(1.908516f);
            pCreature->SetFaction(14);
        }
    }

    void OnQuestComplete(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* pCreature = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18069);
        if (pCreature != nullptr)
            pCreature->Despawn(1000, 0);

        pPlayer->GetMapMgr()->GetInterface()->SpawnCreature(18069, -712.443115f, 7932.182129f, 59.430191f, 4.515952f, true, false, 0, 0);
    }
};

class Quest_The_Ring_of_Blood_The_Warmaul_Champion : public QuestScript
{
public:
    void OnQuestStart(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* pQgiver = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18471);
        if (pQgiver != nullptr)
        {
            pQgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Get in the Ring of Blood, $N. The fight is about to start!");
            pQgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "They had to ship the champion in from the Blade's Edge gladiator pits. He was training on mountain giants - three at a time.", 4000);
            pPlayer->GetMapMgr()->GetInterface()->SpawnCreature(18402, -704.669f, 7871.08f, 45.0387f, 1.59531f, true, false, 0, 0);
        };
    };

    void OnQuestComplete(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* pMogor = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18069);
        if (pMogor != nullptr)
        {
            pMogor->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "WUT!? UNPOSSIBLE!! You fight Mogor now! Mogor destroy!");
        };

        Creature* pWarmaulChamp = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), 0, 18402);
        if (pWarmaulChamp != nullptr)
            pWarmaulChamp->Despawn(1000, 0);
    };
};

class Quest_The_Ring_of_Blood_Skragath : public QuestScript
{
public:
    void OnQuestStart(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* Qgiver = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18471);
        if (Qgiver != nullptr)
        {
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Get in the Ring of Blood, $N. The fight is about to start!");
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "From the parts unknown: Ska'gath! Can $N possibly survive the onslaught of void energies?", 4000);
            pPlayer->GetMapMgr()->GetInterface()->SpawnCreature(18401, -704.669f, 7871.08f, 45.0387f, 1.59531f, true, false, 0, 0);
        };
    };

    void OnQuestComplete(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* mogor = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18069);
        if (mogor != nullptr)
        {
            mogor->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Mogor not impressed! Skra'gat wuz made of da air and shadow! Soft like da squishy orcies!");
        };

        Creature* pSkragath = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), 0, 18401);
        if (pSkragath != nullptr)
            pSkragath->Despawn(1000, 0);
    };
};

class Quest_The_Ring_of_Blood_Rokdar_the_Sundered_Lord : public QuestScript
{
public:
    void OnQuestStart(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* Qgiver = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18471);
        if (Qgiver != nullptr)
        {
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Get in the Ring of Blood, $N. The fight is about to start!");
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Hailing from the mountains of Blade's Edge comes Rokdar the Sundered Lord! $N is in for the fight of his life.", 4000);
            pPlayer->GetMapMgr()->GetInterface()->SpawnCreature(18400, -704.669f, 7871.08f, 45.0387f, 1.59531f, true, false, 0, 0);
        };
    };

    void OnQuestComplete(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* mogor = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18069);
        if (mogor != nullptr)
            mogor->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "WUT!? UNPOSSIBLE!!");

        Creature* pRokdar = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18400);
        if (pRokdar != nullptr)
            pRokdar->Despawn(1000, 0);
    };
};

class Quest_The_Ring_of_Blood_The_Blue_Brothers : public QuestScript
{
public:
    void OnQuestStart(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* pBrokentoe = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18398);
        if (pBrokentoe != nullptr)
            pBrokentoe->Despawn(1000, 0);

        Unit* Qgiver = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18471);
        if (Qgiver != nullptr)
        {
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Get in the Ring of Blood, $N. The fight is about to start!");
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "The battle is about to begin! The unmerciful Murkblood twins versus $N", 4000);
            pPlayer->GetMapMgr()->GetInterface()->SpawnCreature(18399, -704.669f, 7871.08f, 45.0387f, 1.59531f, true, false, 0, 0);
            pPlayer->GetMapMgr()->GetInterface()->SpawnCreature(18399, -708.076f, 7870.41f, 44.8457f, 1.59531f, true, false, 0, 0);
        };
    };

    void OnQuestComplete(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* mogor = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18069);
        if (mogor != nullptr)
        {
            mogor->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "...");
        };

        Creature* pBrother1 = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), 0, 18399);
        if (pBrother1 != nullptr)
            pBrother1->Despawn(1000, 0);

        Creature* pBrother2 = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18399);
        if (pBrother2 != nullptr)
            pBrother2->Despawn(1000, 0);
    };
};

class Quest_The_Ring_of_Blood_Brokentoe : public QuestScript
{
public:
    void OnQuestStart(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* Qgiver = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18471);
        if (Qgiver != nullptr)
        {
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Get in the Ring of Blood, $N. The fight is about to start!");
            pPlayer->GetMapMgr()->GetInterface()->SpawnCreature(18398, -704.669f, 7871.08f, 45.0387f, 1.59531f, true, false, 0, 0)->Despawn(600000, 0);
        };
    };

    void OnQuestComplete(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Unit* Qgiver = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18471);
        if (Qgiver != nullptr)
        {
            Qgiver->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "$N is victorious!");
        };

        Unit* mogor = pPlayer->MAP_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 18069);
        if (mogor != nullptr)
        {
            mogor->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "...");
        };
    };
};

class mogorQAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new mogorQAI(c); }
    explicit mogorQAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
        getCreature()->GetAIInterface()->setAllowedToEnterCombat(false);
    };
};

class NotOnMyWatch : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NotOnMyWatch(c); }
    explicit NotOnMyWatch(Creature* pCreature) : CreatureAIScript(pCreature) {};

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(1000);
        getCreature()->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "In Nagrand, food hunt ogre!");
    };

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        RemoveAIUpdateEvent();
    };

    void OnDied(Unit* /*mTarget*/) override
    {
        RemoveAIUpdateEvent();
    };

    void OnLoad() override
    {
        getCreature()->SetFaction(14);
        RemoveAIUpdateEvent();
    };

    void AIUpdate() override
    {
        if (getCreature()->getHealthPct() < 30)
        {
            Unit* pUnit = getCreature()->GetAIInterface()->getCurrentTarget();
            if (pUnit != nullptr && pUnit->isPlayer())
                static_cast<Player*>(pUnit)->EventAttackStop();

            getCreature()->SetFaction(35);
            getCreature()->getThreatManager().clearAllThreat();
            getCreature()->getThreatManager().removeMeFromThreatLists();
            getCreature()->setStandState(STANDSTATE_SIT);
            getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);

            getCreature()->Despawn(180000, 0);

            RemoveAIUpdateEvent();
        };
    };
};

class LumpGossipScript : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 2, plr->GetSession()->language);
        menu.addItem(GOSSIP_ICON_CHAT, 469, 1);     // Why are Boulderfist out this far? You know this is Kurenai territory!
        menu.sendGossipPacket(plr);
    };

    void onSelectOption(Object* /*pObject*/, Player* plr, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        plr->AddQuestKill(9918, 0, 0);
    }
};

void SetupNagrand(ScriptMgr* mgr)
{
    mgr->register_creature_script(18351, &NotOnMyWatch::Create);
    mgr->register_creature_script(18069, &mogorQAI::Create);

    mgr->register_quest_script(9977, new Quest_The_Ring_of_Blood_The_Final_Challenge());
    mgr->register_quest_script(9973, new Quest_The_Ring_of_Blood_The_Warmaul_Champion());
    mgr->register_quest_script(9972, new Quest_The_Ring_of_Blood_Skragath());
    mgr->register_quest_script(9970, new Quest_The_Ring_of_Blood_Rokdar_the_Sundered_Lord());
    mgr->register_quest_script(9967, new Quest_The_Ring_of_Blood_The_Blue_Brothers());
    mgr->register_quest_script(9962, new Quest_The_Ring_of_Blood_Brokentoe());

    mgr->register_creature_gossip(18351, new LumpGossipScript());
}
