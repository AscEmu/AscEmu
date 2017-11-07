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
#include <Units/Creatures/Pet.h>

class TheKesselRun : public QuestScript
{
public:

    void OnQuestStart(Player* mTarget, QuestLogEntry* qLogEntry)
    {
        if (!mTarget)
            return;
        if (!mTarget->HasSpell(30829))
            mTarget->CastSpell(mTarget, 30829, true);
    }
};

class TheKesselRun1 : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr)
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 1);
        if (plr->HasQuest(9663))
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(454), 1);     // Warn him

        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* EnteredCode, uint32 gossipId)
    {
        plr->AddQuestKill(9663, 0, 0);
    }
};

class TheKesselRun2 : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr)
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 1);
        if (plr->HasQuest(9663))
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(454), 1);     // Warn him

        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* EnteredCode, uint32 gossipId)
    {
        plr->AddQuestKill(9663, 1, 0);
    }
};

class TheKesselRun3 : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr)
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), 1);
        if (plr->HasQuest(9663))
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(454), 1);     // Warn him

        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* EnteredCode, uint32 gossipId)
    {
        plr->AddQuestKill(9663, 2, 0);
    }
};

class SavingPrincessStillpine : public GameObjectAIScript
{
public:

    SavingPrincessStillpine(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new SavingPrincessStillpine(GO); }

    void OnActivate(Player* pPlayer)
    {
        pPlayer->AddQuestKill(9667, 0, 0);

        Creature* princess = pPlayer->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 17682);
        if (princess != nullptr)
            princess->Despawn(1000, 6 * 60 * 1000);
    }
};

class HighChiefBristlelimb : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(HighChiefBristlelimb);
    HighChiefBristlelimb(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        fulborgskilled = 0;
    }

    void OnDied(Unit* mKiller)
    {
        fulborgskilled++;
        if (mKiller->IsPlayer())
        {
            Player* mPlayer = static_cast<Player*>(mKiller);

            if (fulborgskilled > 8 && mPlayer->HasQuest(9667))
            {
                getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(17702, -2419, -12166, 33, 3.45f, true, false, 0, 0)->Despawn(18000000, 0);
                fulborgskilled = 0;
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Chief, we need your help!");
            }
        }
    }

private:

    int fulborgskilled;
};

class WebbedCreature : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(WebbedCreature);
    WebbedCreature(Creature* pCreature) : CreatureAIScript(pCreature)
    {}

    void OnCombatStart(Unit* pTarget)
    {
        _setMeleeDisabled(true);
        getCreature()->setMoveRoot(true);
        getCreature()->GetAIInterface()->StopMovement(0);
    }

    void OnCombatStop(Unit* pTarget)
    {
        _setMeleeDisabled(false);
        getCreature()->setMoveRoot(false);
    }

    void OnDied(Unit* pKiller)
    {
        Player* QuestHolder = NULL;
        if (pKiller->IsPlayer())
            QuestHolder = static_cast<Player*>(pKiller);
        else if (pKiller->IsPet() && static_cast<Pet*>(pKiller)->GetPetOwner() != NULL)
            QuestHolder = static_cast<Pet*>(pKiller)->GetPetOwner();

        if (QuestHolder == NULL)
            return;

        // M4ksiu: I don't think the method is correct, but it can stay the way it was until someone gives proper infos
        QuestLogEntry* qle = QuestHolder->GetQuestLogForEntry(9670);
        LocationVector pos = getCreature()->GetPosition();
        Creature* RandomCreature = NULL;
        if (qle == nullptr)
        {
            // Creatures from Bloodmyst Isle
            uint32 Id[51] = { 17681, 17887, 17550, 17323, 17338, 17341, 17333, 17340, 17353, 17320, 17339, 17337, 17715, 17322, 17494, 17654, 17342, 17328, 17331, 17325, 17321, 17330, 17522, 17329, 17524, 17327, 17661, 17352, 17334, 17326, 17324, 17673, 17336, 17346, 17589, 17609, 17608, 17345, 17527, 17344, 17347, 17525, 17713, 17523, 17348, 17606, 17604, 17607, 17610, 17358, 17588 };
            RandomCreature = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(Id[RandomUInt(50)], pos.x, pos.y, pos.z, pos.o, true, false, 0, 0);
            if (RandomCreature != NULL)
            {
                RandomCreature->m_noRespawn = true;
                RandomCreature->Despawn(60000, 0);
            }

            return;
        }
        else
        {
            uint32 Id[8] = { 17681, 17321, 17330, 17522, 17673, 17336, 17346, 17589 };
            RandomCreature = getCreature()->GetMapMgr()->GetInterface()->SpawnCreature(Id[RandomUInt(7)], pos.x, pos.y, pos.z, pos.o, true, false, 0, 0);
            if (RandomCreature != NULL)
            {
                RandomCreature->m_noRespawn = true;
                RandomCreature->Despawn(60000, 0);
                if (RandomCreature->GetEntry() == 17681)
                {
                    QuestHolder->AddQuestKill(9670, 0, 0);
                }
            }
        }
    }
};


void SetupBloodmystIsle(ScriptMgr* mgr)
{
    mgr->register_quest_script(9663, new TheKesselRun());
    mgr->register_creature_gossip(17440, new TheKesselRun1());
    mgr->register_creature_gossip(17116, new TheKesselRun2());
    mgr->register_creature_gossip(17240, new TheKesselRun3());

    mgr->register_gameobject_script(181928, &SavingPrincessStillpine::Create);

    mgr->register_creature_script(17320, &HighChiefBristlelimb::Create);
    mgr->register_creature_script(17321, &HighChiefBristlelimb::Create);
    mgr->register_creature_script(17680, &WebbedCreature::Create);
}
