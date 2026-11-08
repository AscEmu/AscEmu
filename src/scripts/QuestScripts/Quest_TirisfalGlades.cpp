/*
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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
#include "Movement/MovementManager.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/ClientProtocol.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/Random.hpp"

class TheDormantShade : public QuestScript
{
public:
    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* creat = mTarget->getWorldMap()->getInterface()->spawnCreature(1946, LocationVector(2467.314f, 14.8471f, 23.5950f));
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
            getCreature()->getAIInterface()->setIgnorePlayerCombat(false);
            getCreature()->getAIInterface()->setIgnoreCreatureCombat(false);
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
        getCreature()->getAIInterface()->setIgnorePlayerCombat(true);
        getCreature()->getAIInterface()->setIgnoreCreatureCombat(true);
    }

    WoWGuid _playerGuid;
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

//////////////////////////////////////////////////////////////////////////////////////////
// Deathknell graves (4.x): the Val'kyr Aradne circles the graves and raises the dead
namespace Deathknell
{
    enum
    {
        NPC_ARADNE              = 50372,
        NPC_GRAVE_TARGET        = 50373,
        NPC_RISEN_DEAD          = 50374,
        NPC_MINDLESS_ZOMBIE     = 1501,

        SPELL_RAISE_UNDEAD      = 93446,    // Aradne on the grave target
        SPELL_RISEN_DEAD        = 93447,    // the grave target summons the risen dead
        SPELL_ENRAGE            = 44427,

        DATA_RAISE              = 1,
        PATH_ARADNE             = 1,
        PATH_RISEN              = 1
    };

    static const LocationVector aradnePath[] =
    {
        { 1698.53f, 1687.82f, 134.825f }, { 1696.90f, 1688.16f, 135.093f }, { 1696.82f, 1680.22f, 134.648f },
        { 1695.54f, 1680.51f, 134.857f }, { 1694.45f, 1675.17f, 134.582f }, { 1691.49f, 1672.14f, 134.725f },
        { 1686.68f, 1671.13f, 135.499f }, { 1683.32f, 1672.10f, 136.382f }, { 1676.21f, 1673.95f, 137.504f },
        { 1672.78f, 1674.63f, 138.265f }, { 1671.65f, 1680.40f, 139.247f }, { 1672.95f, 1685.73f, 139.639f },
        { 1673.60f, 1688.58f, 139.695f }, { 1675.72f, 1697.68f, 140.331f }, { 1682.36f, 1695.94f, 138.045f },
        { 1688.03f, 1692.28f, 137.092f }, { 1686.76f, 1686.93f, 137.070f }, { 1683.31f, 1686.49f, 137.777f },
        { 1688.84f, 1685.97f, 136.565f }, { 1701.76f, 1685.93f, 134.241f }, { 1701.76f, 1685.93f, 134.241f },
        { 1708.09f, 1692.04f, 133.826f }, { 1707.34f, 1696.12f, 133.928f }, { 1704.29f, 1699.33f, 134.399f },
        { 1707.40f, 1700.54f, 134.079f }
    };

    // the graves she stops at, seven seconds each
    static bool isGraveNode(uint32_t node) { return node == 2 || node == 4 || node == 18 || node == 25; }

    static const char* aradneSay[] =
    {
        "Rise from the grave and serve the Dark Lady!",
        "In the name of Sylvanas, the Banshee Queen, I bestow this gift upon you!",
        "Rise from death's slumber and join your brothers!",
        "Waken, sleeper. Your new life awaits.",
        "You are returned to life. The Banshee Queen asks for your service."
    };

    static const LocationVector zombiePath[] = { { 1695.18f, 1672.76f, 134.064f }, { 1669.67f, 1661.95f, 140.546f } };
    static const LocationVector barracksPath[] = { { 1696.71f, 1674.25f, 134.094f }, { 1696.79f, 1660.87f, 130.983f } };
}

class Aradne : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Aradne(c); }
    explicit Aradne(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setCanEnterCombat(false);
        getCreature()->setMoveHover(true);

        uint32_t node = 1;
        for (const auto& coords : Deathknell::aradnePath)
        {
            addWaypoint(Deathknell::PATH_ARADNE, createWaypoint(node, Deathknell::isGraveNode(node) ? 7000 : 0, WAYPOINT_MOVE_TYPE_WALK, coords));
            ++node;
        }

        getCreature()->getMovementManager()->movePath(*getCustomPath(Deathknell::PATH_ARADNE), true);
        RegisterAIUpdateEvent(1000);
    }

    void waypointReached(uint32_t nodeId, uint32_t pathId) override
    {
        if (pathId != Deathknell::PATH_ARADNE || !Deathknell::isGraveNode(nodeId))
            return;

        Creature* grave = findNearestCreature(Deathknell::NPC_GRAVE_TARGET, 10.0f);
        if (grave == nullptr)
            return;

        mGraveGuid = grave->getGuid();
        getCreature()->castSpell(grave, Deathknell::SPELL_RAISE_UNDEAD, true);
        mRaiseTimer = _addTimer(2000);
    }

    void AIUpdate() override
    {
        if (mRaiseTimer == 0 || !_isTimerFinished(mRaiseTimer))
            return;

        _removeTimer(mRaiseTimer);
        mRaiseTimer = 0;

        sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, Deathknell::aradneSay[Util::getRandomUInt(4)]);

        if (Creature* grave = getCreature()->getWorldMapCreature(mGraveGuid))
            if (CreatureAIScript* graveScript = grave->GetScript())
                graveScript->SetCreatureData(Deathknell::DATA_RAISE, 1);
    }

private:
    uint64_t mGraveGuid = 0;
    uint32_t mRaiseTimer = 0;
};

class DeathknellGraveTarget : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DeathknellGraveTarget(c); }
    explicit DeathknellGraveTarget(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setCanEnterCombat(false);
        getCreature()->setVisible(false);
    }

    void SetCreatureData(uint32_t type, uint32_t data) override
    {
        if (type == Deathknell::DATA_RAISE && data == 1)
            getCreature()->castSpell(getCreature(), Deathknell::SPELL_RISEN_DEAD, true);
    }
};

// one of three short scenes, then the risen dead leaves
class RisenDead : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RisenDead(c); }
    explicit RisenDead(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setCanEnterCombat(false);
        mScene = static_cast<uint8_t>(Util::getRandomUInt(2));
        mStep = 0;
        mStepTimer = _addTimer(2000);
        RegisterAIUpdateEvent(500);
    }

    void AIUpdate() override
    {
        if (mStepTimer == 0 || !_isTimerFinished(mStepTimer))
            return;

        _removeTimer(mStepTimer);
        mStepTimer = 0;

        switch (mScene)
        {
            case 0:
                enragedScene();
                break;
            case 1:
                barracksScene();
                break;
            default:
                suicideScene();
                break;
        }
        ++mStep;
    }

private:
    void say(const char* text) { sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, text); }
    void emote(const char* text) { sendChatMessage(CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, text); }

    void walk(const LocationVector* path, size_t count, uint32_t moveType = WAYPOINT_MOVE_TYPE_WALK)
    {
        for (size_t i = 0; i < count; ++i)
            addWaypoint(Deathknell::PATH_RISEN, createWaypoint(static_cast<uint32_t>(i + 1), 0, moveType, path[i]));
        getCreature()->getMovementManager()->movePath(*getCustomPath(Deathknell::PATH_RISEN), false);
    }

    void enragedScene()
    {
        if (mStep == 0)
        {
            say("I never asked for this! Leave me alone!");
            mStepTimer = _addTimer(4000);
            return;
        }

        if (const auto zombie = sMySQLStore.getCreatureProperties(Deathknell::NPC_MINDLESS_ZOMBIE))
            getCreature()->setDisplayId(zombie->Male_DisplayID);
        emote("%s enrages and transforms into a Mindless Zombie and runs away.");
        getCreature()->castSpell(getCreature(), Deathknell::SPELL_ENRAGE, true);
        walk(Deathknell::zombiePath, 2, WAYPOINT_MOVE_TYPE_RUN);
        despawn(9000, 0);
    }

    void barracksScene()
    {
        switch (mStep)
        {
            case 0:
                say(Util::getRandomUInt(1) ? "What magic is this that turn back the hand of death?" : "This is Lady Sylvanas's doing?");
                mStepTimer = _addTimer(4000);
                break;
            case 1:
                say(Util::getRandomUInt(1) ? "If the Banshee Queen has offered me this chance, I will gladly serve." : "What use has the Dark Lady for me?");
                mStepTimer = _addTimer(2000);
                break;
            default:
                walk(Deathknell::barracksPath, 2);
                emote("%s walks off towards the barracks.");
                despawn(9000, 0);
                break;
        }
    }

    void suicideScene()
    {
        switch (mStep)
        {
            case 0:
                say(Util::getRandomUInt(1) ? "What... what's happening to me? Why have you done this to me?" : "I... I don't remember... Why have you done this?");
                mStepTimer = _addTimer(4000);
                break;
            case 1:
                say("I should be dead and to death I will return!");
                mStepTimer = _addTimer(3000);
                break;
            default:
                emote("%s commits suicide.");
                getCreature()->die(getCreature(), 0, 0);
                break;
        }
    }

    uint8_t mScene = 0;
    uint8_t mStep = 0;
    uint32_t mStepTimer = 0;
};

void SetupTirisfalGlades(ScriptMgr* mgr)
{
    mgr->register_quest_script(410, new TheDormantShade());
    mgr->register_creature_script(6784, &CalvinMontague::Create);
    mgr->register_creature_script(1931, &Zealot::Create);
    mgr->register_quest_script(24959, new FreshOutOfTheGrave());

    // the graves intro exists from 4.x on
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
    {
        mgr->register_creature_script(Deathknell::NPC_ARADNE, &Aradne::Create);
        mgr->register_creature_script(Deathknell::NPC_GRAVE_TARGET, &DeathknellGraveTarget::Create);
        mgr->register_creature_script(Deathknell::NPC_RISEN_DEAD, &RisenDead::Create);
    }
}
