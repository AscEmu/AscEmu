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
#include "Map/Maps/MapScriptInterface.h"
#include "Movement/MovementManager.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"

LocationVector const WaypointTheSummoning[] =
{
    { 269.29f, -1433.32f, 50.31f }, //1
    { 328.52f, -1442.03f, 40.50f },
    { 333.31f, -1453.69f, 42.01f }  //3
};
std::size_t const pathSize = std::extent<decltype(WaypointTheSummoning)>::value;

class TheSummoning : public QuestScript
{
public:
    void OnQuestStart(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* windwatcher = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 6176);
        if (windwatcher == nullptr)
            return;

        // questgiver will walk to the place where Cyclonian is spawned only walk when we are at home
        if (windwatcher->CalcDistance(250.839996f, -1470.579956f, 55.4491f) > 1) return;
        {
            windwatcher->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Follow me");

            MovementMgr::PointsArray path;
            path.reserve(pathSize);
            std::transform(std::begin(WaypointTheSummoning), std::end(WaypointTheSummoning), std::back_inserter(path), [](LocationVector const& pos)
            {
                return G3D::Vector3(pos.x, pos.y, pos.z);
            });
            MovementMgr::MoveSplineInit init(windwatcher);
            init.SetWalk(true);
            init.MovebyPath(path);
            windwatcher->getMovementManager()->launchMoveSpline(std::move(init), 0, MOTION_PRIORITY_NORMAL, POINT_MOTION_TYPE);

        }
        windwatcher->Despawn(15 * 60 * 1000, 0);

        // spawn cyclonian if not spawned already
        Creature* cyclonian = pPlayer->getWorldMap()->getInterface()->getCreatureNearestCoords(323.947f, -1483.68f, 43.1363f, 6239);
        if (cyclonian == nullptr)
        {
            cyclonian = pPlayer->getWorldMap()->createAndSpawnCreature(6239, LocationVector(323.947f, -1483.68f, 43.1363f, 0.682991f));

            // if spawning cyclonian failed, we have to return.
            if (cyclonian == nullptr)
                return;
        }

        // queue cyclonian for despawn
        cyclonian->Despawn(15 * 60 * 1000, 0);
    }
};

class Bartleby : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new Bartleby(c); }
    explicit Bartleby(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setFaction(11);
        getCreature()->setEmoteState(EMOTE_ONESHOT_EAT);
    }

    void OnDamageTaken(Unit* mAttacker, uint32_t fAmount) override
    {
        if (getCreature()->getHealth() - fAmount <= getCreature()->getMaxHealth() * 0.37f)
        {
            if (mAttacker->isPlayer())
            {
                getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                RegisterAIUpdateEvent(1000);

                if (auto* questLog = static_cast<Player*>(mAttacker)->getQuestLogByQuestId(1640))
                    questLog->sendQuestComplete();
            }
        }
    }

    void AIUpdate() override
    {
        getCreature()->removeAllNegativeAuras();
        getCreature()->setFaction(11);
        getCreature()->setHealthPct(100);
        getCreature()->getThreatManager().clearAllThreat();
        getCreature()->getThreatManager().removeMeFromThreatLists();
        getCreature()->getAIInterface()->handleEvent(EVENT_LEAVECOMBAT, getCreature(), 0);
        _setMeleeDisabled(true);
        getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
        getCreature()->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        RemoveAIUpdateEvent();
    }
};

class BeatBartleby : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        float SSX = mTarget->GetPositionX();
        float SSY = mTarget->GetPositionY();
        float SSZ = mTarget->GetPositionZ();

        Creature* Bartleby = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(SSX, SSY, SSZ, 6090);

        if (Bartleby == nullptr)
            return;

        Bartleby->setFaction(168);
        Bartleby->getAIInterface()->setMeleeDisabled(false);
        Bartleby->getAIInterface()->setAllowedToEnterCombat(true);
    }
};

enum TwiggyFlatheadInfo
{
    NPC_BIG_WILL = 6238,
    NPC_AFFRAY_CHALLENGER = 6240,

    SAY_BIG_WILL_READY = 3375,
    SAY_TWIGGY_FLATHEAD_BEGIN = 3376,
    SAY_TWIGGY_FLATHEAD_FRAY = 3377,
    SAY_TWIGGY_FLATHEAD_DOWN = 3378,
    SAY_TWIGGY_FLATHEAD_OVER = 3379
};

LocationVector const AffrayChallengerLoc[6] =
{
    {-1683.0f, -4326.0f, 2.79f, 0.0f},
    {-1682.0f, -4329.0f, 2.79f, 0.0f},
    {-1683.0f, -4330.0f, 2.79f, 0.0f},
    {-1680.0f, -4334.0f, 2.79f, 1.49f},
    {-1674.0f, -4326.0f, 2.79f, 3.49f},
    {-1677.0f, -4334.0f, 2.79f, 1.66f}
};

class TwiggyFlathead : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TwiggyFlathead(c); }
    explicit TwiggyFlathead(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        initialize();
    }

    void reset()
    {
        initialize();
    }

    void initialize()
    {
        eventInProgress = false;
        eventGrate = false;
        eventBigWill = false;
        WaveTimer = 600000;
        ChallengerChecker = 0;
        Wave = 0;
        pPlayer = nullptr;

        for (uint8_t i = 0; i < 6; ++i)
        {
            affrayChallenger[i] = nullptr;
            challengerDown[i] = false;
        }

        bigWill = nullptr;
    }

    void SetCreatureData64(uint32_t Type, uint64_t Data) override
    {
        switch (Type)
        {
        case 1:
            if (!eventInProgress)
            {
                Player* warrior = getCreature()->getWorldMapPlayer(Data);

                if (warrior)
                    pPlayer = warrior;
            }
            break;
        }
    }

    void AIUpdate() override
    {
        if (eventInProgress)
        {
            Player* warrior = nullptr;

            if (pPlayer)
                warrior = pPlayer;

            if (!warrior || !warrior->hasQuestInQuestLog(1719))
            {
                despawnAndUnsummon();
                reset();
                return;
            }

            if (!warrior->isAlive() && warrior->getQuestLogByQuestId(1719)->getQuestState() == QUEST_INCOMPLETE)
            {
                sendDBChatMessage(SAY_TWIGGY_FLATHEAD_DOWN);
                warrior->getQuestLogByQuestId(1719)->sendQuestFailed(true);
                despawnAndUnsummon();
                reset();
            }

            if (!eventGrate && eventInProgress)
            {
                float x, y, z;
                warrior->getPosition(x, y, z);

                if (x >= -1684 && x <= -1674 && y >= -4334 && y <= -4324)
                {
                    warrior->areaExploredQuestEvent(1719);
                    sendDBChatMessage(SAY_TWIGGY_FLATHEAD_BEGIN, warrior);

                    for (uint8_t i = 0; i < 6; ++i)
                    {
                        Creature* creature = spawnCreature(NPC_AFFRAY_CHALLENGER, AffrayChallengerLoc[i]);
                        if (!creature)
                            continue;

                        creature->setFaction(35);
                        creature->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                        creature->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
                        creature->emote(EMOTE_ONESHOT_ROAR);
                        creature->Despawn(600000, 0);
                        affrayChallenger[i] = creature;
                    }
                    WaveTimer = 5000;
                    ChallengerChecker = 1000;
                    eventGrate = true;
                }
            }
            else if (eventInProgress)
            {
                if (ChallengerChecker <= GetAIUpdateFreq())
                {
                    for (uint8_t i = 0; i < 6; ++i)
                    {
                        if (affrayChallenger[i])
                        {
                            Creature* creature = affrayChallenger[i];
                            if ((!creature || (!creature->isAlive())) && !challengerDown[i])
                            {
                                sendDBChatMessage(3378);
                                challengerDown[i] = true;
                            }
                        }
                    }
                    ChallengerChecker = 1000;
                }
                else ChallengerChecker -= GetAIUpdateFreq();

                if (WaveTimer <= GetAIUpdateFreq())
                {
                    if (Wave < 6 && affrayChallenger[Wave] && !eventBigWill)
                    {
                        sendDBChatMessage(3377);
                        Creature* creature = affrayChallenger[Wave];
                        if (creature && (creature->isAlive()))
                        {
                            creature->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                            creature->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
                            creature->emote(EMOTE_ONESHOT_ROAR);
                            creature->setFaction(14);

                            creature->getAIInterface()->onHostileAction(warrior);
                            ++Wave;
                            WaveTimer = 20000;
                        }
                    }
                    else if (Wave >= 6 && !eventBigWill)
                    {
                        if (Creature* creature = spawnCreature(NPC_BIG_WILL, -1722, -4341, 6.12f, 6.26f))
                        {
                            bigWill = creature;
                            creature->getMovementManager()->movePoint(0, -1682, -4329, 2.79f);
                            creature->emote(EMOTE_STATE_READYUNARMED);
                            creature->Despawn(480000, 0);
                            eventBigWill = true;
                            WaveTimer = 1000;
                        }
                    }
                    else if (Wave >= 6 && eventBigWill && bigWill)
                    {
                        Creature* creature = bigWill;
                        if (!creature || !creature->isAlive())
                        {
                            sendDBChatMessage(SAY_TWIGGY_FLATHEAD_OVER);
                            reset();
                        }
                        else if(!creature->getAIInterface()->isEngaged()) // Makes BIG WILL attackable.
                        {
                            creature->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                            creature->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
                            creature->SendScriptTextChatMessage(SAY_BIG_WILL_READY);
                            creature->emote(EMOTE_ONESHOT_ROAR);
                            creature->setFaction(14);
                            creature->getAIInterface()->onHostileAction(warrior);
                        }
                    }
                }
                else WaveTimer -= GetAIUpdateFreq();
            }
        }
    }

    void despawnAndUnsummon()
    {
        for (uint8_t i = 0; i < 6; ++i) // unsummon challengers
        {
            if (affrayChallenger[i])
            {
                if (affrayChallenger[i] && affrayChallenger[i]->isAlive())
                    affrayChallenger[i]->Despawn(0, 0);
            }
        }

        if (bigWill) // unsummon bigWill
        {
            if (bigWill && bigWill->isAlive())
                bigWill->Despawn(0, 0);
        }
    }

    void DoAction(int32_t const action) override
    {
        if (action == 1 && !eventInProgress)
        {
            eventInProgress = true;
        }
    }

    bool eventInProgress;
    bool eventGrate;
    bool eventBigWill;
    bool challengerDown[6];
    uint8_t Wave;
    uint32_t WaveTimer;
    uint32_t ChallengerChecker;
    Player* pPlayer;
    Creature* affrayChallenger[6];
    Creature* bigWill;
};

void SetupWarrior(ScriptMgr* mgr)
{
    mgr->register_quest_script(1713, new TheSummoning());
    mgr->register_creature_script(6090, &Bartleby::Create);
    mgr->register_quest_script(1640, new BeatBartleby());
    mgr->register_creature_script(6248, &TwiggyFlathead::Create);
}
