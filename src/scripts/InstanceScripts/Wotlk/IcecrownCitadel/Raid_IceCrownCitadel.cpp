/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Raid_IceCrownCitadel.hpp"

#include "GunshipBattle.hpp"
#include "LadyDeathwhisper.hpp"
#include "LordMarrowgar.hpp"
#include "Saurfang.hpp"
#include "Management/Gossip/GossipMenu.hpp"
#include "Movement/MovementManager.h"
#include "Objects/GameObject.h"
#include "Objects/Transporter.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"

//////////////////////////////////////////////////////////////////////////////////////////
//ICC zone: 4812
//Prepared creature entry:
//
//CN_FESTERGUT                36626
//CN_ROTFACE                  36627
//CN_PROFESSOR_PUTRICIDE      36678
//CN_PRINCE_VALANAR           37970
//N_BLOOD_QUEEN_LANATHEL      37955
//CN_SINDRAGOSA               36853
//CN_THE_LICHKING             36597
//

//////////////////////////////////////////////////////////////////////////////////////////
//IceCrownCitadel Instance
IceCrownCitadelScript::IceCrownCitadelScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
{
    Instance = (IceCrownCitadelScript*)pMapMgr->getScript();
    // Overwrite this
    setBossNumber(EncounterCount);

    // Entrance
    introDone = false;
    HighlordEntranceGUID = 0;
    LichKingEntranceGUID = 0;
    BolvarEntranceGUID = 0;

    nerubarBroodkeepersGUIDs[0] = { 0, 0 };
    nerubarBroodkeepersGUIDs[1] = { 0, 0 };

    // Lord Marrowgar
    LordMarrowgarGUID = 0;
    MarrowgarIcewall1GUID = 0;
    MarrowgarIcewall2GUID = 0;
    MarrowgarEntranceDoorGUID = 0;
    bonedAchievement = false;

    // Lady Deathwhisper
    LadyDeathwisperGUID = 0;
    LadyDeathwisperElevatorGUID = 0;
    LadyDeathwisperEntranceDoorGUID = 0;

    // Gunship Battle
    skybreaker = nullptr;
    orgrimmar = nullptr;
    SkybreakerBossGUID = 0;
    OrgrimmarBossGUID = 0;
    DeathbringerSaurfangGbGUID = 0;
    MuradinBronzebeardGbGUID = 0;
    GbBattleMageGUID = 0;
    isPrepared = false;

    // Deathbringer Saurfang
    DeathbringerDoorGUID = 0;
    DeathbringerSaurfangGUID = 0;
    HordeZeppelinAlliance = nullptr;
    deathbringerGoSpawned = false;
}

InstanceScript* IceCrownCitadelScript::Create(WorldMap* pMapMgr) { return new IceCrownCitadelScript(pMapMgr); }

void IceCrownCitadelScript::setLocalData(uint32_t type, uint32_t data)
{
    switch (type)
    {
        case DATA_BONED_ACHIEVEMENT:
        {
            bonedAchievement = data ? true : false;
        } break;
        case DATA_NERUBAR_BROODKEEPER_EVENT:
        {
            uint8_t group = (data == ICC_BROODKEEPER1) ? 0 : 1;

            for (uint32_t guid : nerubarBroodkeepersGUIDs[group])
            {
                if (Creature* nerubar = GetCreatureByGuid(guid))
                    nerubar->GetScript()->DoAction(ACTION_NERUBAR_FALL);
            }
        } break;
    }
}

uint32_t IceCrownCitadelScript::getLocalData(uint32_t type) const
{
    switch (type)
    {
            // Deathbringer Suarfang
        case DATA_SAURFANG_DOOR:
        {
            return DeathbringerDoorGUID;
        }
        case DATA_BONED_ACHIEVEMENT:
        {
            return bonedAchievement;
        }
        default:
            break;
    }
    return 0;
}

Creature* IceCrownCitadelScript::getLocalCreatureData(uint32_t type) const
{
    IceCrownCitadelScript* script = const_cast<IceCrownCitadelScript*>(this);

    switch (type)
    {
            // Intro
        case NPC_INTRO_TIRION:
        {
            return script->GetCreatureByGuid(HighlordEntranceGUID);
        }
        case NPC_INTRO_LICH_KING:
        {
            return script->GetCreatureByGuid(LichKingEntranceGUID);
        }
        case NPC_INTRO_BOLVAR:
        {
            return script->GetCreatureByGuid(BolvarEntranceGUID);
        }
            // Marrowgar
        case CN_LORD_MARROWGAR:
        {
            return script->GetCreatureByGuid(LordMarrowgarGUID);
        }
            // Lady Deathwhisper
        case CN_LADY_DEATHWHISPER:
        {
            return script->GetCreatureByGuid(LadyDeathwisperGUID);
        }
            // Gunshipbattle
        case DATA_SKYBREAKER_BOSS:
        {
            return script->GetCreatureByGuid(SkybreakerBossGUID);
        }
        case DATA_ORGRIMMAR_HAMMER_BOSS:
        {
            return script->GetCreatureByGuid(OrgrimmarBossGUID);
        }
        case DATA_GB_HIGH_OVERLORD_SAURFANG:
        {
            return script->GetCreatureByGuid(DeathbringerSaurfangGbGUID);
        }
        case DATA_GB_MURADIN_BRONZEBEARD:
        {
            return script->GetCreatureByGuid(MuradinBronzebeardGbGUID);
        }
        case DATA_GB_BATTLE_MAGE:
        {
            return script->GetCreatureByGuid(GbBattleMageGUID);
        }
            // Deathbringer Saurfang
        case CN_DEATHBRINGER_SAURFANG:
        {
            return script->GetCreatureByGuid(DeathbringerSaurfangGUID);
        }
        default:
            break;
    }
    return nullptr;
}

void IceCrownCitadelScript::OnCreaturePushToWorld(Creature* pCreature)
{
    WoWGuid guid = pCreature->getGuid();

    switch (pCreature->getEntry())
    {
            // Intro
        case NPC_INTRO_TIRION:
        {
            HighlordEntranceGUID = guid.getGuidLowPart();
            break;
        }
        case NPC_INTRO_LICH_KING:
        {
            LichKingEntranceGUID = guid.getGuidLowPart();
            break;
        }
        case NPC_INTRO_BOLVAR:
        {
            BolvarEntranceGUID = guid.getGuidLowPart();
            break;
        }
            // Broodkeepers
        case NPC_NERUBAR_BROODKEEPER:
        {
            uint8_t group = (pCreature->GetPositionX() > -230.0f) ? 0 : 1;
            nerubarBroodkeepersGUIDs[group].emplace_back(guid.getGuidLowPart());
            break;
        }
            // Lord Marrowgar
        case CN_LORD_MARROWGAR:
        {
            LordMarrowgarGUID = guid.getGuidLowPart();
            break;
        }
            // Lady Deathwisper
        case CN_LADY_DEATHWHISPER:
        {
            LadyDeathwisperGUID = guid.getGuidLowPart();
            break;
        }
            // Gunship
        case NPC_GB_SKYBREAKER:
        {
            SkybreakerBossGUID = guid.getGuidLowPart();
            break;
        }
        case NPC_GB_ORGRIMS_HAMMER:
        {
            OrgrimmarBossGUID = guid.getGuidLowPart();
            break;
        }
        case NPC_GB_HIGH_OVERLORD_SAURFANG:
        {
            DeathbringerSaurfangGbGUID = guid.getGuidLowPart();
            break;
        }
        case NPC_GB_MURADIN_BRONZEBEARD:
        {
            MuradinBronzebeardGbGUID = guid.getGuidLowPart();
            break;
        }
        case NPC_GB_SKYBREAKER_SORCERERS:
        case NPC_GB_KORKRON_BATTLE_MAGE:
        {
            GbBattleMageGUID = guid.getGuidLowPart();
            break;
        }
            // Deathbringer Suarfang
        case CN_DEATHBRINGER_SAURFANG:
        {
            DeathbringerSaurfangGUID = guid.getGuidLowPart();
            break;
        }
        default:
            break;
    }
}

void IceCrownCitadelScript::OnGameObjectPushToWorld(GameObject* pGameObject)
{
    switch (pGameObject->getEntry())
    {
        case GO_MARROWGAR_ICEWALL_1:
        {
            MarrowgarIcewall1GUID = pGameObject->getGuidLow();
            break;
        }
        case GO_MARROWGAR_ICEWALL_2:
        {
            MarrowgarIcewall2GUID = pGameObject->getGuidLow();
            break;
        }
        case GO_MARROWGAR_DOOR:
        {
            MarrowgarEntranceDoorGUID = pGameObject->getGuidLow();
            break;
        }
        case GO_ORATORY_OF_THE_DAMNED_ENTRANCE:
        {
            LadyDeathwisperEntranceDoorGUID = pGameObject->getGuidLow();
            break;
        }
        case GO_LADY_DEATHWHISPER_ELEVATOR:
        {
            LadyDeathwisperElevatorGUID = pGameObject->getGuidLow();
            break;
        }
        case GO_TELE_1:
        case GO_TELE_2:
        case GO_TELE_3:
        case GO_TELE_4:
        case GO_TELE_5:
        {
            pGameObject->setFlags(GO_FLAG_NONE);
            break;
        }
        case GO_SAURFANG_S_DOOR:
        {
            DeathbringerDoorGUID = pGameObject->getGuidLow();
            break;
        }
        default:
            break;
    }

    // State also must changes for Objects out of range so change state on spawning them :)
    SetGameobjectStates(pGameObject);
}

void IceCrownCitadelScript::SetGameobjectStates(GameObject* /*pGameObject*/)
{
    // Gos which are not visible by killing a boss needs a second check...
    if (getBossState(DATA_LORD_MARROWGAR) == Performed)
    {
        if (MarrowgarIcewall1GUID)
            if (GetGameObjectByGuid(MarrowgarIcewall1GUID))
                GetGameObjectByGuid(MarrowgarIcewall1GUID)->setState(GO_STATE_OPEN);        // Icewall 1

        if (MarrowgarIcewall2GUID)
            if (GetGameObjectByGuid(MarrowgarIcewall2GUID))
                GetGameObjectByGuid(MarrowgarIcewall2GUID)->setState(GO_STATE_OPEN);        // Icewall 2

        if (MarrowgarEntranceDoorGUID)
            if (GetGameObjectByGuid(MarrowgarEntranceDoorGUID))
                GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_OPEN);    // Door  
    }

    if (getBossState(DATA_LADY_DEATHWHISPER) == Performed)
    {
        if (LadyDeathwisperEntranceDoorGUID)
            if (GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID))
                GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_OPEN);

        if (LadyDeathwisperElevatorGUID)
            if (GetGameObjectByGuid(LadyDeathwisperElevatorGUID))
            GetGameObjectByGuid(LadyDeathwisperElevatorGUID)->setState(GO_STATE_OPEN);
    }

    if (getBossState(DATA_LADY_DEATHWHISPER) == NotStarted)
    {
        if (LadyDeathwisperEntranceDoorGUID)
            if (GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID))
            GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_OPEN);
    }

    if (getBossState(DATA_DEATHBRINGER_SAURFANG) == NotStarted)
    {
        if (DeathbringerDoorGUID)
            if (GetGameObjectByGuid(DeathbringerDoorGUID))
            GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_CLOSED);
    }

    if (getBossState(DATA_DEATHBRINGER_SAURFANG) == Performed)
    {
        if (DeathbringerDoorGUID)
            if (GetGameObjectByGuid(DeathbringerDoorGUID))
            GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_OPEN);

        if (!getLocalCreatureData(CN_DEATHBRINGER_SAURFANG))
        {
            DoAction(ACTION_SPAWN_GOS);
        }
    }
}

void IceCrownCitadelScript::OnEncounterStateChange(uint32_t entry, uint32_t state)
{
    switch (entry)
    {
        case DATA_LORD_MARROWGAR:
        {
            if (state == InProgress)
            {
                if (MarrowgarEntranceDoorGUID)
                    if (GetGameObjectByGuid(MarrowgarEntranceDoorGUID))
                        GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_CLOSED);
            }
            if (state == NotStarted)
            {
                if (MarrowgarEntranceDoorGUID)
                    if (GetGameObjectByGuid(MarrowgarEntranceDoorGUID))
                        GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_OPEN);
            }
            if (state == Performed)
            {
                if (MarrowgarIcewall1GUID)
                    if (GetGameObjectByGuid(MarrowgarIcewall1GUID))
                        GetGameObjectByGuid(MarrowgarIcewall1GUID)->setState(GO_STATE_OPEN);        // Icewall 1

                if (MarrowgarIcewall2GUID)
                    if (GetGameObjectByGuid(MarrowgarIcewall2GUID))
                        GetGameObjectByGuid(MarrowgarIcewall2GUID)->setState(GO_STATE_OPEN);        // Icewall 2

                if (MarrowgarEntranceDoorGUID)
                    if (GetGameObjectByGuid(MarrowgarEntranceDoorGUID))
                        GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_OPEN);    // Door  
            }
            break;
        }
        case DATA_LADY_DEATHWHISPER:
        {
            if (state == InProgress)
            {
                if (LadyDeathwisperEntranceDoorGUID)
                    if (GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID))
                        GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_CLOSED);
            }
            if (state == NotStarted)
            {
                if (LadyDeathwisperEntranceDoorGUID)
                    if (GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID))
                        GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_CLOSED);
            }
            if (state == Performed)
            {
                if (LadyDeathwisperEntranceDoorGUID)
                    if (GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID))
                        GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_OPEN);

                if (LadyDeathwisperElevatorGUID)
                    if (GetGameObjectByGuid(LadyDeathwisperElevatorGUID))
                        GetGameObjectByGuid(LadyDeathwisperElevatorGUID)->setState(GO_STATE_OPEN);
            }
            break;
        }
        case DATA_DEATHBRINGER_SAURFANG:
        {
            if (state == InProgress)
            {
                if (DeathbringerDoorGUID)
                    if (GetGameObjectByGuid(DeathbringerDoorGUID))
                        if (GetGameObjectByGuid(DeathbringerDoorGUID))
                    GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_CLOSED);
            }

            if (state == NotStarted)
            {
                if (DeathbringerDoorGUID)
                    if (GetGameObjectByGuid(DeathbringerDoorGUID))
                    GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_CLOSED);
            }

            if (state == Performed)
            {
                if (DeathbringerDoorGUID)
                    if (GetGameObjectByGuid(DeathbringerDoorGUID))
                    GetGameObjectByGuid(DeathbringerDoorGUID)->setState(GO_STATE_OPEN);

                if (!getLocalCreatureData(CN_DEATHBRINGER_SAURFANG))
                {
                    DoAction(ACTION_SPAWN_GOS);
                }
            }
            break;
        }
        default:
            break;
    }      
}

void IceCrownCitadelScript::OnAreaTrigger(Player* /*pPlayer*/, uint32_t pAreaId)
{
    switch(pAreaId)
    {
        case ICC_ENTRANCE:
        {
            if (!introDone)
                DoAction(ACTION_STARTINTRO);
            break;
        }
        case ICC_LORD_MARROWGAR_ENTRANCE:
        {
            if (getLocalCreatureData(CN_LORD_MARROWGAR))
                getLocalCreatureData(CN_LORD_MARROWGAR)->GetScript()->DoAction(ACTION_MARROWGAR_INTRO_START);
            break;
        }
        case ICC_LADY_DEATHWHISPER_ENTRANCE:
        {
            if (getLocalCreatureData(CN_LADY_DEATHWHISPER))
                getLocalCreatureData(CN_LADY_DEATHWHISPER)->GetScript()->DoAction(ACTION_LADY_INTRO_START);
            break;
        }
        case ICC_BROODKEEPER1:
        case ICC_BROODKEEPER2:
        {
            setLocalData(DATA_NERUBAR_BROODKEEPER_EVENT, pAreaId);
        } break;
        case ICC_DRAGON_ALLIANCE:
        {
            break;
        }
        case ICC_DRAGON_HORDE:
        {
            break;
        }
        default:
            break;
    }
}

void IceCrownCitadelScript::SpawnEnemyGunship()
{
    if (getInstance()->getTeamIdInInstance() == TEAM_ALLIANCE)
        orgrimmar = sTransportHandler.createTransport(GO_ORGRIM_S_HAMMER_ALLIANCE_ICC, mInstance);

    if (getInstance()->getTeamIdInInstance() == TEAM_HORDE)
       skybreaker = sTransportHandler.createTransport(GO_THE_SKYBREAKER_HORDE_ICC, mInstance);
}

void IceCrownCitadelScript::OnPlayerEnter(Player* /*player*/)
{
    if (!spawnsCreated())
    {
        // setup only the npcs with the correct team...
        switch (getInstance()->getTeamIdInInstance())
        {
            case TEAM_ALLIANCE:
            {
                for (uint8_t i = 0; i < 18; i++)
                    spawnCreature(AllySpawns[i].entry, AllySpawns[i].x, AllySpawns[i].y, AllySpawns[i].z, AllySpawns[i].o, AllySpawns[i].faction);
                break;
            }
            case TEAM_HORDE:
            {
                for (uint8_t i = 0; i < 18; i++)
                    spawnCreature(HordeSpawns[i].entry, HordeSpawns[i].x, HordeSpawns[i].y, HordeSpawns[i].z, HordeSpawns[i].o, HordeSpawns[i].faction);
                break;
            }
            default:
                break;
        }
        setSpawnsCreated();

        Creature* Tirion = getLocalCreatureData(NPC_INTRO_TIRION);
        if (Tirion)
        {
            Creature* Commander = findNearestCreature(Tirion, getInstance()->getTeamIdInInstance() ? NPC_SE_HIGH_OVERLORD_SAURFANG : NPC_SE_MURADIN_BRONZEBEARD, 30.0f);
            if (Commander)
                Commander->setNpcFlags(UNIT_NPC_FLAG_NONE);
        }
    }

    // Spawning the Gunships at the same moment a player enters causes them to bug the npcs sometimes
    if (!isPrepared)
        scriptEvents.addEvent(EVENT_SPAWN_GUNSHIPS, 5000);
}    

void IceCrownCitadelScript::UpdateEvent()
{
    scriptEvents.updateEvents(getUpdateFrequency(), 0);

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case EVENT_SPAWN_GUNSHIPS:
            {
                if (getBossState(DATA_ICECROWN_GUNSHIP_BATTLE) == Performed)
                    return;

                if (!isPrepared)
                {
                    if (getInstance()->getTeamIdInInstance() == TEAM_ALLIANCE)
                        skybreaker = sTransportHandler.createTransport(GO_THE_SKYBREAKER_ALLIANCE_ICC, mInstance);

                    if (getInstance()->getTeamIdInInstance() == TEAM_HORDE)
                        orgrimmar = sTransportHandler.createTransport(GO_ORGRIM_S_HAMMER_HORDE_ICC, mInstance);

                    isPrepared = true;
                }
                break;
            }   
            case EVENT_SPAWN_ZEPPELIN_ALLIANCE:
            {
                HordeZeppelinAlliance = sTransportHandler.createTransport(GO_MIGHTY_WIND, mInstance);
                HordeZeppelinAlliance->EnableMovement(true, mInstance);
                break;
            }   
            case EVENT_WIPE_CHECK:
            {
                if (getInstance()->getTeamIdInInstance() == TEAM_ALLIANCE)
                {
                    DoCheckFallingPlayer(GetCreatureByGuid(MuradinBronzebeardGbGUID));
                    if (DoWipeCheck(skybreaker))
                        scriptEvents.addEvent(EVENT_WIPE_CHECK, 3000);
                    else
                        DoAction(ACTION_FAIL);
                }
                else
                {
                    DoCheckFallingPlayer(GetCreatureByGuid(DeathbringerSaurfangGbGUID));
                    if (DoWipeCheck(orgrimmar))
                        scriptEvents.addEvent(EVENT_WIPE_CHECK, 3000);
                    else
                        DoAction(ACTION_FAIL);
                }
                break;
            }
            case EVENT_START_FLY:
            {
                if (getInstance()->getTeamIdInInstance() == TEAM_ALLIANCE && skybreaker)
                    skybreaker->EnableMovement(true, mInstance);

                if (getInstance()->getTeamIdInInstance() == TEAM_HORDE && orgrimmar)
                    orgrimmar->EnableMovement(true, mInstance);
                break;
            }
            default:
                break;
        }
    }
}

void IceCrownCitadelScript::DoAction(int32_t const action)
{
    switch (action)
    {
        case ACTION_STARTINTRO:
        {
            introDone = true;
            Creature* Tirion = getLocalCreatureData(NPC_INTRO_TIRION);
            Creature* LichKing = getLocalCreatureData(NPC_INTRO_LICH_KING);
            Creature* Bolvar = getLocalCreatureData(NPC_INTRO_BOLVAR);

            if (!Tirion)
                return;

            Creature* Commander = findNearestCreature(Tirion, getInstance()->getTeamIdInInstance() ? NPC_SE_HIGH_OVERLORD_SAURFANG : NPC_SE_MURADIN_BRONZEBEARD, 30.0f);

            if (Tirion && LichKing && Bolvar && Commander)
            {
                Tirion->SendTimedScriptTextChatMessage(EVENT_INTRO01, 1000);
                Tirion->SendTimedScriptTextChatMessage(EVENT_INTRO02, 15000);
                Tirion->SendTimedScriptTextChatMessage(EVENT_INTRO03, 31000);

                LichKing->SendTimedScriptTextChatMessage(EVENT_INTRO04, 35000);
                Tirion->SendTimedScriptTextChatMessage(EVENT_INTRO05, 50000);
                LichKing->SendTimedScriptTextChatMessage(EVENT_INTRO06, 59000);
                LichKing->SendTimedScriptTextChatMessage(EVENT_INTRO07, 74000);
                LichKing->SendTimedScriptTextChatMessage(EVENT_INTRO08, 84000);

                Bolvar->SendTimedScriptTextChatMessage(EVENT_INTRO09, 105000);
                LichKing->SendTimedScriptTextChatMessage(EVENT_INTRO10, 111000);
           
                if (getInstance()->getTeamIdInInstance() == TEAM_ALLIANCE)
                {
                    Commander->SendTimedScriptTextChatMessage(EVENT_INTRO20, 117000);
                    Tirion->SendTimedScriptTextChatMessage(EVENT_INTRO21, 129000);
                    Commander->SendTimedScriptTextChatMessage(EVENT_INTRO22, 138000);
                    Commander->SendTimedScriptTextChatMessage(EVENT_INTRO23, 143000);
                }
                else
                {
                    Commander->SendTimedScriptTextChatMessage(EVENT_INTRO30, 117000);
                    Tirion->SendTimedScriptTextChatMessage(EVENT_INTRO31, 127000);
                    Commander->SendTimedScriptTextChatMessage(EVENT_INTRO32, 136000);
                    Commander->SendTimedScriptTextChatMessage(EVENT_INTRO33, 147000);
                    Commander->SendTimedScriptTextChatMessage(EVENT_INTRO34, 157000);
                }
            }
            break;
        }   
        case ACTION_INTRO_START:
        {
            scriptEvents.addEvent(EVENT_START_FLY, 2500);
            break;
        }
        case ACTION_BATTLE_EVENT:
        {
            scriptEvents.addEvent(EVENT_WIPE_CHECK, 5000);
            setBossState(DATA_ICECROWN_GUNSHIP_BATTLE, InProgress);
            break;
        }
        case ACTION_BATTLE_DONE:
        {
            if (getBossState(DATA_ICECROWN_GUNSHIP_BATTLE) == Performed)
            {
                if (getInstance()->getTeamIdInInstance() == TEAM_ALLIANCE && skybreaker)
                {
                    skybreaker->EnableMovement(true, mInstance);

                    if (orgrimmar)
                        orgrimmar->EnableMovement(true, mInstance);
                }

                if (getInstance()->getTeamIdInInstance() == TEAM_HORDE && orgrimmar)
                {
                    orgrimmar->EnableMovement(true, mInstance);

                    if (skybreaker)
                        skybreaker->EnableMovement(true, mInstance);
                }
            }
            break;
        }
        case ACTION_FAIL:
        {
            // Handle Wipe Here
            break;
        }
        case ACTION_SPAWN_TRANSPORT:
        {
            if (getInstance()->getTeamIdInInstance() == TEAM_ALLIANCE)
            {
                scriptEvents.addEvent(EVENT_SPAWN_ZEPPELIN_ALLIANCE, 1);
            }
            break;
        }
        case ACTION_TRANSPORT_FLY:
        {
            if (HordeZeppelinAlliance)
                HordeZeppelinAlliance->EnableMovement(true, mInstance);

            break;
        }
        case ACTION_SPAWN_GOS:
        {
            if (deathbringerGoSpawned)
                return;

            deathbringerGoSpawned = true;
            GameObject* teleporter1 = nullptr;
            GameObject* teleporter2 = nullptr;

            if (getInstance()->getTeamIdInInstance() == TEAM_HORDE)
            {
                teleporter1 = spawnGameObject(GO_HORDE_TELEPORTER, deathbringerHordeGOs[0].x, deathbringerHordeGOs[0].y, deathbringerHordeGOs[0].z, deathbringerHordeGOs[0].o);
                teleporter2 = spawnGameObject(GO_HORDE_TELEPORTER, deathbringerHordeGOs[1].x, deathbringerHordeGOs[1].y, deathbringerHordeGOs[1].z, deathbringerHordeGOs[1].o);
                spawnGameObject(GO_HORDE_TENT1, deathbringerHordeGOs[2].x, deathbringerHordeGOs[2].y, deathbringerHordeGOs[2].z, deathbringerHordeGOs[2].o);
                spawnGameObject(GO_HORDE_TENT2, deathbringerHordeGOs[3].x, deathbringerHordeGOs[3].y, deathbringerHordeGOs[3].z, deathbringerHordeGOs[3].o);

                spawnCreature(NPC_CANDI, -530.0f, 2229.0f, 539.29f, 2.33f);
                spawnCreature(NPC_MORGAN, -526.0f, 2233.0f, 539.29f, 2.33f);
            }
            else
            {
                teleporter1 = spawnGameObject(GO_ALLIANCE_TELEPORTER, deathbringerAllianceGOs[0].x, deathbringerAllianceGOs[0].y, deathbringerAllianceGOs[0].z, deathbringerAllianceGOs[0].o);
                teleporter2 = spawnGameObject(GO_ALLIANCE_TELEPORTER, deathbringerAllianceGOs[1].x, deathbringerAllianceGOs[1].y, deathbringerAllianceGOs[1].z, deathbringerAllianceGOs[1].o);
                spawnGameObject(GO_ALLIANCE_TENT, deathbringerAllianceGOs[2].x, deathbringerAllianceGOs[2].y, deathbringerAllianceGOs[2].z, deathbringerAllianceGOs[2].o);
                spawnGameObject(GO_ALLIANCE_TENT, deathbringerAllianceGOs[3].x, deathbringerAllianceGOs[3].y, deathbringerAllianceGOs[3].z, deathbringerAllianceGOs[3].o);
                spawnGameObject(GO_ALLIANCE_BANNER, deathbringerAllianceGOs[4].x, deathbringerAllianceGOs[4].y, deathbringerAllianceGOs[4].z, deathbringerAllianceGOs[4].o);

                spawnCreature(NPC_BRAZIE, -530.0f, 2229.0f, 539.29f, 2.33f);
                spawnCreature(NPC_SHELY, -526.0f, 2233.0f, 539.29f, 2.33f);
            }

            if (teleporter1 && teleporter2)
            {
                teleporter1->setState(GO_STATE_OPEN);
                teleporter2->setState(GO_STATE_OPEN);
            }
            break;
        }
        default:
            break;
    }
}

void IceCrownCitadelScript::TransporterEvents(Transporter* transport, uint32_t eventId)
{
    switch (eventId)
    {
        case EVENT_ENEMY_GUNSHIP_DESPAWN:
        {
            if (getBossState(DATA_ICECROWN_GUNSHIP_BATTLE) == Performed)
            {
                transport->removeFromMap();
            }
            break;
        }
        case EVENT_ENEMY_GUNSHIP_COMBAT:
        {
            if (Creature* captain = getInstance()->getTeamIdInInstance() == TEAM_HORDE ? getLocalCreatureData(DATA_GB_HIGH_OVERLORD_SAURFANG) : getLocalCreatureData(DATA_GB_MURADIN_BRONZEBEARD))
                captain->GetScript()->DoAction(ACTION_BATTLE_EVENT);
            // Instance
            transport->getWorldMap()->getScript()->DoAction(ACTION_BATTLE_EVENT);
        }
            [[fallthrough]];
        case EVENT_PLAYERS_GUNSHIP_SPAWN:
        case EVENT_PLAYERS_GUNSHIP_COMBAT:
        {
            transport->EnableMovement(false, mInstance);
            break;
        }
        case EVENT_PLAYERS_GUNSHIP_SAURFANG:
        {
            transport->EnableMovement(false, mInstance);
            break;
        }
        case EVENT_SAURFANG_MIGHTYWIND:
        {
            transport->EnableMovement(false, mInstance);
            break;
        }
        default:
            break;
    }
}

//Wipe check
bool IceCrownCitadelScript::DoWipeCheck(Transporter* /*t*/)
{
    // todo
    return true;
}

//Check falling players
void IceCrownCitadelScript::DoCheckFallingPlayer(Creature* pCreature)
{
    if (pCreature)
    {
        for (const auto& itr : mInstance->getPlayers())
        {
            if (Player* pPlayer = itr.second)
            {
                if (pPlayer->GetPositionZ() < 420.0f && pPlayer->IsWithinDistInMap(pCreature, 300.0f))
                    pPlayer->teleport(pCreature->GetPosition(), mInstance);
            }
        }
    }
}

void IceCrownCitadelScript::TransportBoarded(Unit* pUnit, Transporter* transport)
{
    if (transport->getEntry() == GO_THE_SKYBREAKER_ALLIANCE_ICC)
        pUnit->castSpell(pUnit, SPELL_ON_SKYBREAKER_DECK);

    if (transport->getEntry() == GO_THE_SKYBREAKER_HORDE_ICC)
        pUnit->castSpell(pUnit, SPELL_ON_SKYBREAKER_DECK);

    if (transport->getEntry() == GO_ORGRIM_S_HAMMER_HORDE_ICC)
        pUnit->castSpell(pUnit, SPELL_ON_ORGRIMS_HAMMER_DECK);

    if (transport->getEntry() == GO_ORGRIM_S_HAMMER_ALLIANCE_ICC)
        pUnit->castSpell(pUnit, SPELL_ON_ORGRIMS_HAMMER_DECK);
}

void IceCrownCitadelScript::TransportUnboarded(Unit* pUnit, Transporter* transport)
{
    if (transport->getEntry() == GO_THE_SKYBREAKER_ALLIANCE_ICC)
        pUnit->removeAllAurasById(SPELL_ON_SKYBREAKER_DECK);

    if (transport->getEntry() == GO_THE_SKYBREAKER_HORDE_ICC)
        pUnit->removeAllAurasById(SPELL_ON_SKYBREAKER_DECK);

    if (transport->getEntry() == GO_ORGRIM_S_HAMMER_HORDE_ICC)
        pUnit->removeAllAurasById(SPELL_ON_ORGRIMS_HAMMER_DECK);

    if (transport->getEntry() == GO_ORGRIM_S_HAMMER_ALLIANCE_ICC)
        pUnit->removeAllAurasById(SPELL_ON_ORGRIMS_HAMMER_DECK);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// IceCrown Teleporter
void ICCTeleporterGossip::onHello(Object* object, Player* player)
{
    InstanceScript* pInstance = player->getWorldMap()->getScript();
    if (!pInstance)
        return;

    GossipMenu menu(object->getGuid(), 15221, player->getSession()->language);
    menu.addItem(GOSSIP_ICON_CHAT, 515, 0);     // Teleport to Light's Hammer.

    if (pInstance->getBossState(DATA_LORD_MARROWGAR) == Performed)
        menu.addItem(GOSSIP_ICON_CHAT, 516, 1);      // Teleport to Oratory of The Damned.

    if (pInstance->getBossState(DATA_LADY_DEATHWHISPER) == Performed)
        menu.addItem(GOSSIP_ICON_CHAT, 517, 2);      // Teleport to Rampart of Skulls.

    // GunshipBattle has to be Performed...
    if (pInstance->getBossState(DATA_ICECROWN_GUNSHIP_BATTLE) == Performed || pInstance->getBossState(DATA_DEATHBRINGER_SAURFANG) == Performed)
    menu.addItem(GOSSIP_ICON_CHAT, (518), 3);        // Teleport to Deathbringer's Rise.

    if (pInstance->getBossState(DATA_VALITHRIA_DREAMWALKER) == Performed)
        menu.addItem(GOSSIP_ICON_CHAT, 519, 4);      // Teleport to the Upper Spire.

    if (pInstance->getBossState(DATA_SINDRAGOSA) == Performed)
        menu.addItem(GOSSIP_ICON_CHAT, 520, 5);      // Teleport to Sindragosa's Lair.

    menu.sendGossipPacket(player);
}

void ICCTeleporterGossip::onSelectOption(Object* /*object*/, Player* player, uint32_t Id, const char* /*enteredcode*/, uint32_t /*gossipId*/)
{
    switch (Id)
    {
        case 0:
            player->castSpell(player, 70781, true);     // Light's Hammer
            break;
        case 1:
            player->castSpell(player, 70856, true);     // Oratory of The Damned
            break;
        case 2:
            player->castSpell(player, 70857, true);     // Rampart of Skulls
            break;
        case 3:
            player->castSpell(player, 70858, true);     // Deathbringer's Rise
            break;
        case 4:
            player->castSpell(player, 70859, true);     // Upper Spire
            break;
        case 5:
            player->castSpell(player, 70861, true);     // Sindragosa's Lair
            break;
    }
    GossipMenu::senGossipComplete(player);
}

ICCTeleporterAI::ICCTeleporterAI(GameObject* go) : GameObjectAIScript(go) {}
GameObjectAIScript* ICCTeleporterAI::Create(GameObject* go) { return new ICCTeleporterAI(go); }

ICCTeleporterAI::~ICCTeleporterAI() {}

void ICCTeleporterAI::OnActivate(Player* player)
{
    ICCTeleporterGossip gossip;
    gossip.onHello(_gameobject, player);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Servant Of The Throne
ServantOfTheThroneAI::ServantOfTheThroneAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    addAISpell(SPELL_GLACIALBLAST, 50.0f, TARGET_ATTACKING, 0, 8);
}

CreatureAIScript* ServantOfTheThroneAI::Create(Creature* pCreature) { return new ServantOfTheThroneAI(pCreature); }

//////////////////////////////////////////////////////////////////////////////////////////
/// Ancient Skeletal Soldier
AncientSkeletalSoldierAI::AncientSkeletalSoldierAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    addAISpell(SPELL_SHIELDBASH, 75.0f, TARGET_ATTACKING, 0, 8);
}

CreatureAIScript* AncientSkeletalSoldierAI::Create(Creature* pCreature) { return new AncientSkeletalSoldierAI(pCreature); }

//////////////////////////////////////////////////////////////////////////////////////////
/// Death Bound Ward
DeathBoundWardAI::DeathBoundWardAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    addAISpell(SPELL_DISRUPTINGSHOUT, 50.0f, TARGET_SELF, 0, 8);
    addAISpell(SPELL_SABERLASH, 50.0f, TARGET_ATTACKING, 0, 8, false, true);
}

CreatureAIScript* DeathBoundWardAI::Create(Creature* pCreature) { return new DeathBoundWardAI(pCreature); }

//////////////////////////////////////////////////////////////////////////////////////////
/// Nerubar Broodkeeper
NerubarBroodkeeperAI::NerubarBroodkeeperAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    webBeam = addAISpell(SPELL_WEB_BEAM, 0.0f, TARGET_SELF, 0, 0);
    addAISpell(SPELL_CRYPT_SCARABS, 33.0f, TARGET_ATTACKING, 0, 1);

    auto mending = addAISpell(SPELL_DARK_MENDING, 15.0f, TARGET_RANDOM_FRIEND, 0, 3);
    mending->setMinMaxDistance(0.0f, 40.0f);
    mending->setMinMaxPercentHp(1.0f, 75.0f);

    auto webWrap = addAISpell(SPELL_WEB_WRAP, 33.0f, TARGET_RANDOM_SINGLE, 0, 16);
    webWrap->setMinMaxDistance(0.0f, 40.0f);
}

CreatureAIScript* NerubarBroodkeeperAI::Create(Creature* pCreature) { return new NerubarBroodkeeperAI(pCreature); }

void NerubarBroodkeeperAI::OnLoad()
{
    setDisableGravity(true);
    setImmuneToAll(true);
    getCreature()->setEmoteState(EMOTE_STATE_CUSTOM_SPELL_03);
}

void NerubarBroodkeeperAI::DoAction(int32_t const action)
{
    if (action != ACTION_NERUBAR_FALL)
        return;

    _castAISpell(webBeam);

    float x, y, z;
    getCreature()->getPosition(x, y);
    z = getCreature()->getFloorZ();
    getCreature()->SetSpawnLocation(x, y, z, getCreature()->GetOrientation());

    getCreature()->getMovementManager()->moveLand(POINT_LAND, LocationVector(x, y, z));
    getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
}

void NerubarBroodkeeperAI::OnReachWP(uint32_t type, uint32_t id)
{
    if (type == EFFECT_MOTION_TYPE && id == POINT_LAND)
    {
        setImmuneToAll(false);
        setDisableGravity(false);

        // Hackfix shouldnt be needed :/
        getCreature()->interruptSpell(SPELL_WEB_BEAM);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// 
DeathspeakerAttendantAI::DeathspeakerAttendantAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    addAISpell(SPELL_SHADOWBOLT, 50.0f, TARGET_RANDOM_SINGLE, 0, 8);
    addAISpell(getRaidModeValue(SPELL_SHADOWNOVA_10N, SPELL_SHADOWNOVA_25N, SPELL_SHADOWNOVA_10N, SPELL_SHADOWNOVA_25N), 50.0f, TARGET_VARIOUS, 0, 17);
}

CreatureAIScript* DeathspeakerAttendantAI::Create(Creature* pCreature) { return new DeathspeakerAttendantAI(pCreature); }

//////////////////////////////////////////////////////////////////////////////////////////
/// 
DeathspeakerDiscipleAI::DeathspeakerDiscipleAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    addAISpell(SPELL_DARKBLESSING, 33.0f, 20, [this]() { return getBestUnitTarget(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 30.0f); });
    addAISpell(SPELL_SHADOWBOLT2, 33.0f, 8, [this]() { return getBestUnitTarget(TargetFilter_None); });
    addAISpell(getRaidModeValue(SPELL_SHADOWMEND_10N, SPELL_SHADOWMEND_25N, SPELL_SHADOWMEND_10N, SPELL_SHADOWMEND_25N), 33.0f, 25, [this]() { return getBestUnitTarget(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f); });
}

CreatureAIScript* DeathspeakerDiscipleAI::Create(Creature* pCreature) { return new DeathspeakerDiscipleAI(pCreature); }

//////////////////////////////////////////////////////////////////////////////////////////
/// 
DeathspeakerZealotAI::DeathspeakerZealotAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    addAISpell(SPELL_SHADOWCLEAVE, 75.0f, 8, [this]() { return getBestUnitTarget(TargetFilter_Current); });
}

CreatureAIScript* DeathspeakerZealotAI::Create(Creature* pCreature) { return new DeathspeakerZealotAI(pCreature); }

//////////////////////////////////////////////////////////////////////////////////////////
/// 
DeathspeakerHighPriestAI::DeathspeakerHighPriestAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    spellDarkReck = addAISpell(SPELL_DARKRECKONING_EFFECT, 100.0f, TARGET_CUSTOM, 0, 19);
    addAISpell(SPELL_DARKRECKONING, 75.0f, 10,
        [this]()
        {
            Unit* target = getBestUnitTarget(TargetFilter_NotCurrent);
            spellDarkReck->setCustomTarget(target);
            return target;
        });
}

CreatureAIScript* DeathspeakerHighPriestAI::Create(Creature* pCreature) { return new DeathspeakerHighPriestAI(pCreature); }

//////////////////////////////////////////////////////////////////////////////////////////
/// 
DeathspeakerServantAI::DeathspeakerServantAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    addAISpell(getRaidModeValue(SPELL_CHAOSBOLT_10N, SPELL_CHAOSBOLT_25N, SPELL_CHAOSBOLT_10N, SPELL_CHAOSBOLT_25N), 33.0f, 15, [this]() { return getBestUnitTarget(TargetFilter_None); });
    addAISpell(SPELL_CONSUMINGSHADOWS, 33.0f, 13, [this]() { return getBestUnitTarget(TargetFilter_None); });
    addAISpell(getRaidModeValue(SPELL_CURSEOFAGONY_10N, SPELL_CURSEOFAGONY_25N, SPELL_CURSEOFAGONY_10N, SPELL_CURSEOFAGONY_25N), 33.0f, 17, [this]() { return getBestUnitTarget(TargetFilter_None); });
}

CreatureAIScript* DeathspeakerServantAI::Create(Creature* pCreature) { return new DeathspeakerServantAI(pCreature); }

//////////////////////////////////////////////////////////////////////////////////////////
/// Instance Setup
void SetupICC(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_ICECROWNCITADEL, &IceCrownCitadelScript::Create);

    //Teleporters
    mgr->register_gameobject_script(GO_TELE_1, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_1, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_2, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_2, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_3, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_3, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_4, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_4, new ICCTeleporterGossip());

    mgr->register_gameobject_script(GO_TELE_5, &ICCTeleporterAI::Create);
    mgr->register_go_gossip(GO_TELE_5, new ICCTeleporterGossip());

    // Trash
    mgr->register_creature_script(NPC_SERVANT_OF_THE_THRONE, &ServantOfTheThroneAI::Create);
    mgr->register_creature_script(NPC_ANCIENT_SKELETAL_SOLDIER, &AncientSkeletalSoldierAI::Create);
    mgr->register_creature_script(NPC_DEATHBOUND_WARD, &DeathBoundWardAI::Create);
    mgr->register_creature_script(NPC_NERUBAR_BROODKEEPER, &NerubarBroodkeeperAI::Create);
    mgr->register_creature_script(NPC_DEATHSPEAKER_ATTENDANT, &DeathspeakerAttendantAI::Create);
    mgr->register_creature_script(NPC_DEATHSPEAKER_DISCIPLE, &DeathspeakerDiscipleAI::Create);
    mgr->register_creature_script(NPC_DEATHSPEAKER_ZEALOT, &DeathspeakerZealotAI::Create);
    mgr->register_creature_script(NPC_DEATHSPEAKER_HIGH_PRIEST, &DeathspeakerHighPriestAI::Create);
    mgr->register_creature_script(NPC_DEATHSPEAKER_SERVANT, &DeathspeakerServantAI::Create);

    //Bosses
    mgr->register_creature_script(CN_LORD_MARROWGAR, &LordMarrowgarAI::Create);
    mgr->register_creature_script(CN_LADY_DEATHWHISPER, &LadyDeathwhisperAI::Create);
    mgr->register_creature_script(NPC_GB_MURADIN_BRONZEBEARD, &MuradinAI::Create);
    mgr->register_creature_script(NPC_GB_HIGH_OVERLORD_SAURFANG, &SaurfangAI::Create);
    mgr->register_creature_script(NPC_SE_MURADIN_BRONZEBEARD, &MuradinSaurfangEvent::Create);
    mgr->register_creature_script(NPC_SE_HIGH_OVERLORD_SAURFANG, &OverlordSaurfangEvent::Create);
    mgr->register_creature_script(CN_DEATHBRINGER_SAURFANG, &DeathbringerSaurfangAI::Create);
    //mgr->register_creature_script(CN_VALITHRIA_DREAMWALKER, &ValithriaDreamwalkerAI::Create);
    mgr->register_creature_script(gunshipIds, &GunshipAI::Create);

    //Spell Bone Storm
    mgr->register_spell_script(SPELL_BONE_STORM_EFFECT, new BoneStormDamage);
    mgr->register_spell_script(SPELL_BONE_STORM, new BoneStorm);

    // Spell Bone Spike Graveyard
    mgr->register_spell_script(SPELL_BONE_SPIKE_GRAVEYARD, new BoneSpikeGraveyard);

    // Spell Coldflame
    mgr->register_spell_script(SPELL_COLDFLAME_NORMAL, new Coldflame);
    mgr->register_spell_script(SPELL_COLDFLAME_DAMAGE, new ColdflameDamage);
    mgr->register_spell_script(SPELL_COLDFLAME_BONE_STORM, new ColdflameBonestorm);

    // Spell Bone Slice
    mgr->register_spell_script(SPELL_BONE_SLICE, new BoneSlice);

    // Spell Mana Barrier
    mgr->register_spell_script(SPELL_MANA_BARRIER, new ManaBarrier);

    // Spell Cultist Dark Martyrdom
    mgr->register_spell_script(SPELL_DARK_MARTYRDOM_ADHERENT, new DarkMartyrdom);

    // Spell Grip Of Agony
    mgr->register_spell_script(SPELL_GRIP_OF_AGONY, new GripOfAgony);

    // Generic Blood Link spell script
    uint32_t bloodLinkTriggerIds[] =
    {
        SPELL_BLOOD_NOVA_DAMAGE,
        SPELL_RUNE_OF_BLOOD_LEECH,
        0
    };
    mgr->register_spell_script(bloodLinkTriggerIds, new GenericBloodLinkTrigger);

    // Boiling Blood Spell
    mgr->register_spell_script(SPELL_BOILING_BLOOD, new BoilingBlood);

    mgr->register_spell_script(SPELL_BLOOD_NOVA_TRIGGER, new BloodNova);

    mgr->register_spell_script(SPELL_BLOOD_LINK, new BloodLink);
    mgr->register_spell_script(SPELL_BLOOD_LINK_DUMMY, new BloodLinkDummy);
    mgr->register_spell_script(SPELL_BLOOD_LINK_POWER, new BloodLinkEnergize);

    mgr->register_spell_script(SPELL_REMOVE_MARKS_OF_THE_FALLEN_CHAMPION, new RemoveMarksOfTheFallen);

    //Gossips
    GossipScript* MuradinGossipScript = new MuradinGossip();
    mgr->register_creature_gossip(NPC_GB_MURADIN_BRONZEBEARD, MuradinGossipScript);
    GossipScript* SaurfangGossipScript = new SaurfangGossip();
    mgr->register_creature_gossip(NPC_GB_HIGH_OVERLORD_SAURFANG, SaurfangGossipScript);
    GossipScript* MuradinSaurfangEventGossipScript = new MuradinSeGossip();
    mgr->register_creature_gossip(NPC_SE_MURADIN_BRONZEBEARD, MuradinSaurfangEventGossipScript);
    GossipScript* OverlordSaurfangEventGossipScript = new OverlordSeGossip();
    mgr->register_creature_gossip(NPC_SE_HIGH_OVERLORD_SAURFANG, OverlordSaurfangEventGossipScript);
    GossipScript* ZafodBoomboxGossipScript = new ZafodBoomboxGossip();
    mgr->register_creature_gossip(NPC_GB_ZAFOD_BOOMBOX, ZafodBoomboxGossipScript);

    //Misc
    mgr->register_creature_script(NPC_COLDFLAME, &ColdflameAI::Create);
    mgr->register_creature_script(NPC_BONE_SPIKE, &BoneSpikeAI::Create);

    mgr->register_creature_script(NPC_CULT_FANATIC, CultFanaticAI::Create);
    mgr->register_creature_script(NPC_CULT_ADHERENT, CultAdherentAI::Create);

    mgr->register_creature_script(NPC_SE_SKYBREAKER_MARINE, NpcSaurfangEventAI::Create);
    mgr->register_creature_script(NPC_SE_KOR_KRON_REAVER, NpcSaurfangEventAI::Create);

    mgr->register_creature_script(NPC_GB_ZAFOD_BOOMBOX, ZafodBoomboxAI::Create);

    mgr->register_creature_script(NPC_GB_GUNSHIP_HULL, GunshipHullAI::Create);

    mgr->register_creature_script(canonIds, GunshipCanonAI::Create);

    uint32_t achievementCriteriaIds[] =
    {
        12778,  // 10 nhc
        13036,  // 25 nhc
        13035,  // 10 hc
        13037,  // 25 hc
        0
    };

    mgr->register_achievement_criteria_script(achievementCriteriaIds, new achievement_ive_gone_and_made_a_mess);
}
