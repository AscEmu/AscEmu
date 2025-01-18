/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Server/Script/QuestScript.hpp"
#include "Utilities/Random.hpp"

class TheKesselRun : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        if (!mTarget)
            return;
        if (!mTarget->hasSpell(30829))
            mTarget->castSpell(mTarget, 30829, true);
    }
};

class TheKesselRun1 : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 1);
        if (plr->hasQuestInQuestLog(9663))
            menu.addItem(GOSSIP_ICON_CHAT, 454, 1);     // Warn him

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* /*pObject*/, Player* plr, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        plr->addQuestKill(9663, 0, 0);
    }
};

class TheKesselRun2 : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 1);
        if (plr->hasQuestInQuestLog(9663))
            menu.addItem(GOSSIP_ICON_CHAT, 454, 1);     // Warn him

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* /*pObject*/, Player* plr, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        plr->addQuestKill(9663, 1, 0);
    }
};

class TheKesselRun3 : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 1);
        if (plr->hasQuestInQuestLog(9663))
            menu.addItem(GOSSIP_ICON_CHAT, 454, 1);     // Warn him

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* /*pObject*/, Player* plr, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        plr->addQuestKill(9663, 2, 0);
    }
};

class SavingPrincessStillpine : public GameObjectAIScript
{
public:
    explicit SavingPrincessStillpine(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new SavingPrincessStillpine(GO); }

    void OnActivate(Player* pPlayer) override
    {
        pPlayer->addQuestKill(9667, 0, 0);

        Creature* princess = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 17682);
        if (princess != nullptr)
            princess->Despawn(1000, 6 * 60 * 1000);
    }
};

class HighChiefBristlelimb : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HighChiefBristlelimb(c); }
    explicit HighChiefBristlelimb(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        fulborgskilled = 0;
    }

    void OnDied(Unit* mKiller) override
    {
        fulborgskilled++;
        if (mKiller->isPlayer())
        {
            Player* mPlayer = static_cast<Player*>(mKiller);

            if (fulborgskilled > 8 && mPlayer->hasQuestInQuestLog(9667))
            {
                getCreature()->getWorldMap()->getInterface()->spawnCreature(17702, LocationVector(-2419, -12166, 33, 3.45f), true, false, 0, 0)->Despawn(18000000, 0);
                fulborgskilled = 0;
                getCreature()->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Chief, we need your help!");
            }
        }
    }

private:
    int fulborgskilled;
};

class WebbedCreature : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WebbedCreature(c); }
    explicit WebbedCreature(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        _setMeleeDisabled(true);
        getCreature()->setMoveRoot(true);
        getCreature()->stopMoving();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        _setMeleeDisabled(false);
        getCreature()->setMoveRoot(false);
    }

    void OnDied(Unit* pKiller) override
    {
        Player* QuestHolder = pKiller->getPlayerOwnerOrSelf();
        if (QuestHolder == nullptr)
            return;

        LocationVector pos = getCreature()->GetPosition();
        if (!QuestHolder->hasQuestInQuestLog(9670))
        {
            // Creatures from Bloodmyst Isle
            uint32_t Id[51] = { 17681, 17887, 17550, 17323, 17338, 17341, 17333, 17340, 17353, 17320, 17339, 17337, 17715, 17322, 17494, 17654, 17342, 17328, 17331, 17325, 17321, 17330, 17522, 17329, 17524, 17327, 17661, 17352, 17334, 17326, 17324, 17673, 17336, 17346, 17589, 17609, 17608, 17345, 17527, 17344, 17347, 17525, 17713, 17523, 17348, 17606, 17604, 17607, 17610, 17358, 17588 };
            Creature* RandomCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(Id[Util::getRandomUInt(50)], pos, true, false, 0, 0);
            if (RandomCreature != nullptr)
            {
                RandomCreature->m_noRespawn = true;
                RandomCreature->Despawn(60000, 0);
            }
        }
        else
        {
            uint32_t Id[8] = { 17681, 17321, 17330, 17522, 17673, 17336, 17346, 17589 };
            Creature* RandomCreature = getCreature()->getWorldMap()->getInterface()->spawnCreature(Id[Util::getRandomUInt(7)], pos, true, false, 0, 0);
            if (RandomCreature != nullptr)
            {
                RandomCreature->m_noRespawn = true;
                RandomCreature->Despawn(60000, 0);
                if (RandomCreature->getEntry() == 17681)
                {
                    QuestHolder->addQuestKill(9670, 0, 0);
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
