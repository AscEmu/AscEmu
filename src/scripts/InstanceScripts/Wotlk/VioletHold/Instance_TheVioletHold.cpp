/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_TheVioletHold.hpp"
#include "AzureSaboteur.hpp"
#include "Cyangosa.hpp"
#include "Erekem.hpp"
#include "Ichron.hpp"
#include "Lavanthor.hpp"
#include "Moragg.hpp"
#include "Portal_Common.hpp"
#include "Portal_Elite.hpp"
#include "Portal_Intro.hpp"
#include "Setup.h"
#include "Xevozz.hpp"
#include "Zuramat.hpp"
#include "Management/Gossip/GossipMenu.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "CommonTime.hpp"
#include "Utilities/Random.hpp"


//////////////////////////////////////////////////////////////////////////////////////////
// TheVioletHold Instance
TheVioletHoldScript::TheVioletHoldScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
{
    setBossNumber(EncounterCount);
    setupInstanceData(creatureData, gameObjectData);

    FirstBossId = 0;
    SecondBossId = 0;

    DoorIntegrity = 100;
    WaveCount = 0;
    EventState = EncounterStates::NotStarted;

    LastPortalLocation = Util::getRandomUInt(0, EncouterPortalsCount - 1);

    Defenseless = true;
}

InstanceScript* TheVioletHoldScript::Create(WorldMap* pMapMgr) { return new TheVioletHoldScript(pMapMgr); }

void TheVioletHoldScript::writeSaveDataExtended(std::ostringstream& data)
{
    data << FirstBossId << ' ' << SecondBossId;
}

void TheVioletHoldScript::readSaveDataExtended(std::istringstream& data)
{
    data >> FirstBossId;
    data >> SecondBossId;
}

bool TheVioletHoldScript::setBossState(uint32_t type, EncounterStates state)
{
    if (!InstanceScript::setBossState(type, state))
        return false;

    switch (type)
    {
        case DATA_1ST_BOSS:
            if (state == Performed)
                updateEncountersStateForCreature(NPC_EREKEM, getWorldMap()->getDifficulty());
            break;
        case DATA_2ND_BOSS:
            if (state == Performed)
                updateEncountersStateForCreature(NPC_MORAGG, getWorldMap()->getDifficulty());
            break;
        case DATA_CYANIGOSA:
            if (state == Performed)
                setLocalData(DATA_MAIN_EVENT_STATE, Performed);
            break;
        case DATA_MORAGG:
        case DATA_EREKEM:
        case DATA_ICHORON:
        case DATA_LAVANTHOR:
        case DATA_XEVOZZ:
        case DATA_ZURAMAT:
            if (WaveCount == 6)
                setBossState(DATA_1ST_BOSS, state);
            else if (WaveCount == 12)
                setBossState(DATA_2ND_BOSS, state);

            if (state == Performed)
                setLocalData(DATA_WAVE_COUNT, WaveCount + 1);
            break;
        default:
            break;
    }

    return true;
}

void TheVioletHoldScript::OnCreaturePushToWorld(Creature* pCreature)
{
    WoWGuid guid = pCreature->getGuid();

    switch (pCreature->getEntry())
    {
        case NPC_EREKEM_GUARD:
        {
            for (uint8_t i = 0; i < ErekemGuardCount; ++i)
                if (!ErekemGuardGUIDs[i])
                {
                    ErekemGuardGUIDs[i] = guid.getGuidLowPart();
                    break;
                }
        } break;
        default:
            break;
    }
}

void TheVioletHoldScript::OnGameObjectPushToWorld(GameObject* pGameObject)
{
    switch (pGameObject->getEntry())
    {
        case GO_ACTIVATION_CRYSTAL :
        {
            for (uint8_t i = 0; i < ActivationCrystalCount; ++i)
                if (!ActivationCrystalGUIDs[i])
                {
                    ActivationCrystalGUIDs[i] = pGameObject->getGuidLow();
                    break;
                }
        } break;
        default:
            break;
    }
}

void TheVioletHoldScript::UpdateEvent()
{
    // Wipe Check
    if (getWorldMap()->getPlayerCount() == 0)
    {
        if (EventState == EncounterStates::InProgress)
        {
            EventState = EncounterStates::Failed;
            StateCheck();
        }
        return;
    }

    // Update Event Timers
    scriptEvents.updateEvents(getUpdateFrequency(), 0);

    // Check Door Integrety
    if (EventState == EncounterStates::InProgress)
    {
        // if door is destroyed, event is failed
        if (!getLocalData(DATA_DOOR_INTEGRITY))
            EventState = EncounterStates::Failed;
    }

    // Events
    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case EVENT_NEXT_WAVE:
            {
                // Update Wave Counter
                getInstance()->getWorldStatesHandler().SetWorldStateForZone(4415, 0, WORLD_STATE_VH_WAVE_COUNT, WaveCount);

                switch (WaveCount)
                {
                    case 6:
                    {
                        if (FirstBossId == 0)
                            FirstBossId = Util::getRandomUInt(DATA_MORAGG, DATA_ZURAMAT);

                        if (Creature* sinclari = getCreatureFromData(DATA_SINCLARI))
                        {
                            sinclari->summonCreature(NPC_TELEPORTATION_PORTAL_INTRO, PortalIntroPositions[3], TIMED_DESPAWN, 3 * TimeVarsMs::Second);
                            sinclari->summonCreature(NPC_SABOTEOUR, SaboteurSpawnLocation, DEAD_DESPAWN);
                        }
                    } break;
                    case 12:
                    {
                        if (SecondBossId == 0)
                            do
                            {
                                SecondBossId = Util::getRandomUInt(DATA_MORAGG, DATA_ZURAMAT);
                            } while (SecondBossId == FirstBossId);
                            if (Creature* sinclari = getCreatureFromData(DATA_SINCLARI))
                            {
                                sinclari->summonCreature(NPC_TELEPORTATION_PORTAL_INTRO, PortalIntroPositions[3], TIMED_DESPAWN, 3 * TimeVarsMs::Second);
                                sinclari->summonCreature(NPC_SABOTEOUR, SaboteurSpawnLocation, DEAD_DESPAWN);
                            }
                    } break;
                    case 18:
                    {
                        if (Creature* sinclari = getCreatureFromData(DATA_SINCLARI))
                        {
                            sinclari->summonCreature(NPC_TELEPORTATION_PORTAL_INTRO, PortalIntroPositions[4], TIMED_DESPAWN, 6 * TimeVarsMs::Second);
                            if (Creature* cyanigosa = sinclari->summonCreature(NPC_CYANIGOSA, CyanigosaSpawnLocation, DEAD_DESPAWN))
                                cyanigosa->castSpell(cyanigosa, SPELL_CYANIGOSA_ARCANE_POWER_STATE, true);

                            startCyanigosaIntro();
                        }
                    } break;
                    default:
                        spawnPortal();
                        break;
                }
            } break;
            case EVENT_STATE_CHECK:
            {
                StateCheck();
                scriptEvents.addEvent(EVENT_STATE_CHECK, 3 * TimeVarsMs::Second);
            } break;
            case EVENT_TIMER1:
            {
                uint32_t bossId = 0;

                if (WaveCount == 6)
                    bossId = FirstBossId;
                else
                    bossId = SecondBossId;

                if (Creature* boss = getCreatureFromData(bossId))
                {
                    switch (bossId)
                    {
                        case DATA_MORAGG:
                        {
                            boss->PlaySoundToSet(SOUND_MORAGG_SPAWN);
                            boss->castSpell(boss, SPELL_MORAGG_EMOTE_ROAR);
                        } break;
                        case DATA_EREKEM:
                        {
                            boss->GetScript()->sendDBChatMessage(SAY_EREKEM_SPAWN);
                        } break;
                        case DATA_ICHORON:
                        {
                            boss->GetScript()->sendDBChatMessage(SAY_ICHORON_SPAWN);
                        } break;
                        case DATA_LAVANTHOR:
                        {
                            boss->castSpell(boss, SPELL_LAVANTHOR_SPECIAL_UNARMED);
                        } break;
                        case DATA_XEVOZZ:
                        {
                            boss->GetScript()->sendDBChatMessage(SAY_XEVOZZ_SPAWN);
                        } break;
                        case DATA_ZURAMAT:
                        {
                            boss->castSpell(boss, SPELL_ZURAMAT_COSMETIC_CHANNEL_OMNI);
                            boss->GetScript()->sendDBChatMessage(SAY_ZURAMAT_SPAWN);
                        } break;
                    }
                }

                scriptEvents.addEvent(EVENT_TIMER2, 5 * TimeVarsMs::Second);
            } break;
            case EVENT_TIMER2:
            {
                uint32_t bossId = 0;

                if (WaveCount == 6)
                    bossId = FirstBossId;
                else
                    bossId = SecondBossId;

                if (Creature* boss = getCreatureFromData(bossId))
                {
                    switch (bossId)
                    {
                        case DATA_MORAGG:
                        {
                            boss->getMovementManager()->moveSmoothPath(POINT_INTRO, MoraggPath, MoraggPathSize, true);
                        } break;
                        case DATA_EREKEM:
                        {
                            boss->getMovementManager()->moveSmoothPath(POINT_INTRO, ErekemPath, ErekemPathSize, true);

                            if (Creature* guard = GetCreatureByGuid(getLocalData(DATA_EREKEM_GUARD_1)))
                                guard->getMovementManager()->moveSmoothPath(POINT_INTRO, ErekemGuardLeftPath, ErekemGuardLeftPathSize, true);
                            if (Creature* guard = GetCreatureByGuid(getLocalData(DATA_EREKEM_GUARD_2)))
                                guard->getMovementManager()->moveSmoothPath(POINT_INTRO, ErekemGuardRightPath, ErekemGuardRightPathSize, true);

                        } break;
                        case DATA_ICHORON:
                        {
                            boss->getMovementManager()->moveSmoothPath(POINT_INTRO, IchoronPath, IchoronPathSize, true);
                        } break;
                        case DATA_LAVANTHOR:
                        {
                            boss->getMovementManager()->moveSmoothPath(POINT_INTRO, LavanthorPath, LavanthorPathSize, true);
                        } break;
                        case DATA_XEVOZZ:
                        {
                            boss->emote(EMOTE_ONESHOT_TALK_NOSHEATHE);
                        } break;
                        case DATA_ZURAMAT:
                        {
                            boss->getMovementManager()->moveSmoothPath(POINT_INTRO, ZuramatPath, ZuramatPathSize, true);
                        } break;
                    }
                }

                scriptEvents.addEvent(EVENT_TIMER3, 8 * TimeVarsMs::Second);
            } break;
            case EVENT_TIMER3:
            {
                uint32_t bossId = 0;

                if (WaveCount == 6)
                    bossId = FirstBossId;
                else
                    bossId = SecondBossId;

                if (Creature* boss = getCreatureFromData(bossId))
                {
                    boss->getAIInterface()->setImmuneToNPC(false);
                    boss->getAIInterface()->setImmuneToPC(false);
                    boss->removeUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);

                    switch (bossId)
                    {
                        case DATA_EREKEM:
                        {
                            boss->emote(EMOTE_ONESHOT_ROAR);

                            for (uint32_t i = DATA_EREKEM_GUARD_1; i <= DATA_EREKEM_GUARD_2; ++i)
                            {
                                if (Creature* guard = GetCreatureByGuid(getLocalData(i)))
                                {
                                    guard->getAIInterface()->setImmuneToNPC(false);
                                    guard->getAIInterface()->setImmuneToPC(false);
                                    guard->removeUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
                                }
                            }
                        } break;
                        case DATA_XEVOZZ:
                        {
                            boss->getMovementManager()->moveSmoothPath(POINT_INTRO, XevozzPath, XevozzPathSize, true);
                        } break;
                    }
                }
            } break;
            case EVENT_CYANIGOSA_INTRO1:
            {
                if (Creature* cyanigosa = getCreatureFromData(DATA_CYANIGOSA))
                    if (cyanigosa->GetScript())
                        cyanigosa->GetScript()->sendDBChatMessage(SAY_CYANIGOSA_SPAWN);
            } break;
            case EVENT_CYANIGOSA_INTRO2:
            {
                if (Creature* cyanigosa = getCreatureFromData(DATA_CYANIGOSA))
                    cyanigosa->getMovementManager()->moveJump(CyanigosaJumpLocation, 10.0f, 27.44744f);
            } break;
            case EVENT_CYANIGOSA_INTRO3:
            {
                if (Creature* cyanigosa = getCreatureFromData(DATA_CYANIGOSA))
                {
                    cyanigosa->removeAllAurasById(SPELL_CYANIGOSA_ARCANE_POWER_STATE);
                    cyanigosa->castSpell(cyanigosa, SPELL_CYANIGOSA_TRANSFORM, true);
                    cyanigosa->getAIInterface()->setImmuneToNPC(false);
                    cyanigosa->getAIInterface()->setImmuneToPC(false);
                    cyanigosa->removeUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
                }
            } break;
            default:
                break;
        }
    }
}

void TheVioletHoldScript::setLocalData(uint32_t type, uint32_t data)
{
    switch (type)
    {
        case DATA_WAVE_COUNT:
        {
            WaveCount = data;
            if (WaveCount)
            {
                // Add Next Wave
                scriptEvents.addEvent(EVENT_NEXT_WAVE, (isBossWave(WaveCount - 1) ? 45 : 5) * TimeVarsMs::Second);
            }
        } break;
        case DATA_DOOR_INTEGRITY:
        {
            DoorIntegrity = data;
            Defenseless = false;
            getInstance()->getWorldStatesHandler().SetWorldStateForZone(4415, 0, WORLD_STATE_VH_PRISON_STATE, DoorIntegrity);
        } break;
        case DATA_MAIN_EVENT_STATE:
        {
            EventState = data;

            switch (EventState)
            {
                case EncounterStates::NotStarted: // Reset the Encounter
                {
                    // Open Door
                    if (GameObject* mainDoor = getGameObjectFromData(DATA_MAIN_DOOR))
                    {
                        mainDoor->setState(GO_STATE_OPEN);
                        mainDoor->removeFlags(GO_FLAG_LOCKED);
                    }

                    // Reset Worldstates
                    getInstance()->getWorldStatesHandler().SetWorldStateForZone(4415, 0, WORLD_STATE_VH_WAVE_COUNT, WaveCount);
                    getInstance()->getWorldStatesHandler().SetWorldStateForZone(4415, 0, WORLD_STATE_VH_PRISON_STATE, DoorIntegrity);
                    getInstance()->getWorldStatesHandler().SetWorldStateForZone(4415, 0, WORLD_STATE_VH_SHOW, 0);

                    // Make Activation Crystalls not Selectable
                    for (uint8_t i = 0; i < ActivationCrystalCount; ++i)
                        if (GameObject* crystal = GetGameObjectByGuid(ActivationCrystalGUIDs[i]))
                            crystal->setFlags(GO_FLAG_NOT_SELECTABLE);
                } break;
                case EncounterStates::InProgress: // We Started the Encounter
                {
                    getInstance()->getWorldStatesHandler().SetWorldStateForZone(4415, 0, WORLD_STATE_VH_WAVE_COUNT, WaveCount);
                    getInstance()->getWorldStatesHandler().SetWorldStateForZone(4415, 0, WORLD_STATE_VH_PRISON_STATE, DoorIntegrity);
                    getInstance()->getWorldStatesHandler().SetWorldStateForZone(4415, 0, WORLD_STATE_VH_SHOW, 1);

                    WaveCount = 1;
                    scriptEvents.addEvent(EVENT_NEXT_WAVE, 1 * TimeVarsMs::Second);
                    scriptEvents.addEvent(EVENT_STATE_CHECK, 3 * TimeVarsMs::Second);

                    for (uint8_t i = 0; i < ActivationCrystalCount; ++i)
                        if (GameObject* crystal = GetGameObjectByGuid(ActivationCrystalGUIDs[i]))
                            crystal->removeFlags(GO_FLAG_NOT_SELECTABLE);

                } break;
                case EncounterStates::Performed: // Encounter Done
                {
                    // Open Door
                    if (GameObject* mainDoor = getGameObjectFromData(DATA_MAIN_DOOR))
                    {
                        mainDoor->setState(GO_STATE_OPEN);
                        mainDoor->removeFlags(GO_FLAG_LOCKED);
                    }

                    // Hide Worldstates
                    getInstance()->getWorldStatesHandler().SetWorldStateForZone(4415, 0, WORLD_STATE_VH_SHOW, 0);

                    // Play Outro
                    if (Creature* sinclari = getCreatureFromData(DATA_SINCLARI))
                        sinclari->GetScript()->DoAction(ACTION_SINCLARI_OUTRO);
                } break;
            }
        } break;
        case DATA_START_BOSS_ENCOUNTER:
        {
            switch (WaveCount)
            {
                case 6:
                    startBossEncounter(FirstBossId);
                    break;
                case 12:
                    startBossEncounter(SecondBossId);
                    break;
            }
        } break;
        case DATA_HANDLE_CELLS:
        {
            handleCells(data, false);
        } break;
        default:
            break;
    }
};

uint32_t TheVioletHoldScript::getLocalData(uint32_t type) const
{
    switch (type)
    {
        case DATA_1ST_BOSS:
            return FirstBossId;
        case DATA_2ND_BOSS:
            return SecondBossId;
        case DATA_MAIN_EVENT_STATE:
            return EventState;
        case DATA_WAVE_COUNT:
            return WaveCount;
        case DATA_DOOR_INTEGRITY:
            return DoorIntegrity;
        case DATA_EREKEM_GUARD_1:
            return ErekemGuardGUIDs[0];
        case DATA_EREKEM_GUARD_2:
            return ErekemGuardGUIDs[1];
        case DATA_DEFENSELESS:
            return Defenseless ? 1 : 0;
        default:
            break;
    }

    return 0;
}

void TheVioletHoldScript::spawnPortal()
{
    LastPortalLocation = (LastPortalLocation + Util::getRandomUInt(1, EncouterPortalsCount - 1)) % (EncouterPortalsCount);
    if (Creature* sinclari = getCreatureFromData(DATA_SINCLARI))
    {
        if (LastPortalLocation < PortalPositionsSize)
        {
            if (Creature* portal = sinclari->summonCreature(NPC_TELEPORTATION_PORTAL, PortalPositions[LastPortalLocation], CORPSE_DESPAWN))
                portal->GetScript()->SetCreatureData(DATA_PORTAL_LOCATION, LastPortalLocation);
        }
        else
        {
            if (Creature* portal = sinclari->summonCreature(NPC_TELEPORTATION_PORTAL_ELITE, PortalElitePositions[LastPortalLocation - PortalPositionsSize], CORPSE_DESPAWN))
                portal->GetScript()->SetCreatureData(DATA_PORTAL_LOCATION, LastPortalLocation);
        }
    }
}

void TheVioletHoldScript::startCyanigosaIntro()
{
    scriptEvents.addEvent(EVENT_CYANIGOSA_INTRO1, 2 * TimeVarsMs::Second);
    scriptEvents.addEvent(EVENT_CYANIGOSA_INTRO2, 6 * TimeVarsMs::Second);
    scriptEvents.addEvent(EVENT_CYANIGOSA_INTRO3, 7 * TimeVarsMs::Second);
}

void TheVioletHoldScript::StateCheck()
{
    // if main event is in progress and players have wiped then reset instance
    if ((EventState == EncounterStates::InProgress && WipeCheck()) || EventState == EncounterStates::Failed)
    {
        resetBossEncounter(FirstBossId);
        resetBossEncounter(SecondBossId);
        resetBossEncounter(DATA_CYANIGOSA);

        WaveCount = 0;
        DoorIntegrity = 100;
        Defenseless = true;
        setLocalData(DATA_MAIN_EVENT_STATE, EncounterStates::NotStarted);

        scriptEvents.resetEvents();

        if (Creature* sinclari = getCreatureFromData(DATA_SINCLARI))
        {
            sinclari->getAIInterface()->enterEvadeMode();
            sinclari->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
        }
    }
}

bool TheVioletHoldScript::WipeCheck()
{
    for (auto player : getWorldMap()->getPlayers())
    {
        if (Player* plr = player.second)
        {
            // Skip Gamemasters
            if (plr->hasPlayerFlags(PLAYER_FLAG_GM))
                continue;

            // Players still Alive
            if (plr->isAlive())
                return false;
        }
    }
    return true;
}

void TheVioletHoldScript::startBossEncounter(uint8_t bossId)
{
    scriptEvents.addEvent(EVENT_TIMER1, 3 * TimeVarsMs::Second);
    handleCells(bossId);
}

void TheVioletHoldScript::resetBossEncounter(uint8_t bossId)
{
    if (bossId < DATA_CYANIGOSA || bossId > DATA_ZURAMAT)
        return;

    Creature* boss = getCreatureFromData(bossId);
    if (!boss)
        return;

    switch (bossId)
    {
        case DATA_CYANIGOSA:
        {
            boss->ToSummon()->unSummon();
        } break;
        case DATA_EREKEM:
        {
            for (uint32_t i = DATA_EREKEM_GUARD_1; i <= DATA_EREKEM_GUARD_2; ++i)
            {
                if (Creature* guard = GetCreatureByGuid(getLocalData(i)))
                {
                    if (guard->isDead())
                        guard->Despawn(1000, 1);

                    if (getBossState(bossId) == EncounterStates::Performed)
                    {
                        guard->SetPosition(guard->GetSpawnPosition());
                        updateKilledBoss(guard);
                    }

                    guard->getMovementManager()->moveTargetedHome();
                    guard->getAIInterface()->setImmuneToNPC(true);
                    guard->getAIInterface()->setImmuneToPC(true);
                    guard->addUnitFlags(UNIT_FLAG_IGNORE_CREATURE_COMBAT);
                }
            }
        } [[fallthrough]];
        default:
        {
            if (boss->isDead())
            {
                // respawn and update to a placeholder npc to avoid be looted again
                boss->SetPosition(boss->GetSpawnPosition());
                updateKilledBoss(boss);
                boss->Despawn(1000, 1);
            }
        } break;
    }
}

void TheVioletHoldScript::updateKilledBoss(Creature* boss)
{
    switch (boss->getEntry())
    {
        case NPC_XEVOZZ:
            boss->updateEntry(NPC_DUMMY_XEVOZZ);
            break;
        case NPC_LAVANTHOR:
            boss->updateEntry(NPC_DUMMY_LAVANTHOR);
            break;
        case NPC_ICHORON:
            boss->updateEntry(NPC_DUMMY_ICHORON);
            break;
        case NPC_ZURAMAT:
            boss->updateEntry(NPC_DUMMY_ZURAMAT);
            break;
        case NPC_EREKEM:
            boss->updateEntry(NPC_DUMMY_EREKEM);
            break;
        case NPC_MORAGG:
            boss->updateEntry(NPC_DUMMY_MORAGG);
            break;
        case NPC_EREKEM_GUARD:
            boss->updateEntry(NPC_DUMMY_EREKEM_GUARD);
            break;
        default:
            break;
    }
}

void TheVioletHoldScript::handleCells(uint8_t bossId, bool open)
{
    switch (bossId)
    {
        case DATA_MORAGG:
        {
            if (GameObject* go = getGameObjectFromData(DATA_MORAGG_CELL))
                go->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
        } break;
        case DATA_EREKEM:
        {
            if (GameObject* go = getGameObjectFromData(DATA_EREKEM_CELL))
                go->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);

            if (GameObject* go = getGameObjectFromData(DATA_EREKEM_LEFT_GUARD_CELL))
                go->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);

            if (GameObject* go = getGameObjectFromData(DATA_EREKEM_RIGHT_GUARD_CELL))
                go->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
        } break;
        case DATA_ICHORON:
        {
            if (GameObject* go = getGameObjectFromData(DATA_ICHORON_CELL))
                go->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
        } break;
        case DATA_LAVANTHOR:
        {
            if (GameObject* go = getGameObjectFromData(DATA_LAVANTHOR_CELL))
                go->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
        } break;
        case DATA_XEVOZZ:
        {
            if (GameObject* go = getGameObjectFromData(DATA_XEVOZZ_CELL))
                go->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
        } break;
        case DATA_ZURAMAT:
        {
            if (GameObject* go = getGameObjectFromData(DATA_ZURAMAT_CELL))
                go->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
        } break;
        default:
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Sinclari AI
SinclariAI::SinclariAI(Creature* pCreature) : CreatureAIScript(pCreature) 
{
    // Instance Script
    mInstance = getInstanceScript();

    // Get us in a Defined State
    Reset();
}

CreatureAIScript* SinclariAI::Create(Creature* pCreature) { return new SinclariAI(pCreature); }

void SinclariAI::justReachedSpawn()
{
    Reset();
}

void SinclariAI::AIUpdate(unsigned long time_passed) 
{
    if (!mInstance)
        return;

    scriptEvents.updateEvents(time_passed, getScriptPhase());

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case 1:
            {
                getCreature()->setMoveWalk(true);
                getCreature()->getMovementManager()->movePoint(0, SinclariPositions[0]);
            } break;
            case 2:
            {
                getCreature()->emote(EMOTE_ONESHOT_USESTANDING);
                // Spawn Defense System From WorldMap so it wont get Despawned later on
                getCreature()->getWorldMap()->summonCreature(NPC_DEFENSE_SYSTEM, DefenseSystemLocation);
            } break;
            case 3:
            {
                getCreature()->setFacingTo(SinclariPositions[0].getOrientation());
                sendDBChatMessage(SAY_SINCLARI_INTRO_1);

                std::list<Creature*> guardList;
                GetCreatureListWithEntryInGrid(guardList, NPC_VIOLET_HOLD_GUARD, 100.0f);
                for (Creature* guard : guardList)
                {
                    if (!guard->isAlive())
                        continue;

                    guard->getAIInterface()->setReactState(REACT_PASSIVE);
                    guard->setMoveWalk(false);
                    guard->getMovementManager()->movePoint(0, GuardsMovePosition);
                }
            } break;
            case 4:
            {
                getCreature()->getMovementManager()->movePoint(0, SinclariPositions[1]);
                summons.despawnAll();
            } break;
            case 5:
            {
                getCreature()->setFacingTo(SinclariPositions[1].getOrientation());

                std::list<Creature*> guardList;
                GetCreatureListWithEntryInGrid(guardList, NPC_VIOLET_HOLD_GUARD, 100.0f);
                for (Creature* guard : guardList)
                    guard->setVisible(false);
            } break;
            case 6:
            {
                sendDBChatMessage(SAY_SINCLARI_INTRO_2);
            } break;
            case 7:
            {
                getCreature()->emote(EMOTE_ONESHOT_TALK_NOSHEATHE);
            } break;
            case 8:
            {
                // Close the Door and Lock it
                if (GameObject* mainDoor = mInstance->getGameObjectFromData(DATA_MAIN_DOOR))
                {
                    mainDoor->setState(GO_STATE_CLOSED);
                    mainDoor->setFlags(GO_FLAG_LOCKED);
                }
            } break;
            case 9:
            {
                // Tell the Instance Script we are Started
                mInstance->setLocalData(DATA_MAIN_EVENT_STATE, EncounterStates::InProgress);
            } break;
            case 10:
            {
                getCreature()->setNpcFlags(UNIT_NPC_FLAG_NONE);
            } break;
        default:
            break;
        }
    }
}

void SinclariAI::DoAction(int32_t const action)
{
    if (action == ACTION_INTRO_START)
    {
        scriptEvents.addEvent(1, 100);      // Move
        scriptEvents.addEvent(2, 2000);     // Use Defense System
        scriptEvents.addEvent(3, 5000);     // Face To Players and Talk also Move Guards
        scriptEvents.addEvent(4, 7000);     // Talk More
        scriptEvents.addEvent(5, 12000);    // Despawn Guards
        scriptEvents.addEvent(6, 13000);    // Talk
        scriptEvents.addEvent(7, 19000);    // Put Weapon Away
        scriptEvents.addEvent(8, 20000);    // Close Door
        scriptEvents.addEvent(9, 25000);    // Start Encounter
        scriptEvents.addEvent(10, 30000);    // Gossip
    }

    if (action == ACTION_SINCLARI_OUTRO)
    {
        sendDBChatMessage(SAY_SINCLARI_OUTRO);
        getCreature()->getMovementManager()->movePoint(0, SinclariPositions[3]);
        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }
}

void SinclariAI::onSummonedCreature(Creature* summon)
{
    summons.summon(summon);
}

void SinclariAI::OnSummonDespawn(Creature* summon)
{
    summons.despawn(summon);
}


void SinclariAI::Reset()
{
    // Despawn All Portals
    summons.despawnAll();

    // Spawn All Portals
    for (uint8_t i = 0; i < PortalIntroCount; ++i)
    {
        if (Creature* summon = summonCreature(NPC_TELEPORTATION_PORTAL_INTRO, PortalIntroPositions[i]))
        {
            if (summon->GetScript())
                summon->GetScript()->SetCreatureData(DATA_PORTAL_LOCATION, i);
        }
    }

    // Make me a Gossip Again
    getCreature()->setVisible(true);
    getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);

    // Reset Guards
    std::list<Creature*> guardList;
    GetCreatureListWithEntryInGrid(guardList, NPC_VIOLET_HOLD_GUARD, 100.0f);
    for (Creature* guard : guardList)
    {
        guard->Despawn(1000, 1);
        guard->setVisible(true);
        guard->getAIInterface()->setReactState(REACT_AGGRESSIVE);
        guard->getAIInterface()->enterEvadeMode();
    }
}

SinclariTriggerAI::SinclariTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();
}

CreatureAIScript* SinclariTriggerAI::Create(Creature* pCreature) { return new SinclariTriggerAI(pCreature); }

//////////////////////////////////////////////////////////////////////////////////////////
/// Gossip: Sinclari
void SinclariGossip::onHello(Object* pObject, Player* pPlayer)
{
    TheVioletHoldScript* pInstance = (TheVioletHoldScript*)pPlayer->getWorldMap()->getScript();
    if (!pInstance)
        return;

    //Page 1: Textid and first menu item
    if (pInstance->getLocalData(DATA_MAIN_EVENT_STATE) == EncounterStates::NotStarted)
    {
        GossipMenu menu(pObject->getGuid(), 13853, 0);
        menu.addItem(GOSSIP_ICON_CHAT, (600), 1);
        menu.sendGossipPacket(pPlayer);
    }

    //If VioletHold is started, Sinclari has this item for people who would join.
    if (pInstance->getLocalData(DATA_MAIN_EVENT_STATE) == EncounterStates::InProgress)
    {
        GossipMenu menu(pObject->getGuid(), 13853, 0);
        menu.addItem(GOSSIP_ICON_CHAT, (602), 3);
        menu.sendGossipPacket(pPlayer);
    }
}

void SinclariGossip::onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/)
{
    TheVioletHoldScript* pInstance = (TheVioletHoldScript*)pPlayer->getWorldMap()->getScript();
    if (!pInstance)
        return;

    if (!pObject->isCreature())
        return;

    switch (Id)
    {
        case 1:
        {
            GossipMenu menu(pObject->getGuid(), 13854, 0);
            menu.addItem(GOSSIP_ICON_CHAT, (601), 2);
            menu.sendGossipPacket(pPlayer);
        } break;
        case 2:
        {
            static_cast<Creature*>(pObject)->GetScript()->DoAction(ACTION_INTRO_START);
            static_cast<Creature*>(pObject)->setNpcFlags(UNIT_NPC_FLAG_NONE);
        } break;
        case 3:
        {
            GossipMenu::senGossipComplete(pPlayer);
            pPlayer->safeTeleport(pPlayer->GetInstanceID(), 608, LocationVector(1830.531006f, 803.939758f, 44.340508f, 6.281611f));
        } break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// VHGuards AI
VHGuardsAI::VHGuardsAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();
}

CreatureAIScript* VHGuardsAI::Create(Creature* pCreature) { return new VHGuardsAI(pCreature); }

void VHGuardsAI::OnLoad()
{
    if (mInstance)
    {
        // Just incase we where Dead and Respawned mit fight make us Invisible
        if (mInstance->getLocalData(DATA_MAIN_EVENT_STATE) == EncounterStates::InProgress)
            getCreature()->setVisible(false);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Trash AI
TrashAI::TrashAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();

    switch (getCreature()->getEntry())
    {
        case NPC_AZURE_INVADER_1:
        {
            addAISpell(SPELL_CLEAVE, 75.0f, TARGET_ATTACKING, 0, 5);
            addAISpell(SPELL_IMPALE, 75.0f, TARGET_ATTACKING, 0, 4);
        } break;
        case NPC_AZURE_INVADER_2:
        case NPC_AZURE_INVADER_3:
        {
            addAISpell(SPELL_BRUTAL_STRIKE, 75.0f, TARGET_ATTACKING, 0, 5);
            addAISpell(SPELL_SUNDER_ARMOR, 75.0f, TARGET_ATTACKING, 0, 8);
        } break;
        case NPC_AZURE_SPELLBREAKER_1:
        {
            addAISpell(SPELL_ARCANE_BLAST, 75.0f, 5, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 30.0f); });
            addAISpell(SPELL_SLOW, 75.0f, 4, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 30.0f); });
        } break;
        case NPC_AZURE_SPELLBREAKER_2:
        case NPC_AZURE_SPELLBREAKER_3:
        {
            addAISpell(SPELL_CHAINS_OF_ICE, 75.0f, 7, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 30.0f); });
            addAISpell(SPELL_CONE_OF_COLD, 75.0f, TARGET_ATTACKING, 0, 4);
        } break;
        case NPC_AZURE_BINDER_1:
        {
            addAISpell(SPELL_ARCANE_EXPLOSION, 75.0f, TARGET_ATTACKING, 0, 5);
            addAISpell(SPELL_ARCANE_BARRAGE, 75.0f, 4, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 30.0f); });
        } break;
        case NPC_AZURE_BINDER_2:
        case NPC_AZURE_BINDER_3:
        {
            addAISpell(SPELL_FROST_NOVA, 75.0f, TARGET_ATTACKING, 0, 5);
            addAISpell(SPELL_FROSTBOLT, 75.0f, 4, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 40.0f); });
        } break;
        case NPC_AZURE_MAGE_SLAYER_1:
        {
            addAISpell(SPELL_ARCANE_EMPOWERMENT, 75.0f, TARGET_SELF, 0, 5);
        } break;
        case NPC_AZURE_MAGE_SLAYER_2:
        case NPC_AZURE_MAGE_SLAYER_3:
        {
            addAISpell(SPELL_SPELL_LOCK, 75.0f, 5, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 30.0f); });
        } break;
        case NPC_AZURE_RAIDER_1:
        case NPC_AZURE_RAIDER_2:
        {
            addAISpell(SPELL_CONCUSSION_BLOW, 75.0f, TARGET_ATTACKING, 0, 5);
            addAISpell(SPELL_MAGIC_REFLECTION, 75.0f, TARGET_SELF, 0, 8);
        } break;
        case NPC_AZURE_CAPTAIN_1:
        {
            addAISpell(SPELL_MORTAL_STRIKE, 75.0f, TARGET_ATTACKING, 0, 5);
            addAISpell(SPELL_WHIRLWIND_OF_STEEL, 75.0f, TARGET_SELF, 0, 8);
        } break;
        case NPC_AZURE_SORCEROR_1:
        {
            addAISpell(SPELL_ARCANE_STREAM, 33.0f, 5, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 35.0f); });
            addAISpell(SPELL_MANA_DETONATION, 33.0f, TARGET_SELF, 0, 2);
        } break;
        case NPC_AZURE_STALKER_1:
        {
            addAISpell(SPELL_TACTICAL_BLINK, 33.0f, 8, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 40.0f); });
            addAISpell(SPELL_BACKSTAB, 33.0f, 1, [this]() { return getBestUnitTarget(TargetFilter_InRangeOnly, 0.0f, 5.0f); });
        } break;
    }
    
    mlastWaypointId = 0;
}

CreatureAIScript* TrashAI::Create(Creature* pCreature) { return new TrashAI(pCreature); }

void TrashAI::AIUpdate(unsigned long /*time_passed*/)
{
    if (!_isInCombat())
    {
        if (MovementGenerator* movementGenerator = getCreature()->getMovementManager()->getCurrentMovementGenerator(MOTION_SLOT_DEFAULT))
            movementGenerator->resume(0);
    }
}

void TrashAI::waypointReached(uint32_t waypointId, uint32_t /*pathId*/)
{
    if (waypointId == mlastWaypointId)
    {
        if (mInstance && mInstance->getLocalData(DATA_MAIN_EVENT_STATE) == EncounterStates::InProgress)
        getCreature()->castSpell(mInstance->getCreatureFromData(DATA_PRISON_SEAL), SPELL_DESTROY_DOOR_SEAL, false);
    }
}

void TrashAI::SetCreatureData(uint32_t type, uint32_t data)
{
    if (type == DATA_PORTAL_LOCATION)
    {
        LocationVector const* path = nullptr;

        switch (data)
        {
            case 0:
                path = getPathFrom(FirstPortalWPs);
                break;
            case 7:
                switch (Util::getRandomInt(0, 1))
                {
                case 0:
                    path = getPathFrom(SecondPortalFirstWPs);
                    break;
                case 1:
                    path = getPathFrom(SecondPortalSecondWPs);
                    break;
                }
                break;
            case 2:
                path = getPathFrom(ThirdPortalWPs);
                break;
            case 6:
                path = getPathFrom(FourthPortalWPs);
                break;
            case 1:
                path = getPathFrom(FifthPortalWPs);
                break;
            case 5:
                path = getPathFrom(SixthPortalWPs);
                break;
            default:
                path = getPathFrom(DefaultPortalWPs);
                break;
        }

        if (path)
        {
            for (uint32_t i = 0; i <= mlastWaypointId; i++)
            {
                WaypointNode node = WaypointNode(i, path[i].getPositionX() + Util::getRandomInt(-1, 1), path[i].getPositionY() + Util::getRandomInt(-1, 1), path[i].getPositionZ(), 0, 0);
                node.moveType = WAYPOINT_MOVE_TYPE_RUN;
                addWaypoint(1, node);
            }

            getCreature()->SetSpawnLocation(path[mlastWaypointId].getPositionX(), path[mlastWaypointId].getPositionY(), path[mlastWaypointId].getPositionZ(), float(M_PI));
            
            // Make us Run
            getCreature()->setMoveWalk(false);

            // Move
            getCreature()->getMovementManager()->movePath(*getCustomPath(1), false);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// DefenseSystem AI
VHDefenseSystemAI::VHDefenseSystemAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    // Instance Script
    mInstance = getInstanceScript();
}

CreatureAIScript* VHDefenseSystemAI::Create(Creature* pCreature) { return new VHDefenseSystemAI(pCreature); }

void VHDefenseSystemAI::OnLoad()
{
    scriptEvents.addEvent(1, 4 * TimeVarsMs::Second);
    scriptEvents.addEvent(2, 5 * TimeVarsMs::Second);
    scriptEvents.addEvent(3, 6 * TimeVarsMs::Second);

    // Despawn us After 7 Seconds
    despawn(7000, 0);
}

void VHDefenseSystemAI::AIUpdate(unsigned long time_passed)
{
    scriptEvents.updateEvents(time_passed, getScriptPhase());

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case 1:
            {
                getCreature()->castSpell(getCreature(), SPELL_ARCANE_LIGHTNING_DAMAGE, false);
                getCreature()->castSpell(getCreature(), SPELL_ARCANE_LIGHTNING_DUMMY, false);
            } break;
            case 2:
            {
                getCreature()->castSpell(getCreature(), SPELL_ARCANE_LIGHTNING_INSTAKILL, false);
            } break;
            case 3:
            {
                getCreature()->castSpell(getCreature(), SPELL_ARCANE_LIGHTNING_INSTAKILL, false);
            } break;
            default:
                break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Activation Crystal AI
ActivationCrystalAI::ActivationCrystalAI(GameObject* pGameObject) : GameObjectAIScript(pGameObject)
{
    // Instance Script
    mInstance = getInstanceScript();
}

GameObjectAIScript* ActivationCrystalAI::Create(GameObject* pGameObject) { return new ActivationCrystalAI(pGameObject); }

void ActivationCrystalAI::OnSpawn()
{
    _gameobject->addFlags(GO_FLAG_NOT_SELECTABLE);
}

void ActivationCrystalGossip::onHello(Object* /*object*/, Player* player)
{
    player->castSpell(player, SPELL_CRYSTAL_ACTIVATION, true);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: 58040 - Destroy Door Seal
void DestroyDoorSeal::onAuraCreate(Aura* aur)
{
    if (aur->GetUnitCaster() && aur->GetUnitCaster()->getWorldMap()->getScript())
        mInstance = aur->GetUnitCaster()->getWorldMap()->getScript();
}

SpellScriptExecuteState DestroyDoorSeal::onAuraPeriodicTick(Aura* aur, AuraEffectModifier* /*aurEff*/, float_t* /*damage*/)
{
    if (aur->GetUnitCaster() && aur->GetUnitCaster()->isAlive())
    {
        if (mInstance)
        {
            if (uint32_t integrity = mInstance->getLocalData(DATA_DOOR_INTEGRITY))
                mInstance->setLocalData(DATA_DOOR_INTEGRITY, integrity - 1);
        }
    }
    return SpellScriptExecuteState::EXECUTE_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: 57912,58152,57930 Arcane Lightning Targetting
void ArcaneLightning::filterEffectTargets(Spell* spell, uint8_t /*effIndex*/, std::vector<uint64_t>* effectTargets)
{
    effectTargets->clear();

    for (const auto& itr : spell->getUnitCaster()->getInRangeObjectsSet())
    {
        if (itr->isCreature() && itr->getEntry() != NPC_PRISON_SEAL && itr->ToCreature()->getAIInterface()->canOwnerAttackUnit(spell->getUnitCaster()))
        {
            effectTargets->push_back(itr->getGuid());
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Setup
void SetupTheVioletHold(ScriptMgr* mgr)
{
// Instance
    mgr->register_instance_script(MAP_VIOLET_HOLD, &TheVioletHoldScript::Create);

// Bosses
    mgr->register_creature_script(NPC_XEVOZZ, &XevozzAI::Create);
    mgr->register_creature_script(NPC_DUMMY_XEVOZZ, &XevozzAI::Create);
    mgr->register_creature_script(NPC_ETHEREALSPHERE, &EtherealSphereAI::Create);

    mgr->register_creature_script(NPC_LAVANTHOR, &LavanthorAI::Create);
    mgr->register_creature_script(NPC_DUMMY_LAVANTHOR, &LavanthorAI::Create);

    mgr->register_creature_script(NPC_ICHORON, &IchronAI::Create);
    mgr->register_creature_script(NPC_DUMMY_ICHORON, &IchronAI::Create);
    mgr->register_creature_script(NPC_ICHOR_GLOBULE, &IchronGlobuleAI::Create);
    mgr->register_achievement_criteria_script(7320, new achievement_Dehydration);

    mgr->register_creature_script(NPC_ZURAMAT, &ZuramatAI::Create);
    mgr->register_creature_script(NPC_DUMMY_ZURAMAT, &ZuramatAI::Create);
    mgr->register_creature_script(NPC_VOID_SENTRY, &VoidSentryAI::Create);
    mgr->register_achievement_criteria_script(7587, new achievement_void_dance);

    mgr->register_creature_script(NPC_EREKEM, &ErekemAI::Create);
    mgr->register_creature_script(NPC_DUMMY_EREKEM, &ErekemAI::Create);
    mgr->register_creature_script(NPC_EREKEM_GUARD, &ErekemGuardAI::Create);

    mgr->register_creature_script(NPC_MORAGG, &MoraggAI::Create);
    mgr->register_creature_script(NPC_DUMMY_MORAGG, &MoraggAI::Create);

    mgr->register_creature_script(NPC_CYANIGOSA, &CyangosaAI::Create);

// Sinclari
    mgr->register_creature_script(NPC_SINCLARI, &SinclariAI::Create);
    GossipScript* SinclariGossipScript = new SinclariGossip();
    mgr->register_creature_gossip(NPC_SINCLARI, SinclariGossipScript);
    mgr->register_creature_script(NPC_SINCLARI_TRIGGER, &SinclariTriggerAI::Create);

// Portals
    mgr->register_creature_script(NPC_TELEPORTATION_PORTAL_INTRO, &IntroPortalAI::Create);
    mgr->register_creature_script(NPC_TELEPORTATION_PORTAL, &CommonPortalAI::Create);
    mgr->register_creature_script(NPC_TELEPORTATION_PORTAL_ELITE, &ElitePortalAI::Create);

// Trash
    // Intro
    uint32_t entrys1[] = { NPC_AZURE_INVADER_1, NPC_AZURE_MAGE_SLAYER_1, NPC_AZURE_BINDER_1, 0 };
    mgr->register_creature_script(entrys1, &TrashAI::Create);
    // Common
    uint32_t entrys2[] = { NPC_PORTAL_GUARDIAN, NPC_PORTAL_KEEPER, NPC_AZURE_SPELLBREAKER_1, NPC_AZURE_INVADER_2, NPC_AZURE_SPELLBREAKER_2, NPC_AZURE_MAGE_SLAYER_2, NPC_AZURE_BINDER_2, 0 };
    mgr->register_creature_script(entrys2, &TrashAI::Create);
    //Elite
    uint32_t entrys3[] = { NPC_AZURE_CAPTAIN_1, NPC_AZURE_RAIDER_1, NPC_AZURE_STALKER_1, NPC_AZURE_SORCEROR_1, 0 };
    mgr->register_creature_script(entrys3, &TrashAI::Create);
    // Boss Waves
    mgr->register_creature_script(NPC_SABOTEOUR, &AzureSaboteurAI::Create);

// Guards
    mgr->register_creature_script(NPC_VIOLET_HOLD_GUARD, &VHGuardsAI::Create);

// Defense System
    mgr->register_creature_script(NPC_DEFENSE_SYSTEM, &VHDefenseSystemAI::Create);

// Activation Crystall
    mgr->register_gameobject_script(GO_ACTIVATION_CRYSTAL, &ActivationCrystalAI::Create);
    mgr->register_go_gossip(GO_ACTIVATION_CRYSTAL, new ActivationCrystalGossip());

// Spells
    mgr->register_spell_script(SPELL_DESTROY_DOOR_SEAL, new DestroyDoorSeal);
    uint32_t entrys4[] = { SPELL_ARCANE_LIGHTNING_DAMAGE, SPELL_ARCANE_LIGHTNING_INSTAKILL, SPELL_ARCANE_LIGHTNING_DUMMY, 0 };
    mgr->register_spell_script(entrys4, new ArcaneLightning);
    mgr->register_spell_script(Ichron::SPELL_MERGE, new IchronMerge);
    mgr->register_spell_script(Ichron::SPELL_PROTECTIVE_BUBBLE, new IchronBubble);
}
