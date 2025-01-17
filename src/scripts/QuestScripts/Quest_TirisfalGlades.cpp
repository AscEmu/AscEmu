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
#include "Management/QuestLogEntry.hpp"
#include "Management/QuestProperties.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"

class TheDormantShade : public QuestScript
{
public:
    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* creat = mTarget->getWorldMap()->getInterface()->spawnCreature(1946, LocationVector(2467.314f, 14.8471f, 23.5950f), true, false, 0, 0);
        creat->Despawn(60000, 0);
        creat->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You have disturbed my rest. Now face my wrath!");
    }
};

enum Calvin
{
    SAY_COMPLETE                = 10759,
    SPELL_DRINK                 = 7737,
    QUEST_590                   = 590,

    EVENT_EMOTE_RUDE            = 1,
    EVENT_TALK                  = 2,
    EVENT_DRINK                 = 3,
    EVENT_SET_QUESTGIVER_FLAG   = 4,
    EVENT_STAND                 = 5
};

class CalvinMontague : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CalvinMontague(c); }
    explicit CalvinMontague(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setFaction(68);
        getCreature()->setStandState(STANDSTATE_STAND);
    }

    void DamageTaken(Unit* /*_attacker*/, uint32_t* damage) override
    {
        if (getCreature()->getHealthPct() < 15 || *damage > getCreature()->getHealth())
        {
            // Dont let us die
            *damage = 0;

            // Stopp Sttacking Me
            removeCombat();

            // Evade
            getCreature()->getAIInterface()->enterEvadeMode();

            // Outro Event
            scriptEvents.addEvent(EVENT_EMOTE_RUDE, 3000);
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(time_passed, getScriptPhase());

        if (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
                case EVENT_EMOTE_RUDE:
                {
                    getCreature()->emote(EMOTE_ONESHOT_RUDE);
                    scriptEvents.addEvent(EVENT_TALK, 2000);
                } break;
                case EVENT_TALK:
                {
                    sendDBChatMessage(SAY_COMPLETE);
                    scriptEvents.addEvent(EVENT_DRINK, 5000);
                } break;
                case EVENT_DRINK:
                {
                    // Finish the Quest
                    if (Player* plr = getCreature()->getWorldMapPlayer(_playerGuid))
                    {
                        if (auto* questLog = static_cast<Player*>(plr)->getQuestLogByQuestId(QUEST_590))
                            questLog->sendQuestComplete();
                    }
                    _playerGuid = 0;
                    getCreature()->castSpell(nullptr, SPELL_DRINK);
                    scriptEvents.addEvent(EVENT_SET_QUESTGIVER_FLAG, 12000);
                } break;
                case EVENT_SET_QUESTGIVER_FLAG:
                {
                    getCreature()->addNpcFlags(UNIT_NPC_FLAG_QUESTGIVER);
                    scriptEvents.addEvent(EVENT_STAND, 3000);
                } break;
                case EVENT_STAND:
                {
                    getCreature()->setStandState(STANDSTATE_STAND);
                } break;
            }
        }
    }

    void onQuestAccept(Player* player, QuestProperties const* qst) override
    {
        if (qst->id == QUEST_590)
        {
            // setPlayer
            _playerGuid = player->getGuid();

            getCreature()->removeNpcFlags(UNIT_NPC_FLAG_QUESTGIVER);
            getCreature()->setFaction(28);
            getCreature()->getAIInterface()->setMeleeDisabled(false);
            getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
            getCreature()->getAIInterface()->setImmuneToPC(false);
            getCreature()->getAIInterface()->setImmuneToNPC(false);
        }
    }

    void removeCombat()
    {
        getCreature()->getThreatManager().clearAllThreat();
        getCreature()->getThreatManager().removeMeFromThreatLists();
        getCreature()->removeAllNegativeAuras();
        getCreature()->setFaction(68);
        _setMeleeDisabled(true);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->getAIInterface()->setImmuneToPC(true);
        getCreature()->getAIInterface()->setImmuneToNPC(true);
    }

    uint64_t _playerGuid = 0;
};

class Zealot : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Zealot(c); }
    explicit Zealot(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        if (!getCreature()->hasAurasWithId(3287))
            return;
        if (iWaypointId == 2)
        {
            getCreature()->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "My mind. . .me flesh. . .I'm. . .rotting. . . .!");
        }

        if (iWaypointId == 7)
        {
            getCreature()->castSpell(getCreature(), sSpellMgr.getSpellInfo(5), true);
        }
    }
};

class FreshOutOfTheGrave : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        uint32_t rigorMortisSpell = 73523;
        uint32_t ressurrectSpell = 73524;

        // Ressurect our Player
        mTarget->castSpell(mTarget, sSpellMgr.getSpellInfo(ressurrectSpell), true);

        // Remove death Aura
        if (mTarget->hasAurasWithId(rigorMortisSpell))
            mTarget->removeAllAurasById(rigorMortisSpell);
    }
};

void SetupTirisfalGlades(ScriptMgr* mgr)
{
    mgr->register_quest_script(410, new TheDormantShade());
    mgr->register_creature_script(6784, &CalvinMontague::Create);
    mgr->register_creature_script(1931, &Zealot::Create);
    mgr->register_quest_script(24959, new FreshOutOfTheGrave());
}
