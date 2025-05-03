/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_TrialOfTheCrusader.hpp"

#include "Northrend_Beasts.hpp"
#include "Lord_Jaraxxus.hpp"
#include "Faction_Champion.hpp"
#include "Twin_Valkyr.hpp"
#include "Anubarak.hpp"
#include "Management/AchievementMgr.h"
#include "Management/WeatherMgr.hpp"
#include "Management/Gossip/GossipMenu.hpp"
#include "Movement/MovementManager.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "CommonTime.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
//Trial Of The Crusader Instance
TrialOfTheCrusaderInstanceScript::TrialOfTheCrusaderInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
{
    setBossNumber(EncounterCount);
    setupInstanceData(creatureData, gameObjectData);

    // Northend Beasts Encounter
    NorthrendBeasts = EncounterStates::NotStarted;

    TrialCounter                    = 50;

    // Achievement stuff
    SnoboldCount                    = 0;
    NotOneButTwoJormungarsTimer     = 0;
    ResilienceWillFixItTimer        = 0;
    MistressOfPainCount             = 0;
    CrusadersSpecialState           = false;
    TributeToImmortalityEligible    = true;
}

InstanceScript* TrialOfTheCrusaderInstanceScript::Create(WorldMap* pMapMgr) { return new TrialOfTheCrusaderInstanceScript(pMapMgr); }

void TrialOfTheCrusaderInstanceScript::UpdateEvent()
{
    if (getLocalData(TYPE_NORTHREND_BEASTS) == SNAKES_SPECIAL && NotOneButTwoJormungarsTimer)
    {
        if (NotOneButTwoJormungarsTimer <= getUpdateFrequency())
            NotOneButTwoJormungarsTimer = 0;
        else
            NotOneButTwoJormungarsTimer -= getUpdateFrequency();
    }

    if (CrusadersSpecialState && ResilienceWillFixItTimer)
    {
        if (ResilienceWillFixItTimer <= getUpdateFrequency())
            ResilienceWillFixItTimer = 0;
        else
            ResilienceWillFixItTimer -= getUpdateFrequency();
    }
}

void TrialOfTheCrusaderInstanceScript::OnAreaTrigger(Player* pPlayer, uint32_t pAreaId)
{
    if (pAreaId == 5475)
    {
        if (Creature* anubarak = getCreatureFromData(DATA_ANUBARAK))
            anubarak->GetScript()->DoAction(5475);
    }
}

void TrialOfTheCrusaderInstanceScript::OnPlayerEnter(Player* player)
{
    if (getInstance()->isHeroic())
    {
        getInstance()->getWorldStatesHandler().SetWorldStateForZone(player->getZoneId(), 0, UPDATE_STATE_UI_SHOW, 1);
        getInstance()->getWorldStatesHandler().SetWorldStateForZone(player->getZoneId(), 0, UPDATE_STATE_UI_COUNT, getLocalData(TYPE_COUNTER));
    }
    else
        getInstance()->getWorldStatesHandler().SetWorldStateForZone(player->getZoneId(), 0, UPDATE_STATE_UI_SHOW, 0);
}

void TrialOfTheCrusaderInstanceScript::OnPlayerDeath(Player* /*pVictim*/, Unit* /*pKiller*/)
{
    if (getWorldMap()->isCombatInProgress())
        TributeToImmortalityEligible = false;
}

void TrialOfTheCrusaderInstanceScript::DoAction(int32_t action)
{
    switch (action)
    {
        case ACTION_OPEN_GATE:
        {
            if (auto door = getGameObjectFromData(DATA_MAIN_GATE))
                useDoorOrButton(door);
        } break;
        case ACTION_CLOSE_GATE:
        {
            if (auto door = getGameObjectFromData(DATA_MAIN_GATE))
                closeDoorOrButton(door);
        } break;
        case ACTION_ENCOUNTER_STARTED:
        {
            if (auto door = getGameObjectFromData(DATA_EAST_PORTCULLIS))
                useDoorOrButton(door);
        } break;
        case ACTION_ENCOUNTER_STOPPED:
        {
            if (auto door = getGameObjectFromData(DATA_EAST_PORTCULLIS))
                closeDoorOrButton(door);
        } break;
    }
    
}

bool TrialOfTheCrusaderInstanceScript::setBossState(uint32_t type, EncounterStates state)
{
    if (!InstanceScript::setBossState(type, state))
        return false;

    switch (state)
    {
        case EncounterStates::InProgress:
            DoAction(ACTION_ENCOUNTER_STARTED);
            break;
        case EncounterStates::Performed:
        case EncounterStates::Failed:
            DoAction(ACTION_ENCOUNTER_STOPPED);
            break;
    }

    switch (type)
    {
        case DATA_JARAXXUS:
        {
            if (state == EncounterStates::Failed)
            {
                if (Creature* fordring = getCreatureFromData(DATA_FORDRING))
                    fordring->GetScript()->DoAction(ACTION_JARAXXUS_WIPE);

                MistressOfPainCount = 0;
            }
            else if (state == EncounterStates::Performed)
            {
                if (Creature* fordring = getCreatureFromData(DATA_FORDRING))
                    fordring->GetScript()->DoAction(ACTION_JARAXXUS_DEFEATED);
            }
        } break;
        case DATA_FACTION_CRUSADERS:
        {
            switch (state)
            {
                case EncounterStates::InProgress:
                {
                    ResilienceWillFixItTimer = 0;
                } break;
                case EncounterStates::Failed:
                {
                    CrusadersSpecialState = false;
                    if (Creature* fordring = getCreatureFromData(DATA_FORDRING))
                        fordring->GetScript()->DoAction(ACTION_FACTION_WIPE);
                } break;
                case EncounterStates::Performed:
                {
                    updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_DEFEAT_FACTION_CHAMPIONS);
                    if (ResilienceWillFixItTimer > 0)
                        updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_CHAMPIONS_KILLED_IN_MINUTE);

                    //DoRespawnGameObject(GetGuidData(DATA_CRUSADERS_CHEST), 7_days);

                    if (GameObject* cache = getGameObjectFromData(DATA_CRUSADERS_CHEST))
                        cache->removeFlags(GO_FLAG_NOT_SELECTABLE);

                    if (Creature* fordring = getCreatureFromData(DATA_FORDRING))
                        fordring->GetScript()->DoAction(ACTION_CHAMPIONS_DEFEATED);
                } break;
                default:
                    break;
            }
        } break;
        case DATA_TWIN_VALKIRIES:
        {
            // Cleanup chest
            if (GameObject* cache = getGameObjectFromData(DATA_CRUSADERS_CHEST))
                cache->Delete();

            switch (state)
            {
                case EncounterStates::Failed:
                    if (Creature* fordring = getCreatureFromData(DATA_FORDRING))
                        fordring->GetScript()->DoAction(ACTION_VALKYR_WIPE);
                    break;
                case EncounterStates::Performed:
                    if (Creature* fordring = getCreatureFromData(DATA_FORDRING))
                        fordring->GetScript()->DoAction(ACTION_VALKYR_DEFEATED);
                    break;
                default:
                    break;
            }
        } break;
        case DATA_ANUBARAK:
        {
            switch (state)
            {
                case EncounterStates::Performed:
                {
                    uint32_t tributeChest = 0;
                    if (getInstance()->getDifficulty() == InstanceDifficulty::RAID_10MAN_HEROIC)
                    {
                        if (TrialCounter >= 50)
                            tributeChest = GO_TRIBUTE_CHEST_10H_99;
                        else
                        {
                            if (TrialCounter >= 45)
                                tributeChest = GO_TRIBUTE_CHEST_10H_50;
                            else
                            {
                                if (TrialCounter >= 25)
                                    tributeChest = GO_TRIBUTE_CHEST_10H_45;
                                else
                                    tributeChest = GO_TRIBUTE_CHEST_10H_25;
                            }
                        }
                    }
                    else if (getInstance()->getDifficulty() == InstanceDifficulty::RAID_25MAN_HEROIC)
                    {
                        if (TrialCounter >= 50)
                            tributeChest = GO_TRIBUTE_CHEST_25H_99;
                        else
                        {
                            if (TrialCounter >= 45)
                                tributeChest = GO_TRIBUTE_CHEST_25H_50;
                            else
                            {
                                if (TrialCounter >= 25)
                                    tributeChest = GO_TRIBUTE_CHEST_25H_45;
                                else
                                    tributeChest = GO_TRIBUTE_CHEST_25H_25;
                            }
                        }
                    }

                    if (tributeChest)
                        if (Creature* tirion = getCreatureFromData(DATA_FORDRING))
                            if (GameObject* chest = tirion->summonGameObject(tributeChest, LocationVector(805.62f, 134.87f, 142.16f, 3.27f), QuaternionData(), 7 * TimeVarsMs::Day))
                                chest->setRespawnTime(chest->getRespawnDelay());
                }  break;
                default:
                    break;
            }
        } break;
        default:
            break;
    }

    // Trial Counting
    if (state == EncounterStates::Failed)
    {
        if (getInstance()->isHeroic())
        {
            // decrease attempt counter at wipe
            --TrialCounter;
            getInstance()->getWorldStatesHandler().SetWorldStateForZone(0, 0, UPDATE_STATE_UI_COUNT, getLocalData(TYPE_COUNTER));

            // if theres no more attemps allowed
            if (!TrialCounter)
            {
                if (Creature* anubarak = getCreatureFromData(DATA_ANUBARAK))
                    anubarak->Despawn(0, 0);
            }
        }
    }

    return true;
}

void TrialOfTheCrusaderInstanceScript::writeSaveDataExtended(std::ostringstream& data)
{
    data << TrialCounter;
}

void TrialOfTheCrusaderInstanceScript::readSaveDataExtended(std::istringstream& data)
{
    data >> TrialCounter;
}

void TrialOfTheCrusaderInstanceScript::setLocalData(uint32_t type, uint32_t data)
{
    switch (type)
    {
        case TYPE_NORTHREND_BEASTS:
        {
            NorthrendBeasts = data;
            switch (data)
            {
                case GORMOK_IN_PROGRESS:
                {
                    setBossState(DATA_NORTHREND_BEASTS, EncounterStates::InProgress);
                } break;
                case GORMOK_DONE:
                {
                    if (Creature* tirion = getCreatureFromData(DATA_FORDRING))
                        tirion->GetScript()->DoAction(ACTION_START_JORMUNGARS);
                } break;
                case SNAKES_IN_PROGRESS:
                {
                    NotOneButTwoJormungarsTimer = 0;
                } break;
                case SNAKES_SPECIAL:
                {
                    NotOneButTwoJormungarsTimer = 10 * TimeVarsMs::Second;
                } break;
                case SNAKES_DONE:
                {
                    if (NotOneButTwoJormungarsTimer > 0)
                        updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_WORMS_KILLED_IN_10_SECONDS);
                    if (Creature* tirion = getCreatureFromData(DATA_FORDRING))
                        tirion->GetScript()->DoAction(ACTION_START_ICEHOWL);
                } break;
                case ICEHOWL_DONE:
                {
                    NorthrendBeasts = EncounterStates::Performed;
                    setBossState(DATA_NORTHREND_BEASTS, EncounterStates::Performed);
                    setLocalData(DATA_DESPAWN_SNOBOLDS, 0);

                    if (Creature* combatStalker = getCreatureFromData(DATA_BEASTS_COMBAT_STALKER))
                        combatStalker->Despawn(2000, 0);

                    if (Creature* fordring = getCreatureFromData(DATA_FORDRING))
                        fordring->GetScript()->DoAction(ACTION_NORTHREND_BEASTS_DEFEATED);
                } break;
                case EncounterStates::Failed:
                {
                    setBossState(DATA_NORTHREND_BEASTS, EncounterStates::Failed);
                    if (Creature* tirion = getCreatureFromData(DATA_FORDRING))
                        tirion->GetScript()->DoAction(ACTION_NORTHREND_BEASTS_WIPE);
                    SnoboldCount = 0;
                } break;
                default:
                    break;
            }
        } break;
        case TYPE_COUNTER:
        {
            TrialCounter = data;
        } break;
        case DATA_DESPAWN_SNOBOLDS:
        {
            for (uint32_t const guid : snoboldGUIDS)
                if (Creature* snobold = GetCreatureByGuid(guid))
                    snobold->Despawn(2000, 0);

            snoboldGUIDS.clear();
        } break;
            //Achievements
        case DATA_FACTION_CRUSADERS: // Achivement Resilience will Fix
        {
            ResilienceWillFixItTimer = 60 * TimeVarsMs::Second;
            CrusadersSpecialState = true;
        } break;
        default:
            break;
    }
}

uint32_t TrialOfTheCrusaderInstanceScript::getLocalData(uint32_t type) const
{
    switch (type)
    {
        case TYPE_NORTHREND_BEASTS:
            return NorthrendBeasts;
        case TYPE_COUNTER:
            return TrialCounter;
        default:
            break;
    }

    return 0;
}

void TrialOfTheCrusaderInstanceScript::OnCreaturePushToWorld(Creature* pCreature)
{
    WoWGuid guid = pCreature->getGuid();

    switch (pCreature->getEntry())
    {
        case NPC_VALKYR_STALKER_DARK:
        case NPC_VALKYR_STALKER_LIGHT:
        {
            stalkerGUIDS.push_back(guid.getGuidLowPart());
        } break;
        case NPC_SNOBOLD_VASSAL:
        {
            ++SnoboldCount;
            snoboldGUIDS.push_back(guid.getGuidLowPart());
        } break;
        case jaraxxus::NPC_MISTRESS_OF_PAIN:
        {
            ++MistressOfPainCount;
        } break;
        default:
            break;
    }
}

void TrialOfTheCrusaderInstanceScript::OnGameObjectPushToWorld(GameObject* pGameObject)
{
    switch (pGameObject->getEntry())
    {
        case GO_ARGENT_COLISEUM_FLOOR:
            {
                if (getBossState(DATA_LICH_KING) == EncounterStates::Performed)
                    static_cast<GameObject_Destructible*>(pGameObject)->setDestructibleState(GO_DESTRUCTIBLE_DAMAGED);
            } break;
            default:
                break;
    }
}

void TrialOfTheCrusaderInstanceScript::OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/)
{
    switch (pVictim->getEntry())
    {
        case NPC_SNOBOLD_VASSAL:
        {
            --SnoboldCount;
        } break;
        case jaraxxus::NPC_MISTRESS_OF_PAIN:
        {
            --MistressOfPainCount;
        } break;
        default:
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Gossip: Barrett Ramsey
void BarretGossip::onHello(Object* pObject, Player* plr)
{
    GossipMenu menu(pObject->getGuid(), 14500);

    InstanceScript* script = pObject->getWorldMap()->getInstance()->getScript();

    if (script)
    {
        if (script->getBossState(DATA_NORTHREND_BEASTS) != EncounterStates::Performed)
            menu.addItem(GOSSIP_ICON_CHAT, MENUID_NORTHREND_BEASTS, 1);
        
        if (script->getBossState(DATA_NORTHREND_BEASTS) == EncounterStates::Performed && (script->getBossState(DATA_JARAXXUS) != EncounterStates::Performed))
            menu.addItem(GOSSIP_ICON_CHAT, MENUID_JARAXXUS, 2);

        if (script->getBossState(DATA_JARAXXUS) == EncounterStates::Performed && (script->getBossState(DATA_FACTION_CRUSADERS) != EncounterStates::Performed))
            menu.addItem(GOSSIP_ICON_CHAT, MENUID_FACTION_CHAMPIONS, 3);
        
        
        if (script->getBossState(DATA_FACTION_CRUSADERS) == EncounterStates::Performed && (script->getBossState(DATA_TWIN_VALKIRIES) != EncounterStates::Performed))
            menu.addItem(GOSSIP_ICON_CHAT, MENUID_VALKYR, 4);
        
        if (script->getBossState(DATA_TWIN_VALKIRIES) == EncounterStates::Performed && (script->getBossState(DATA_LICH_KING) != EncounterStates::Performed))
            menu.addItem(GOSSIP_ICON_CHAT, MENUID_LK, 5);
    }
    menu.sendGossipPacket(plr);
}

void BarretGossip::encounterStarted(uint32_t action, Creature* fordring, Creature* self)
{
    fordring->GetScript()->DoAction(action);

    // Clear NPC FLAGS
    self->setNpcFlags(UNIT_NPC_FLAG_NONE);

    // Move
    self->getMovementManager()->moveAlongSplineChain(POINT_BARRETT_DESPAWN, SPLINE_INITIAL_MOVEMENT, false);
}

void BarretGossip::onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/)
{
    TrialOfTheCrusaderInstanceScript* pInstance = (TrialOfTheCrusaderInstanceScript*)pPlayer->getWorldMap()->getScript();
    if (!pInstance)
        return;

    Creature* fordring = pInstance->getCreatureFromData(DATA_FORDRING);
    if (!fordring || !fordring->GetScript())
        return;

    switch (Id)
    {
        case 1:
            encounterStarted(ACTION_START_GORMOK, fordring, static_cast<Creature*>(pObject));
            break;
        case 2:
            encounterStarted(ACTION_START_JARAXXUS_EVENT, fordring, static_cast<Creature*>(pObject));
            break;
        case 3:
            encounterStarted(ACTION_START_CHAMPIONS, fordring, static_cast<Creature*>(pObject));
            break;
        case 4:
            encounterStarted(ACTION_START_VALKYR, fordring, static_cast<Creature*>(pObject));
            break;
        case 5:
            encounterStarted(ACTION_START_LK_EVENT, fordring, static_cast<Creature*>(pObject));
            break;
    }

    GossipMenu::senGossipComplete(pPlayer);
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Barrett Ramsey
BarretAI::BarretAI(Creature* pCreature) : CreatureAIScript(pCreature) { }

CreatureAIScript* BarretAI::Create(Creature* pCreature) { return new BarretAI(pCreature); }

void BarretAI::OnReachWP(uint32_t type, uint32_t id)
{
    if (type != SPLINE_CHAIN_MOTION_TYPE)
        return;

    if (id == POINT_BARRETT_DESPAWN)
        despawn();
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Tirion Fordring
TirionAI::TirionAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mInstance = (TrialOfTheCrusaderInstanceScript*)getInstanceScript();

    summons.clear();

    mFactionLeaderData = 0;
    mJormungarsSummoned = false;
    mIcehowlSummoned = false;
}

CreatureAIScript* TirionAI::Create(Creature* pCreature) { return new TirionAI(pCreature); }

void TirionAI::OnLoad()
{
    mFactionLeaderData = mInstance->getInstance()->getTeamIdInInstance() == TEAM_ALLIANCE ? DATA_VARIAN : DATA_GARROSH;
    mJormungarsSummoned = false;
    mIcehowlSummoned = false;
    addAIFunction(&TirionAI::handleBarrettSummon, DoOnceScheduler(1s));
}

void TirionAI::onSummonedCreature(Creature* summon)
{
    summons.summon(summon);
}

void TirionAI::DoAction(int32_t action)
{
    switch (action)
    {
        case ACTION_START_GORMOK:
        {
            addMessage(Message(TIRION_SAY_WELCOME), DoOnceScheduler());
            addAIFunction(&TirionAI::GormokIntro, DoOnceScheduler(24s));
        }
        break;
        case ACTION_START_GORMOK_FAIL:
        {
            addAIFunction(&TirionAI::GormokIntro, DoOnceScheduler(1s));
        } break;
        case ACTION_START_JORMUNGARS:
        {
            if (mJormungarsSummoned)
                return;

            mJormungarsSummoned = true;
            addAIFunction(&TirionAI::JormungarsIntro, DoOnceScheduler());
        } break;
        case ACTION_START_ICEHOWL:
        {
            if (mIcehowlSummoned)
                return;

            mIcehowlSummoned = true;
            addAIFunction(&TirionAI::IcehowlIntro, DoOnceScheduler());
        } break;
        case ACTION_NORTHREND_BEASTS_WIPE:
        {
            mJormungarsSummoned = false;
            mIcehowlSummoned = false;

            addMessage(Message(TIRION_SAY_BEASTS_WIPE), DoOnceScheduler());
            addAIFunction(&TirionAI::handleBarrettSummon, DoOnceScheduler(13s));
        } break;
        case ACTION_NORTHREND_BEASTS_DEFEATED:
        {
            addMessage(Message(TIRION_SAY_BEASTS_DONE), DoOnceScheduler());
            addEmote(Emote(EMOTE_ONESHOT_EXCLAMATION), DoOnceScheduler(2s));
            addAIFunction(&TirionAI::handleBarrettSummon, DoOnceScheduler(6s));
        } break;
        case ACTION_START_JARAXXUS_EVENT:
        {
            addAIFunction(&TirionAI::JaraxxusIntro, DoOnceScheduler(1s));
        } break;
        case ACTION_SUMMON_JARAXXUS:
        {
            summonCreature(NPC_JARAXXUS, JaraxxusSpawnPosition);
        } break;
        case ACTION_KILL_JARAXXUS:
        {
            addMessage(Message(TIRION_SAY_KILL_JARAXXUS), DoOnceScheduler(6s));
            addEmote(Emote(EMOTE_ONESHOT_TALK_NOSHEATHE), DoOnceScheduler(8s));
        } break;
        case ACTION_JARAXXUS_DEFEATED:
        {
            addAIFunction(&TirionAI::Lamet, DoOnceScheduler(7s));
        } break;
        case ACTION_JARAXXUS_WIPE:
        {
            addAIFunction(&TirionAI::handleBarrettSummon, DoOnceScheduler(30s));
        } break;

        case ACTION_START_CHAMPIONS:
        {
            addMessage(Message(TIRION_SAY_CHAMPIONS), DoOnceScheduler());

            uint32_t data = mInstance->getInstance()->getTeamIdInInstance() == TEAM_ALLIANCE ? DATA_GARROSH : DATA_VARIAN;
            if (Creature* otherFactionLeader = getInstanceScript()->getCreatureFromData(data))
                otherFactionLeader->GetScript()->DoAction(ACTION_START_CHAMPIONS);

            addMessage(Message(TIRION_SAY_ALLOW_COMBAT), DoOnceScheduler(26s));
            addAIFunction(&TirionAI::summonChampions, DoOnceScheduler(26s + 7s));
            break;
        }
        case ACTION_FACTION_WIPE:
        {
            addAIFunction(&TirionAI::handleBarrettSummon, DoOnceScheduler(4s));
        } break;
        case ACTION_START_CHAMPIONS_ENGAGE:
        {
            uint32_t data = mInstance->getInstance()->getTeamIdInInstance() == TEAM_ALLIANCE ? DATA_GARROSH : DATA_VARIAN;
            if (Creature* otherFactionLeader = getInstanceScript()->getCreatureFromData(data))
                otherFactionLeader->GetScript()->DoAction(ACTION_START_CHAMPIONS_ENGAGE);

            addAIFunction(&TirionAI::summonChampions, DoOnceScheduler(3s));
            break;
        }
        case ACTION_CHAMPIONS_DEFEATED:
        {
            addMessage(Message(TIRION_SAY_TRAGIC_VICTORY), DoOnceScheduler(7s));
            addAIFunction(&TirionAI::handleBarrettSummon, DoOnceScheduler(31s));
        } break;
        case ACTION_START_VALKYR:
        {
            addMessage(Message(TIRION_SAY_WORK_TOGETHER), DoOnceScheduler());
            addAIFunction(&TirionAI::valkyrSummon, DoOnceScheduler(17s));
        } break;
        case ACTION_START_VALKYR_ENGAGE:
        {
            addMessage(Message(TIRION_SAY_GAME_BEGIN), DoOnceScheduler());
            addAIFunction(&TirionAI::valkyrSummon, DoOnceScheduler(5s));
        } break;
        case ACTION_VALKYR_WIPE:
        {
            addAIFunction(&TirionAI::handleBarrettSummon, DoOnceScheduler(6s));
            summons.despawnEntry(NPC_LIGHT_ESSENCE);
            summons.despawnEntry(NPC_DARK_ESSENCE);
        } break;
        case ACTION_VALKYR_DEFEATED:
        {
            addAIFunction(&TirionAI::handleBarrettSummon, DoOnceScheduler(4s));

            if (Creature* factionLeader = getInstanceScript()->getCreatureFromData(mFactionLeaderData))
                factionLeader->GetScript()->DoAction(ACTION_VALKYR_DEFEATED);

            summons.despawnEntry(NPC_LIGHT_ESSENCE);
            summons.despawnEntry(NPC_DARK_ESSENCE);
        } break;
        case ACTION_START_LK_EVENT:
        {
            addMessage(Message(TIRION_SAY_UNITED), DoOnceScheduler());
            sWeatherMgr.sendWeatherForZone(6, 0.5f, AREA_TRIAL_OF_THE_CRUSADER);
            addAIFunction(&TirionAI::lickkingChallenge, DoOnceScheduler(19s));
            addMessage(Message(TIRION_SAY_ARTHAS), DoOnceScheduler(26s));
        } break;
        case ACTION_LK_EVENT_FINISHED:
        {
            addAIFunction(&TirionAI::lichkingSouls, DoOnceScheduler(2s));
        } break;
    }
}

void TirionAI::GormokIntro(CreatureAIFunc pThis)
{
    TirionAI* script = static_cast<TirionAI*>(this);
    if (script && script->getInstanceScript())
    {
        sendDBChatMessageByIndex(TIRION_SAY_GORMOK);

        if (Creature* factionLeader = getInstanceScript()->getCreatureFromData(mFactionLeaderData))
            factionLeader->GetScript()->DoAction(ACTION_START_GORMOK);

        addEmote(Emote(EMOTE_ONESHOT_EXCLAMATION), DoOnceScheduler(6s));
        addAIFunction(&TirionAI::GormokSpawn, DoOnceScheduler(12s));
    }
}

void TirionAI::GormokSpawn(CreatureAIFunc pThis)
{
    TirionAI* script = static_cast<TirionAI*>(this);
    if (script && script->getInstanceScript())
    {
        getInstanceScript()->DoAction(ACTION_OPEN_GATE);
        summonCreature(NPC_GORMOK, NorthrendBeastsSpawnPositions[0], CORPSE_TIMED_DESPAWN, 12 * TimeVarsMs::Second);
    }
}

void TirionAI::JormungarsIntro(CreatureAIFunc pThis)
{
    sendDBChatMessageByIndex(TIRION_SAY_JORMUNGARS);
    addEmote(Emote(EMOTE_ONESHOT_EXCLAMATION), DoOnceScheduler(7s));
    addAIFunction(&TirionAI::JormungarsSpawn, DoOnceScheduler(1s));
}

void TirionAI::JormungarsSpawn(CreatureAIFunc pThis)
{
    TirionAI* script = static_cast<TirionAI*>(this);
    if (script && script->getInstanceScript())
    {
        getInstanceScript()->DoAction(ACTION_OPEN_GATE);
        summonCreature(NPC_DREADSCALE, NorthrendBeastsSpawnPositions[1]);
    }
}

void TirionAI::IcehowlIntro(CreatureAIFunc pThis)
{
    sendDBChatMessageByIndex(TIRION_SAY_ICEHOWL);
    addEmote(Emote(EMOTE_ONESHOT_EXCLAMATION), DoOnceScheduler(6s));
    addAIFunction(&TirionAI::IcehowlSpawn, DoOnceScheduler(1s));
}

void TirionAI::IcehowlSpawn(CreatureAIFunc pThis)
{
    TirionAI* script = static_cast<TirionAI*>(this);
    if (script && script->getInstanceScript())
    {
        getInstanceScript()->DoAction(ACTION_OPEN_GATE);
        summonCreature(NPC_ICEHOWL, NorthrendBeastsSpawnPositions[0], CreatureSummonDespawnType::DEAD_DESPAWN);
    }
}

void TirionAI::JaraxxusIntro(CreatureAIFunc pThis)
{
    addMessage(Message(TIRION_SAY_WILFRED), DoOnceScheduler());
    addAIFunction(&TirionAI::JaraxxusSpawn, DoOnceScheduler(7s));
}

void TirionAI::JaraxxusSpawn(CreatureAIFunc pThis)
{
    TirionAI* script = static_cast<TirionAI*>(this);
    if (script && script->getInstanceScript())
    {
        getInstanceScript()->DoAction(ACTION_OPEN_GATE);
        summonCreature(NPC_FIZZLEBANG, WilfredSpawnPosition, CreatureSummonDespawnType::CORPSE_TIMED_DESPAWN, 16 * TimeVarsMs::Second);
    }
}

void TirionAI::Lamet(CreatureAIFunc pThis)
{
    addMessage(Message(TIRION_SAY_LAMENT), DoOnceScheduler());

    if (Creature* varian = getInstanceScript()->getCreatureFromData(DATA_VARIAN))
        varian->GetScript()->DoAction(ACTION_JARAXXUS_DEFEATED);

    if (Creature* garrosh = getInstanceScript()->getCreatureFromData(DATA_GARROSH))
        garrosh->GetScript()->DoAction(ACTION_JARAXXUS_DEFEATED);

    addMessage(Message(TIRION_SAY_CALM_DOWN), DoOnceScheduler(33s));
    addAIFunction(&TirionAI::handleBarrettSummon, DoOnceScheduler(53s));
}

void TirionAI::summonChampions(CreatureAIFunc pThis)
{
    if (Creature* factitonController = summonCreature(NPC_CHAMPIONS_CONTROLLER, ToCCommonLoc[1]))
        factitonController->GetScript()->DoAction(champions::ACTION_SUMMON);

    addAIFunction(&TirionAI::ChampionsStart, DoOnceScheduler(3s));
}

void TirionAI::ChampionsStart(CreatureAIFunc pThis)
{
    if (Creature* factitonController = getInstanceScript()->getCreatureFromData(DATA_FACTION_CRUSADERS))
        factitonController->GetScript()->DoAction(champions::ACTION_START);
}

void TirionAI::valkyrSummon(CreatureAIFunc pThis)
{
    Creature* lightbane = summonCreature(NPC_FJOLA_LIGHTBANE, ToCSpawnLoc[1], CreatureSummonDespawnType::CORPSE_TIMED_DESPAWN, DESPAWN_TIME);
    Creature* darkbane = summonCreature(NPC_EYDIS_DARKBANE, ToCSpawnLoc[2], CreatureSummonDespawnType::CORPSE_TIMED_DESPAWN, DESPAWN_TIME);
    
    if (lightbane && darkbane)
    {
        lightbane->getAIInterface()->setReactState(REACT_PASSIVE);
        lightbane->summonCreature(NPC_LIGHT_ESSENCE, TwinValkyrsLoc[0]);
        lightbane->summonCreature(NPC_LIGHT_ESSENCE, TwinValkyrsLoc[1]);
        lightbane->GetScript()->setLinkedCreatureAIScript(darkbane->GetScript());

        darkbane->getAIInterface()->setReactState(REACT_PASSIVE);
        darkbane->summonCreature(NPC_DARK_ESSENCE, TwinValkyrsLoc[2]);
        darkbane->summonCreature(NPC_DARK_ESSENCE, TwinValkyrsLoc[3]);
        darkbane->GetScript()->setLinkedCreatureAIScript(lightbane->GetScript());
    }
    getInstanceScript()->DoAction(ACTION_OPEN_GATE);
}

void TirionAI::lickkingChallenge(CreatureAIFunc pThis)
{
    summonCreature(NPC_LICH_KING_VOICE, LocationVector(646.073f, 60.3333f, 394.856f), TIMED_DESPAWN, 50 * TimeVarsMs::Second);

    if (Creature* lkVoice = getInstanceScript()->getCreatureFromData(DATA_LICH_KING_VOICE))
        lkVoice->GetScript()->sendDBChatMessageByIndex(LK_VOICE_SAY_CHALLENGE);

    summonCreature(NPC_ARTHAS_PORTAL, ArthasPortalSpawnPosition, CreatureSummonDespawnType::TIMED_DESPAWN, 34 * TimeVarsMs::Second);
    
    addAIFunction(&TirionAI::summonLickKing, DoOnceScheduler(5s));
}

void TirionAI::lichkingSouls(CreatureAIFunc pThis)
{
    if (Creature* lkVoice = getInstanceScript()->getCreatureFromData(DATA_LICH_KING_VOICE))
        lkVoice->GetScript()->sendDBChatMessageByIndex(LK_VOICE_SAY_SOULS_WILL_BE_MINE);
}

void TirionAI::summonLickKing(CreatureAIFunc pThis)
{
    summonCreature(NPC_LICH_KING, LichKingSpawnPosition, CreatureSummonDespawnType::TIMED_DESPAWN, 38 * TimeVarsMs::Second);
}

void TirionAI::handleBarrettSummon(CreatureAIFunc pThis)
{
    if (isHeroic() && mInstance->getBossState(DATA_NORTHREND_BEASTS) != EncounterStates::Performed)
        summonCreature(NPC_BARRETT_BEASTS_HC, BarretSpawnPosition);
    else if (mInstance->getBossState(DATA_NORTHREND_BEASTS) != EncounterStates::Performed)
        summonCreature(NPC_BARRETT_BEASTS, BarretSpawnPosition);
    else if (mInstance->getBossState(DATA_JARAXXUS) != EncounterStates::Performed)
    {
        summonCreature(NPC_BARRETT_JARAXXUS, BarretSpawnPosition);
        if (mInstance->getBossState(DATA_JARAXXUS) == EncounterStates::Failed)
            DoAction(ACTION_SUMMON_JARAXXUS);
    }
    else if (mInstance->getBossState(DATA_FACTION_CRUSADERS) != EncounterStates::Performed)
        summonCreature(NPC_BARRETT_FACTION, BarretSpawnPosition);
    else if (mInstance->getBossState(DATA_TWIN_VALKIRIES) != EncounterStates::Performed)
        summonCreature(NPC_BARRETT_VALKYR, BarretSpawnPosition);
    else if (mInstance->getBossState(DATA_LICH_KING) != EncounterStates::Performed)
        summonCreature(NPC_BARRETT_LK, BarretSpawnPosition);
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Garrosh
GaroshAI::GaroshAI(Creature* pCreature) : CreatureAIScript(pCreature) { }

CreatureAIScript* GaroshAI::Create(Creature* pCreature) { return new GaroshAI(pCreature); }

void GaroshAI::DoAction(int32_t action)
{
    switch (action)
    {
        case ACTION_START_GORMOK:
            addMessage(Message(GARROSH_SAY_BEASTS), DoOnceScheduler(12s));
            break;
        case ACTION_JARAXXUS_DEFEATED:
            addMessage(Message(GARROSH_SAY_ALLIANCE_DOGS), DoOnceScheduler(14s));
            break;
        case ACTION_START_CHAMPIONS:
            addMessage(Message(GARROSH_SAY_DEMAND_JUSTICE), DoOnceScheduler(8s));
            break;
        case ACTION_SAY_KILLED_PLAYER:
        {
            Message killedMessage;
            killedMessage.addDBMessage(GARROSH_SAY_KILLED_1);
            killedMessage.addDBMessage(GARROSH_SAY_KILLED_2);
            killedMessage.addDBMessage(GARROSH_SAY_KILLED_3);
            addMessage(killedMessage, DoOnceScheduler());
        } break;
        case ACTION_CHAMPIONS_DEFEATED:
            addMessage(Message(GARROSH_SAY_FACTION_DEAD), DoOnceScheduler());
            break;
        case ACTION_VALKYR_DEFEATED:
            addMessage(Message(GARROSH_SAY_VALKYR_DEAD), DoOnceScheduler(5s));
            break;
        case ACTION_START_CHAMPIONS_ENGAGE:
            addMessage(Message(GARROSH_SAY_NO_MERCY), DoOnceScheduler(6s));
            break;
        default:
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Varian
VarianAI::VarianAI(Creature* pCreature) : CreatureAIScript(pCreature) { }

CreatureAIScript* VarianAI::Create(Creature* pCreature) { return new VarianAI(pCreature); }

void VarianAI::DoAction(int32_t action)
{
    switch (action)
    {
        case ACTION_START_GORMOK:
            addMessage(Message(VARIAN_SAY_BEASTS), DoOnceScheduler(12s));
            break;
        case ACTION_JARAXXUS_DEFEATED:
            addMessage(Message(VARIAN_SAY_COME_PIGS), DoOnceScheduler(24s));
            break;
        case ACTION_START_CHAMPIONS:
            addMessage(Message(VARIAN_SAY_DEMAND_JUSTICE), DoOnceScheduler(9s));
            break;
        case ACTION_SAY_KILLED_PLAYER:
        {
            Message killedMessage;
            killedMessage.addDBMessage(VARIAN_SAY_KILLED_1);
            killedMessage.addDBMessage(VARIAN_SAY_KILLED_2);
            killedMessage.addDBMessage(VARIAN_SAY_KILLED_3);
            addMessage(killedMessage, DoOnceScheduler());
        } break;
        case ACTION_CHAMPIONS_DEFEATED:
            addMessage(Message(VARIAN_SAY_FACTION_DEAD), DoOnceScheduler());
            break;
        case ACTION_VALKYR_DEFEATED:
            addMessage(Message(VARIAN_SAY_VALKYR_DEAD), DoOnceScheduler(6s));
            break;
        case ACTION_START_CHAMPIONS_ENGAGE:
            addMessage(Message(VARIAN_SAY_FIGHT_GLORY), DoOnceScheduler(6s));
            break;
        default:
            break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
///  LichKing
LichKingAI::LichKingAI(Creature* pCreature) : CreatureAIScript(pCreature) { }

CreatureAIScript* LichKingAI::Create(Creature* pCreature) { return new LichKingAI(pCreature); }

void LichKingAI::InitOrReset()
{
    setUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    setReactState(REACT_PASSIVE);
    getInstanceScript()->setBossState(DATA_LICH_KING, EncounterStates::InProgress);
    addAIFunction(&LichKingAI::startMove, DoOnceScheduler(1s));
}

void LichKingAI::OnReachWP(uint32_t type, uint32_t id)
{
    if (type != SPLINE_CHAIN_MOTION_TYPE)
        return;

    if (id == POINT_MIDDLE)
        addAIFunction(&LichKingAI::startTalk, DoOnceScheduler(4s));
}

void LichKingAI::startMove(CreatureAIFunc pThis)
{
    moveAlongSplineChain(POINT_MIDDLE, SPLINE_INITIAL_MOVEMENT, true);
}

void LichKingAI::startTalk(CreatureAIFunc pThis)
{
    addMessage(Message(LK_SAY_EMPIRE), DoOnceScheduler());
    addAIFunction(&LichKingAI::changeWeather, DoOnceScheduler(4s));
    addAIFunction(&LichKingAI::breakPlatform, DoOnceScheduler(18s));
}

void LichKingAI::changeWeather(CreatureAIFunc pThis)
{
    addEmote(Emote(EMOTE_STATE_TALK, true), DoOnceScheduler());
    sWeatherMgr.sendWeatherForZone(1, 0.0f, AREA_TRIAL_OF_THE_CRUSADER);
    addEmote(Emote(EMOTE_ONESHOT_EXCLAMATION), DoOnceScheduler(10s));
    addEmote(Emote(EMOTE_ONESHOT_KNEEL), DoOnceScheduler(10s + 3s));
}

void LichKingAI::breakPlatform(CreatureAIFunc pThis)
{
    castSpellOnSelf(SPELL_LK_FROST_NOVA, true);
    castSpellOnSelf(SPELL_CORPSE_TELEPORT, true);
    castSpellOnSelf(SPELL_DESTROY_FLOOR_KNOCKUP, true);

    if (Creature* fordring = getInstanceScript()->getCreatureFromData(DATA_FORDRING))
        fordring->GetScript()->DoAction(ACTION_LK_EVENT_FINISHED);

    if (GameObject_Destructible* floor = static_cast<GameObject_Destructible*>(getInstanceScript()->getGameObjectFromData(DATA_COLISEUM_FLOOR)))
        floor->setDestructibleState(GO_DESTRUCTIBLE_DAMAGED);

    getInstanceScript()->setBossState(DATA_LICH_KING, EncounterStates::Performed);
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Arthas Portal
ArthasPortalAI::ArthasPortalAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    setUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    setReactState(REACT_PASSIVE);
    addAIFunction([this](CreatureAIFunc pThis)
        {
            castSpellOnSelf(SPELL_ARTHAS_PORTAL, true);
            sWeatherMgr.sendWeatherForZone(7, 0.75f, AREA_TRIAL_OF_THE_CRUSADER);
        }, DoOnceScheduler(3s, 100.0f));
}

CreatureAIScript* ArthasPortalAI::Create(Creature* pCreature) { return new ArthasPortalAI(pCreature); }

//////////////////////////////////////////////////////////////////////////////////////////
/// Instance Setup
void SetupTrialOfTheCrusader(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TRIAL_OF_THE_CRUSADER, &TrialOfTheCrusaderInstanceScript::Create);

    GossipScript* BarretGossipScript = new BarretGossip();
    mgr->register_creature_gossip(NPC_BARRETT_BEASTS, BarretGossipScript);
    mgr->register_creature_gossip(NPC_BARRETT_BEASTS_HC, BarretGossipScript);
    mgr->register_creature_gossip(NPC_BARRETT_JARAXXUS, BarretGossipScript);
    mgr->register_creature_gossip(NPC_BARRETT_FACTION, BarretGossipScript);
    mgr->register_creature_gossip(NPC_BARRETT_VALKYR, BarretGossipScript);
    mgr->register_creature_gossip(NPC_BARRETT_LK, BarretGossipScript);
    mgr->register_creature_script(NPC_BARRET, &BarretAI::Create);

    mgr->register_creature_script(NPC_TIRION_FORDRING, &TirionAI::Create);
    mgr->register_creature_script(NPC_GARROSH, &GaroshAI::Create);
    mgr->register_creature_script(NPC_VARIAN, &VarianAI::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Northrend Beasts
    mgr->register_creature_script(NPC_BEASTS_COMBAT_STALKER, &CombatStalkerAI::Create);
    mgr->register_creature_script(NPC_GORMOK, &GormokAI::Create);
    mgr->register_creature_script(NPC_SNOBOLD_VASSAL, &SnoboldAI::Create);
    mgr->register_creature_script(NPC_FIREBOMB, &FireBombAI::Create);

    mgr->register_creature_script(NPC_DREADSCALE, &DreadscaleAI::Create);
    mgr->register_creature_script(NPC_ACIDMAW, &AcidmawAI::Create);
    mgr->register_creature_script(NPC_ICEHOWL, &IcehowlAI::Create);
    mgr->register_creature_script(NPC_SLIME_POOL, &SlimePoolAI::Create);

    mgr->register_spell_script(Beasts::Gormok::SPELL_RIDE_PLAYER, new RidePlayer);
    mgr->register_spell_script(Beasts::Gormok::SPELL_SNOBOLLED, new Snobolled);
    mgr->register_spell_script(Beasts::Gormok::SPELL_FIRE_BOMB, new Firebomb);

    mgr->register_spell_script(Beasts::Dreadscale_Acidmaw::SPELL_PARALYTIC_SPRAY, new ParalyticSpray);
    mgr->register_spell_script(Beasts::Dreadscale_Acidmaw::SPELL_BURNING_SPRAY, new BurningSpray);

    mgr->register_spell_script(Beasts::Dreadscale_Acidmaw::SPELL_PARALYTIC_TOXIN, new ParalyticToxin);
    mgr->register_spell_script(Beasts::Dreadscale_Acidmaw::SPELL_BURNING_BILE2, new BurningBile);
    mgr->register_spell_script(Beasts::Dreadscale_Acidmaw::SPELL_SLIME_POOL_EFFECT, new SlimePool);
    mgr->register_spell_script(Beasts::Dreadscale_Acidmaw::SPELL_PARALYSIS, new Paralysis);

    mgr->register_script_effect(Beasts::Icehowl::SPELL_ARCTIC_BREATH, &ArcticBreathEffect);
    mgr->register_spell_script(Beasts::Icehowl::SPELL_TRAMPLE, new Trample);
    mgr->register_spell_script(Beasts::Icehowl::SPELL_MASSIVE_CRASH, new MassiceCrash);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Jaraxxus
    mgr->register_creature_script(NPC_JARAXXUS, &JaraxxusAI::Create);
    mgr->register_creature_script(NPC_FIZZLEBANG, &FrizzlebangAI::Create);
    mgr->register_creature_script(NPC_WILFRED_PORTAL, &PortalAI::Create);

    mgr->register_creature_script(jaraxxus::NPC_LEGION_FLAME, &LegionFlameAI::Create);
    mgr->register_creature_script(jaraxxus::NPC_INFERNAL_VOLCANO, &InfernalVolcanoAI::Create);
    mgr->register_creature_script(jaraxxus::NPC_FEL_INFERNAL, &FelInfernalAI::Create);
    mgr->register_creature_script(jaraxxus::NPC_NETHER_PORTAL, &NetherPortalAI::Create);
    mgr->register_creature_script(jaraxxus::NPC_MISTRESS_OF_PAIN, &MistressOfPainAI::Create);

    mgr->register_script_effect(jaraxxus::SPELL_FEL_STREAK_VISUAL, &FelStreakEffect);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Fraction
    mgr->register_creature_script(NPC_CHAMPIONS_CONTROLLER, &ChampionControllerAI::Create);

    // Healers
    mgr->register_creature_script(NPC_HORDE_DRUID_RESTORATION, &DruidAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_DRUID_RESTORATION, &DruidAI::Create);
    mgr->register_creature_script(NPC_HORDE_SHAMAN_RESTORATION, &ShamanAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_SHAMAN_RESTORATION, &ShamanAI::Create);
    mgr->register_creature_script(NPC_HORDE_PALADIN_HOLY, &PaladinAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_PALADIN_HOLY, &PaladinAI::Create);
    mgr->register_creature_script(NPC_HORDE_PRIEST_DISCIPLINE, &PriestAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_PRIEST_DISCIPLINE, &PriestAI::Create);
    // Ranged
    mgr->register_creature_script(NPC_HORDE_PRIEST_SHADOW, &ShadowPriestAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_PRIEST_SHADOW, &ShadowPriestAI::Create);
    mgr->register_creature_script(NPC_HORDE_WARLOCK, &WarlockAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_WARLOCK, &WarlockAI::Create);
    mgr->register_creature_script(NPC_HORDE_MAGE, &MageAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_MAGE, &MageAI::Create);
    mgr->register_creature_script(NPC_HORDE_HUNTER, &HunterAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_HUNTER, &HunterAI::Create);
    mgr->register_creature_script(NPC_HORDE_DRUID_BALANCE, &BoomkinAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_DRUID_BALANCE, &BoomkinAI::Create);
    // Melee
    mgr->register_creature_script(NPC_HORDE_WARRIOR, &WarriorAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_WARRIOR, &WarriorAI::Create);
    mgr->register_creature_script(NPC_HORDE_DEATH_KNIGHT, &DKAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_DEATH_KNIGHT, &DKAI::Create);
    mgr->register_creature_script(NPC_HORDE_ROGUE, &RogueAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_ROGUE, &RogueAI::Create);
    mgr->register_creature_script(NPC_HORDE_SHAMAN_ENHANCEMENT, &EnhancerAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_SHAMAN_ENHANCEMENT, &EnhancerAI::Create);
    mgr->register_creature_script(NPC_HORDE_PALADIN_RETRIBUTION, &RetriAI::Create);
    mgr->register_creature_script(NPC_ALLIANCE_PALADIN_RETRIBUTION, &RetriAI::Create);
    // Pets
    mgr->register_creature_script(NPC_PET_WARLOCK, &PetWarlockAI::Create);
    mgr->register_creature_script(NPC_PET_HUNTER, &PetHunterAI::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Twins
    mgr->register_creature_script(NPC_FJOLA_LIGHTBANE, &FjolaAI::Create);
    mgr->register_creature_script(NPC_EYDIS_DARKBANE, &EydisAI::Create);

    GossipScript* EssenceGossipScript = new EssenceGossip();
    mgr->register_creature_gossip(NPC_DARK_ESSENCE, EssenceGossipScript);
    mgr->register_creature_gossip(NPC_LIGHT_ESSENCE, EssenceGossipScript);

    mgr->register_creature_script(NPC_UNLEASHED_DARK, &UnleashedDarkAI::Create);
    mgr->register_creature_script(NPC_UNLEASHED_LIGHT, &UnleashedLightAI::Create);
    mgr->register_creature_script(NPC_BULLET_CONTROLLER, &BulletCotrollerAI::Create);

    mgr->register_script_effect(twins::SPELL_POWERING_UP, &PoweringUpEffect);
    mgr->register_spell_script(twins::SPELL_DARK_ESSENCE, new EssenceScript);
    mgr->register_spell_script(twins::SPELL_LIGHT_ESSENCE, new EssenceScript);
    mgr->register_spell_script(twins::SPELL_POWER_TWINS, new PowerOfTwinsScript);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Anubarak
    mgr->register_creature_script(NPC_ANUBARAK, &AnubarakAI::Create);
    mgr->register_creature_script(anubarak::NPC_SCARAB, &SwarmScrabAI::Create);
    mgr->register_creature_script(anubarak::NPC_BURROWER, &BurrowerAI::Create);
    mgr->register_creature_script(anubarak::NPC_FROST_SPHERE, &FrostSphereAI::Create);
    mgr->register_creature_script(anubarak::NPC_SPIKE, &SpikeAI::Create);

    mgr->register_creature_script(NPC_LICH_KING, &LichKingAI::Create);
    mgr->register_creature_script(NPC_ARTHAS_PORTAL, &ArthasPortalAI::Create);

    mgr->register_dummy_spell(anubarak::SPELL_PERMAFROST_DUMMY, &PermafrostDummySpell);
}
