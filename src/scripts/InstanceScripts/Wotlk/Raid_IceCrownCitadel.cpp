/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_IceCrownCitadel.h"
#include "Objects/Faction.h"
#include "Units/Summons/Summon.h"
#include "../world/Objects/ObjectMgr.h"
#include "../world/Management/TransporterHandler.h"

//////////////////////////////////////////////////////////////////////////////////////////
//ICC zone: 4812
//Prepared creature entry:
//
//CN_DEATHBRINGER_SAURFANG    37813
//CN_FESTERGUT                36626
//CN_ROTFACE                  36627
//CN_PROFESSOR_PUTRICIDE      36678
//CN_PRINCE_VALANAR           37970
//N_BLOOD_QUEEN_LANATHEL     37955
//CN_SINDRAGOSA               36853
//CN_THE_LICHKING             36597
//
///\todo  start boss scripts
//////////////////////////////////////////////////////////////////////////////////////////
//Event: GunshipBattle
//
//Affects:
//Available teleports. If GunshipBattle done -> Teleportlocation 4 available.
//
//Devnotes:
//Far away from implementing this :(
//////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//IceCrownCitadel Instance
class IceCrownCitadelScript : public InstanceScript
{
public:

    explicit IceCrownCitadelScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        TeamInInstance = 3;
        MarrowgarIcewall1GUID = 0;
        MarrowgarIcewall2GUID = 0;
        MarrowgarEntranceDoorGUID = 0;
        LadyDeathwisperElevatorGUID = 0;
        LadyDeathwisperEntranceDoorGUID = 0;
        
        addData(DATA_BONED_ACHIEVEMENT, uint32_t(true));
        addData(DATA_GUNSHIP_EVENT, NotStarted);
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new IceCrownCitadelScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
        case GO_MARROWGAR_ICEWALL_1:
            MarrowgarIcewall1GUID = static_cast<uint32_t>(pGameObject->getGuid());
            break;

        case GO_MARROWGAR_ICEWALL_2:
            MarrowgarIcewall2GUID = static_cast<uint32_t>(pGameObject->getGuid());
            break;

        case GO_MARROWGAR_DOOR:
            MarrowgarEntranceDoorGUID = static_cast<uint32_t>(pGameObject->getGuid());
            break;
        case GO_ORATORY_OF_THE_DAMNED_ENTRANCE:
            LadyDeathwisperEntranceDoorGUID = static_cast<uint32_t>(pGameObject->getGuid());
            break;
        case GO_LADY_DEATHWHISPER_ELEVATOR:
            LadyDeathwisperElevatorGUID = static_cast<uint32_t>(pGameObject->getGuid());
            break;
        case GO_TELE_1:
        case GO_TELE_2:
        case GO_TELE_3:
        case GO_TELE_4:
        case GO_TELE_5:
            pGameObject->setFlags(GO_FLAG_NONE);
            break;
        }

        // State also must changes for Objects out of range so change state on spawning them :)
        SetGameobjectStates(pGameObject);
    }

    void SetGameobjectStates(GameObject* pGameObject)
    {
        // Gos which are not visible by killing a boss needs a second check...
        if (getData(CN_LORD_MARROWGAR) == Finished)
        {
            if (MarrowgarIcewall1GUID)
                GetGameObjectByGuid(MarrowgarIcewall1GUID)->setState(GO_STATE_OPEN);        // Icewall 1

            if (MarrowgarIcewall2GUID)
                GetGameObjectByGuid(MarrowgarIcewall2GUID)->setState(GO_STATE_OPEN);        // Icewall 2

            if (MarrowgarEntranceDoorGUID)
                GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_OPEN);    // Door  
        }

        if (getData(CN_LADY_DEATHWHISPER) == Finished)
        {
            if (LadyDeathwisperEntranceDoorGUID)
                GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_OPEN);

            if (LadyDeathwisperElevatorGUID)
                GetGameObjectByGuid(LadyDeathwisperElevatorGUID)->setState(GO_STATE_OPEN);
        }

        if (getData(CN_LADY_DEATHWHISPER) == NotStarted)
        {
            /*if (LadyDeathwisperElevatorGUID)
                GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_OPEN);

            if (LadyDeathwisperElevatorGUID)
            {
                GetGameObjectByGuid(LadyDeathwisperElevatorGUID)->setState(GO_STATE_CLOSED);
                GetGameObjectByGuid(LadyDeathwisperElevatorGUID)->setLevel(0);
            }*/
        }
    }

    void SpawnTransports(MapMgr* pMapMgr)
    {
        if (TeamInInstance == TEAM_ALLIANCE)
        {
            if (AllianceTransporterHordeShip = sObjectMgr.LoadTransportInInstance(pMapMgr, GO_ORGRIM_S_HAMMER_ALLIANCE_ICC, 108000))
            {
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_ORGRIMS_HAMMER, 1.845810f, 1.268872f, 34.526218f, 1.5890f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_HIGH_OVERLORD_SAURFANG, 37.18615f, 0.00016f, 36.78849f, 3.13683f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_INVISIBLE_STALKER, 37.18615f, 0.00016f, 36.78849f, 3.13683f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_BATTLE_MAGE, 47.2929f, -4.308941f, 37.5555f, 3.05033f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_BATTLE_MAGE, 47.34621f, 4.032004f, 37.70952f, 3.05033f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_BATTLE_MAGE, 15.03016f, 0.00016f, 37.70952f, 1.55138f);

                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -13.19547f, -27.160213f, 35.47252f, 3.10672f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -18.33902f, -25.230491f, 33.04052f, 3.00672f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -60.1251f, -1.27014f, 42.8335f, 5.16073f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -48.2651f, 16.78034f, 34.2515f, 0.04292f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -14.8356f, 27.931688f, 33.363f, 1.73231f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 10.2702f, 20.62966f, 35.37483f, 1.6f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 39.32459f, 14.50176f, 36.88428f, 1.6f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 46.17223f, -6.638763f, 37.35444f, 1.32f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 27.4456f, -13.397498f, 36.34746f, 1.6f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 18.16184f, 1.37897f, 35.31705f, 1.6f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -18.11516f, -0.196236f, 45.15709f, 2.9f);
                AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -18.11844f, -0.19624f, 49.18192f, 1.6f);

                if (GetDifficulty() == MODE_NORMAL_10MEN || GetDifficulty() == MODE_HEROIC_10MEN)
                {
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_AXETHROWER, -3.170555f, 28.30652f, 34.21082f, 1.66527f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_AXETHROWER, -12.0928f, 27.65942f, 33.58557f, 1.66527f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_AXETHROWER, 14.92804f, 26.18018f, 35.47803f, 1.66527f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_AXETHROWER, 24.70331f, 25.36584f, 35.97845f, 1.66527f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_ROCKETEER, -11.44849f, -25.71838f, 33.64343f, 1.49248f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_ROCKETEER, 12.30336f, -25.69653f, 35.32373f, 1.49248f);
                }
                else
                {
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_AXETHROWER, -3.170555f, 28.30652f, 34.21082f, 1.66527f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_AXETHROWER, -12.0928f, 27.65942f, 33.58557f, 1.66527f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_AXETHROWER, 14.92804f, 26.18018f, 35.47803f, 1.66527f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_AXETHROWER, 24.70331f, 25.36584f, 35.97845f, 1.66527f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_AXETHROWER, 19.92804f, 27.18018f, 35.47803f, 1.66527f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_AXETHROWER, -7.70331f, 28.36584f, 33.88557f, 1.66527f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_ROCKETEER, -11.44849f, -25.71838f, 33.64343f, 1.49248f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_ROCKETEER, 12.30336f, -25.69653f, 35.32373f, 1.49248f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_ROCKETEER, -3.44849f, -25.71838f, 34.21082f, 1.49248f);
                    AllianceTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_KORKRON_ROCKETEER, 3.30336f, -25.69653f, 35.32373f, 1.49248f);
                }
            }

            if (AllianceTransporterAllianceShip = sObjectMgr.LoadTransportInInstance(pMapMgr, GO_THE_SKYBREAKER_ALLIANCE_ICC, 108000))
            {
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER, -17.156807f, -1.633260f, 20.81273f, 4.52672f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_MURADIN_BRONZEBEARD, 13.51547f, -0.160213f, 20.87252f, 3.10672f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_HIHG_CAPTAIN_JUSTIN_BARTLETT, 42.78902f, -0.010491f, 25.24052f, 3.00672f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_HIGH_OVERLORD_SAURFANG_NV, -12.9806f, -22.9462f, 21.659f, 4.72416f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_ZAFOD_BOOMBOX, 18.8042f, 9.907914f, 20.33559f, 3.10672f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_DECKHAND, -64.8423f, 4.4658f, 23.4352f, 2.698897f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_DECKHAND, 35.54972f, 19.93269f, 25.0333f, 4.71242f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_DECKHAND, -36.39837f, 3.13127f, 20.4496f, 1.5708f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_DECKHAND, -36.23974f, -2.75767f, 20.4506f, 4.69496f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_DECKHAND, 41.94677f, 44.08411f, 24.66587f, 1.62032f);

                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 13.51547f, -0.160213f, 20.87252f, 3.10672f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 42.78902f, -0.010491f, 25.24052f, 3.00672f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 14.0551f, 3.65014f, 20.7935f, 3.16073f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 14.0551f, -4.65034f, 20.7915f, 3.04292f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -17.8356f, 0.031688f, 20.823f, 4.73231f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -34.2702f, -26.18966f, 21.37483f, 1.6f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -11.64459f, -19.85176f, 20.88428f, 1.6f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -19.88223f, -6.578763f, 20.57444f, 1.6f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -41.4456f, -7.647498f, 20.49746f, 1.6f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 0.554884f, -1.232897f, 20.53705f, 1.6f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -50.16516f, 9.716236f, 23.58709f, 1.6f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 11.45844f, 16.36624f, 20.54192f, 1.6f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 19.72286f, -2.193787f, 33.06982f, 1.6f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 19.72286f, -2.193787f, 33.06982f, 1.6f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 8.599396f, -28.55855f, 24.79919f, 1.6f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 38.94339f, -33.808f, 25.39618f, 1.6f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 58.15474f, 0.748094f, 41.87663f, 1.6f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 5.607554f, -6.350654f, 34.00357f, 1.6f);
                AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 4.780305f, -29.05227f, 35.09634f, 1.6f);

                if (GetDifficulty() == MODE_NORMAL_10MEN || GetDifficulty() == MODE_HEROIC_10MEN)
                {
                    AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_ALLIANCE_CANON, -5.15231f, -22.9462f, 21.659f, 4.72416f);
                    AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_ALLIANCE_CANON, -28.0876f, -22.9462f, 21.659f, 4.72416f);
                }
                else
                {
                    AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_ALLIANCE_CANON, -5.15231f, -22.9462f, 21.659f, 4.72416f);
                    AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_ALLIANCE_CANON, -14.9806f, -22.9462f, 21.659f, 4.72416f);
                    AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_ALLIANCE_CANON, -21.7406f, -22.9462f, 21.659f, 4.72416f);
                    AllianceTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_ALLIANCE_CANON, -28.0876f, -22.9462f, 21.659f, 4.72416f);
                }
            }
        }
        else if(TeamInInstance == TEAM_HORDE)
        {
            if (HordeTransporterAllianceShip = sObjectMgr.LoadTransportInInstance(pMapMgr, GO_THE_SKYBREAKER_HORDE_ICC, 77800))
            {
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER, -17.156807f, -1.633260f, 20.81273f, 4.52672f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_MURADIN_BRONZEBEARD, 13.51547f, -0.160213f, 20.87252f, 3.10672f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_HIHG_CAPTAIN_JUSTIN_BARTLETT, 42.78902f, -0.010491f, 25.24052f, 3.00672f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_SORCERERS, 14.0551f, 3.65014f, 20.7935f, 3.16073f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_SORCERERS, 14.0551f, -4.65034f, 20.7915f, 3.04292f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_SORCERERS, -17.8356f, 0.031688f, 20.823f, 4.73231f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 13.51547f, -0.160213f, 20.87252f, 3.10672f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 42.78902f, -0.010491f, 25.24052f, 3.00672f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 14.0551f, 3.65014f, 20.7935f, 3.16073f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 14.0551f, -4.65034f, 20.7915f, 3.04292f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -17.8356f, 0.031688f, 20.823f, 4.73231f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -34.2702f, -26.18966f, 21.37483f, 1.6f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -11.64459f, -19.85176f, 20.88428f, 1.6f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -19.88223f, -6.578763f, 20.57444f, 1.6f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -41.4456f, -7.647498f, 20.49746f, 1.6f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 0.554884f, -1.232897f, 20.53705f, 1.6f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -50.16516f, 9.716236f, 23.58709f, 1.6f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 11.45844f, 16.36624f, 20.54192f, 1.6f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 19.72286f, -2.193787f, 33.06982f, 1.6f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 19.72286f, -2.193787f, 33.06982f, 1.6f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 8.599396f, -28.55855f, 24.79919f, 1.6f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 38.94339f, -33.808f, 25.39618f, 1.6f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 58.15474f, 0.748094f, 41.87663f, 1.6f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 5.607554f, -6.350654f, 34.00357f, 1.6f);
                HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 4.780305f, -29.05227f, 35.09634f, 1.6f);

                if (GetDifficulty() == MODE_NORMAL_10MEN || GetDifficulty() == MODE_HEROIC_10MEN)
                {
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_RIFLEMAN, -5.15231f, -22.9462f, 21.659f, 4.72416f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_RIFLEMAN, -14.9806f, -22.9462f, 21.659f, 4.72416f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_RIFLEMAN, -21.7406f, -22.9462f, 21.659f, 4.72416f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_RIFLEMAN, -28.0876f, -22.9462f, 21.659f, 4.72416f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_MORTAR_SOLDIER, -8.61003f, 15.483f, 20.4158f, 4.69854f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_MORTAR_SOLDIER, -27.9583f, 14.8875f, 20.4428f, 4.77865f);
                }
                else
                {
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_RIFLEMAN, 0.15231f, -22.9462f, 21.659f, 4.72416f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_RIFLEMAN, -5.15231f, -22.9462f, 21.659f, 4.72416f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_RIFLEMAN, -14.9806f, -22.9462f, 21.659f, 4.72416f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_RIFLEMAN, -21.7406f, -22.9462f, 21.659f, 4.72416f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_RIFLEMAN, -28.0876f, -22.9462f, 21.659f, 4.72416f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_RIFLEMAN, -33.0876f, -22.9462f, 21.659f, 4.72416f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_MORTAR_SOLDIER, -8.61003f, 15.483f, 20.4158f, 4.69854f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_MORTAR_SOLDIER, -27.9583f, 14.8875f, 20.4428f, 4.77865f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_MORTAR_SOLDIER, -15.61003f, 15.483f, 20.4158f, 4.69854f);
                    HordeTransporterAllianceShip->AddNPCPassengerInInstance(NPC_GB_SKYBREAKER_MORTAR_SOLDIER, -20.9583f, 14.8875f, 20.4428f, 4.77865f);
                }

            }

            if (HordeTransporterHordeShip = sObjectMgr.LoadTransportInInstance(pMapMgr, GO_ORGRIM_S_HAMMER_HORDE_ICC, 77800))
            {
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_ORGRIMS_HAMMER, 1.845810f, 1.268872f, 34.526218f, 1.5890f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_HIGH_OVERLORD_SAURFANG, 37.18615f, 0.00016f, 36.78849f, 3.13683f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_MURADIN_BRONZEBEARD_NV, -7.09684f, 30.582f, 34.5013f, 1.53591f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_INVISIBLE_STALKER, 37.30764f, -0.143823f, 36.7936f, 3.13683f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_ZAFOD_BOOMBOX, 35.18615f, 15.30652f, 37.64343f, 3.05033f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -13.19547f, -27.160213f, 35.47252f, 3.10672f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -18.33902f, -25.230491f, 33.04052f, 3.00672f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -60.1251f, -1.27014f, 42.8335f, 5.16073f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -48.2651f, 16.78034f, 34.2515f, 0.04292f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -14.8356f, 27.931688f, 33.363f, 1.73231f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 10.2702f, 20.62966f, 35.37483f, 1.6f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 39.32459f, 14.50176f, 36.88428f, 1.6f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 46.17223f, -6.638763f, 37.35444f, 1.32f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 27.4456f, -13.397498f, 36.34746f, 1.6f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, 18.16184f, 1.37897f, 35.31705f, 1.6f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -18.11516f, -0.196236f, 45.15709f, 2.9f);
                HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_GUNSHIP_HULL, -18.11844f, -0.19624f, 49.18192f, 1.6f);

                if (GetDifficulty() == MODE_NORMAL_10MEN || GetDifficulty() == MODE_HEROIC_10MEN)
                {
                    HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_HORDE_CANON, 22.6225f, 28.9309f, 36.3929f, 1.53591f);
                    HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_HORDE_CANON, -21.7509f, 29.4207f, 34.2588f, 1.53591f);
                }
                else
                {
                    HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_HORDE_CANON, 22.6225f, 28.9309f, 36.3929f, 1.53591f);
                    HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_HORDE_CANON, 9.87745f, 30.5047f, 35.7147f, 1.53591f);
                    HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_HORDE_CANON, -7.09684f, 30.582f, 34.5013f, 1.53591f);
                    HordeTransporterHordeShip->AddNPCPassengerInInstance(NPC_GB_HORDE_CANON, -21.7509f, 29.4207f, 34.2588f, 1.53591f);
                }
            }
        }
    }

    void OnPlayerEnter(Player* player) override
    {
        if(TeamInInstance == 3)
            TeamInInstance = player->getTeam();

        if (!spawnsCreated())
        {
            // setup only the npcs with the correct team...
            switch (TeamInInstance)
            {
                case TEAM_ALLIANCE:
                    for (uint8_t i = 0; i < 13; i++)
                        spawnCreature(AllySpawns[i].entry, AllySpawns[i].x, AllySpawns[i].y, AllySpawns[i].z, AllySpawns[i].o, AllySpawns[i].faction);
                    break;
                case TEAM_HORDE:
                    for (uint8_t i = 0; i < 13; i++)
                        spawnCreature(HordeSpawns[i].entry, HordeSpawns[i].x, HordeSpawns[i].y, HordeSpawns[i].z, HordeSpawns[i].o, HordeSpawns[i].faction);
                    break;
            }
            
            SpawnTransports(GetInstance());

            setSpawnsCreated();
        }
        else
            sObjectMgr.LoadTransportForPlayers(player);
    }

    void OnEncounterStateChange(uint32_t entry, uint32_t state) override
    {
        switch (entry)
        {
        case CN_LORD_MARROWGAR:
            if (state == InProgress)
            {
                if (MarrowgarEntranceDoorGUID)
                    GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_CLOSED);
            }
                
            if (state == NotStarted)
            {
                if (MarrowgarEntranceDoorGUID)
                    GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_OPEN);
            }
                
            if (state == Finished)
            {
                if (MarrowgarIcewall1GUID)
                    GetGameObjectByGuid(MarrowgarIcewall1GUID)->setState(GO_STATE_OPEN);        // Icewall 1

                if (MarrowgarIcewall2GUID)
                    GetGameObjectByGuid(MarrowgarIcewall2GUID)->setState(GO_STATE_OPEN);        // Icewall 2

                if (MarrowgarEntranceDoorGUID)
                    GetGameObjectByGuid(MarrowgarEntranceDoorGUID)->setState(GO_STATE_OPEN);    // Door  
            }
            break;
        case CN_LADY_DEATHWHISPER:
            if (state == InProgress)
            {
                if (LadyDeathwisperEntranceDoorGUID)
                    GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_CLOSED);
            }        

            if (state == NotStarted)
            {
                if (LadyDeathwisperElevatorGUID)
                    GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_CLOSED);

                if (LadyDeathwisperElevatorGUID)
                    GetGameObjectByGuid(LadyDeathwisperElevatorGUID)->setState(GO_STATE_OPEN);
            }
            if (state == Finished)
            {
                if (LadyDeathwisperEntranceDoorGUID)
                    GetGameObjectByGuid(LadyDeathwisperEntranceDoorGUID)->setState(GO_STATE_OPEN);

                if (LadyDeathwisperElevatorGUID)
                    GetGameObjectByGuid(LadyDeathwisperElevatorGUID)->setState(GO_STATE_OPEN);
            }
            break;
        }
    }

    //Function start motion of the ship
    void StartFlyShip(Transporter* t)
    {
        t->BuildStartMovePacket(mInstance);
        t->setDynamic(0x10830010); // Seen in sniffs
        t->setParentRotation(3, 1.0f);

        GameObjectProperties const* goinfo = t->GetGameObjectProperties();
        t->GenerateWaypoints(goinfo->raw.parameter_0);      
    }

    //Function stop motion of the ship
    void StopFlyShip(Transporter* t)
    {
        t->BuildStopMovePacket(mInstance);
        RelocateTransport(t);    
    }

    void RelocateTransport(Transporter* t)
    {
        InstanceScript* instance = t->GetMapMgr()->GetScript();

        if (!t || !instance)
            return;

        // Transoprt movemend on server-side is ugly hack, so we do sincronize positions
        switch (t->getEntry())
        {
        case GO_THE_SKYBREAKER_ALLIANCE_ICC:
            if (instance->getData(DATA_GUNSHIP_EVENT) != Performed)
                static_cast<Object*>(t)->SetPosition(-377.184021f, 2073.548584f, 445.753387f, t->GetOrientation());
            else if (instance->getData(DATA_GUNSHIP_EVENT) == Performed)
                static_cast<Object*>(t)->SetPosition(-583.942627f, 2212.364990f, 534.673889f, t->GetOrientation());
            break;
        case GO_ORGRIM_S_HAMMER_ALLIANCE_ICC:
            if (instance->getData(DATA_GUNSHIP_EVENT) != Performed)
                static_cast<Object*>(t)->SetPosition(-384.878479f, 1989.831665f, 431.549438f, t->GetOrientation());
            break;
        case GO_ORGRIM_S_HAMMER_HORDE_ICC:
            if (instance->getData(DATA_GUNSHIP_EVENT) != Performed)
                static_cast<Object*>(t)->SetPosition(-438.142365f, 2395.725830f, 436.781647f, t->GetOrientation());
            else if (instance->getData(DATA_GUNSHIP_EVENT) == Performed)
                static_cast<Object*>(t)->SetPosition(-583.942627f, 2212.364990f, 534.673889f, t->GetOrientation());
            break;
        case GO_THE_SKYBREAKER_HORDE_ICC:
            if (instance->getData(DATA_GUNSHIP_EVENT) != Performed)
                static_cast<Object*>(t)->SetPosition(-435.854156f, 2475.328125f, 449.364105f, t->GetOrientation());
            break;
        }

        t->m_timer = 0;
        t->UpdateNPCPositions();
    }

public:
        Transporter* AllianceTransporterAllianceShip;
        Transporter* AllianceTransporterHordeShip;

        Transporter* HordeTransporterAllianceShip;
        Transporter* HordeTransporterHordeShip;

protected:
    uint8_t TeamInInstance;
    uint32_t MarrowgarIcewall1GUID;
    uint32_t MarrowgarIcewall2GUID;
    uint32_t MarrowgarEntranceDoorGUID;
    uint32_t LadyDeathwisperElevatorGUID;
    uint32_t LadyDeathwisperEntranceDoorGUID;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// IceCrown Teleporter
class ICCTeleporterGossip : public GossipScript
{
public:

    void onHello(Object* object, Player* player) override
    {
        InstanceScript* pInstance = player->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        GossipMenu menu(object->getGuid(), 15221, player->GetSession()->language);
        menu.addItem(GOSSIP_ICON_CHAT, 515, 0);     // Teleport to Light's Hammer.

        if (pInstance->getData(CN_LORD_MARROWGAR) == Finished)
            menu.addItem(GOSSIP_ICON_CHAT, 516, 1);      // Teleport to Oratory of The Damned.

        if (pInstance->getData(CN_LADY_DEATHWHISPER) == Finished)
            menu.addItem(GOSSIP_ICON_CHAT, 517, 2);      // Teleport to Rampart of Skulls.

        // GunshipBattle has to be finished...
        //menu.addItem(GOSSIP_ICON_CHAT, (518), 3);        // Teleport to Deathbringer's Rise.

        if (pInstance->getData(CN_VALITHRIA_DREAMWALKER) == Finished)
            menu.addItem(GOSSIP_ICON_CHAT, 519, 4);      // Teleport to the Upper Spire.

        if (pInstance->getData(NPC_COLDFLAME) == Finished)
            menu.addItem(GOSSIP_ICON_CHAT, 520, 5);      // Teleport to Sindragosa's Lair.

        menu.sendGossipPacket(player);
    }

    void onSelectOption(Object* /*object*/, Player* player, uint32_t Id, const char* /*enteredcode*/, uint32_t /*gossipId*/) override
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
};

class ICCTeleporterAI : public GameObjectAIScript
{
public:

    explicit ICCTeleporterAI(GameObject* go) : GameObjectAIScript(go) {}

    ~ICCTeleporterAI() {}

    static GameObjectAIScript* Create(GameObject* go) { return new ICCTeleporterAI(go); }

    void OnActivate(Player* player) override
    {
        ICCTeleporterGossip gossip;
        gossip.onHello(_gameobject, player);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Lord Marrowgar
class LordMarrowgarAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LordMarrowgarAI)
    explicit LordMarrowgarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        boneStormDuration = RAID_MODE<uint32_t>(20000, 30000, 20000, 30000);
        baseSpeed = pCreature->getSpeedRate(TYPE_RUN, false);
        introDone = false;
        boneSlice = false;
        boneStormtarget = nullptr;
        coldflameLastPos = getCreature()->GetPosition();

        // Scripted Spells not autocastet
        boneSliceSpell = addAISpell(SPELL_BONE_SLICE, 0.0f, TARGET_ATTACKING);
        boneStormSpell = addAISpell(SPELL_BONE_STORM, 0.0f, TARGET_SELF);
        boneStormSpell->addDBEmote(SAY_MARR_BONE_STORM_EMOTE);

        boneSpikeGraveyardSpell = addAISpell(SPELL_BONE_SPIKE_GRAVEYARD, 0.0f, TARGET_SELF);
        coldflameNormalSpell = addAISpell(SPELL_COLDFLAME_NORMAL, 0.0f, TARGET_SELF);
        coldflameBoneStormSpell = addAISpell(SPELL_COLDFLAME_BONE_STORM, 0.0f, TARGET_SELF);

        berserkSpell = addAISpell(SPELL_BERSERK, 0.0f, TARGET_SELF);
        berserkSpell->addDBEmote(SAY_MARR_BERSERK);                  // THE MASTER'S RAGE COURSES THROUGH ME!
        berserkSpell->mIsTriggered = true;

        // Messages
        addEmoteForEvent(Event_OnCombatStart, SAY_MARR_AGGRO);     // The Scourge will wash over this world as a swarm of death and destruction!
        addEmoteForEvent(Event_OnTargetDied, SAY_MARR_KILL_1);      // More bones for the offering!
        addEmoteForEvent(Event_OnTargetDied, SAY_MARR_KILL_2);      // Languish in damnation!
        addEmoteForEvent(Event_OnDied, SAY_MARR_DEATH);            // I see... Only darkness.
    }

    void OnLoad()
    {
        sendDBChatMessage(SAY_MARR_ENTER_ZONE);      // This is the beginning AND the end, mortals. None may enter the master's sanctum!
        introDone = true;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        // common events
        scriptEvents.addEvent(EVENT_ENABLE_BONE_SLICE, 10000);
        scriptEvents.addEvent(EVENT_BONE_SPIKE_GRAVEYARD, Util::getRandomInt(10000, 15000));
        scriptEvents.addEvent(EVENT_COLDFLAME, 5000);
        scriptEvents.addEvent(EVENT_WARN_BONE_STORM, Util::getRandomInt(45000, 50000));
        scriptEvents.addEvent(EVENT_ENRAGE, 600000);
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        Reset();
    }

    void Reset()
    {
        scriptEvents.resetEvents();

        getCreature()->setSpeedRate(TYPE_RUN, baseSpeed, true);
        getCreature()->RemoveAura(SPELL_BONE_STORM);
        getCreature()->RemoveAura(SPELL_BERSERK);

        boneSlice = false;
        boneSpikeImmune.clear();
    }

    void AIUpdate() override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        if (_isCasting())
            return;

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_BONE_SPIKE_GRAVEYARD:
                if (_isHeroic() || !getCreature()->HasAura(SPELL_BONE_STORM))
                    _castAISpell(boneSpikeGraveyardSpell);

                scriptEvents.addEvent(EVENT_BONE_SPIKE_GRAVEYARD, Util::getRandomInt(15000, 20000));
                break;
            case EVENT_COLDFLAME:
                coldflameLastPos = getCreature()->GetPosition();

                if (!getCreature()->HasAura(SPELL_BONE_STORM))
                    _castAISpell(coldflameNormalSpell);
                else
                    _castAISpell(coldflameBoneStormSpell);
                
                scriptEvents.addEvent(EVENT_COLDFLAME, 5000);
                break;
            case EVENT_WARN_BONE_STORM:
                boneSlice = false;
                _castAISpell(boneStormSpell);

                scriptEvents.delayEvent(EVENT_BONE_SPIKE_GRAVEYARD, 3000);
                scriptEvents.delayEvent(EVENT_COLDFLAME, 3000);

                scriptEvents.addEvent(EVENT_BONE_STORM_BEGIN, 3050);
                scriptEvents.addEvent(EVENT_WARN_BONE_STORM, Util::getRandomInt(90000, 95000));
            case EVENT_BONE_STORM_BEGIN:
                getCreature()->setSpeedRate(TYPE_RUN, baseSpeed*3.0f, true);
                sendDBChatMessage(SAY_MARR_BONE_STORM); // BONE STORM!
                
                scriptEvents.addEvent(EVENT_BONE_STORM_END, boneStormDuration + 1);
            case EVENT_BONE_STORM_MOVE:
            {
                scriptEvents.addEvent(EVENT_BONE_STORM_MOVE, boneStormDuration / 3);

                boneStormtarget= getBestPlayerTarget(TargetFilter_NotCurrent);
                if (!boneStormtarget)
                    boneStormtarget = getBestPlayerTarget(TargetFilter_Aggroed);
                
                if (boneStormtarget)
                    getCreature()->GetAIInterface()->MoveTo(boneStormtarget->GetPositionX(), boneStormtarget->GetPositionY(), boneStormtarget->GetPositionZ());

                break;
            }
            case EVENT_BONE_STORM_END:  
                getCreature()->setSpeedRate(TYPE_RUN, baseSpeed, true);
                scriptEvents.removeEvent(EVENT_BONE_STORM_MOVE);
                scriptEvents.addEvent(EVENT_ENABLE_BONE_SLICE, 10000);

                if (!_isHeroic())
                    scriptEvents.addEvent(EVENT_BONE_SPIKE_GRAVEYARD, Util::getRandomInt(15000, 20000));
                break;
            case EVENT_ENABLE_BONE_SLICE:
                boneSlice = true;
                break;
            case EVENT_ENRAGE:
                _castAISpell(berserkSpell);
                break;
            }
        }
     
        // We should not melee attack when storming
        if (getCreature()->HasAura(SPELL_BONE_STORM))
        {
            _setMeleeDisabled(true);
            return;
        }

        _setMeleeDisabled(false);

        // 10 seconds since encounter start Bone Slice replaces melee attacks
        if (boneSlice)
        {
            _castAISpell(boneSliceSpell);
        }

    }

    LocationVector const* GetLastColdflamePosition() const
    {
        return &coldflameLastPos;
    }

    void SetLastColdflamePosition(LocationVector pos)
    {
        coldflameLastPos = pos;
    }

    virtual void SetCreatureData64(uint32 Type, uint64 Data)
    {
        switch (Type)
        {
        case DATA_COLDFLAME_GUID:
            coldflameTarget = Data;
            break;
        case DATA_SPIKE_IMMUNE:
            boneSpikeImmune.push_back(Data);
            break;
        }
    }

    virtual uint64 GetCreatureData64(uint32 Type) const
    { 
        switch (Type)
        {
        case DATA_COLDFLAME_GUID:
            return coldflameTarget;
        case DATA_SPIKE_IMMUNE + 0:
        case DATA_SPIKE_IMMUNE + 1:
        case DATA_SPIKE_IMMUNE + 2:
        {
            uint32_t index = Type - DATA_SPIKE_IMMUNE;
            if (index < boneSpikeImmune.size())
                return boneSpikeImmune[index];

            break;
        }
        }

        return 0;
    }

    virtual void DoAction(int32 const action)
    {
        if (action != ACTION_CLEAR_SPIKE_IMMUNITIES)
            return;

        boneSpikeImmune.clear();
    }

protected:
    // Common
    InstanceScript* mInstance;
    float baseSpeed;
    bool introDone;
    bool boneSlice;

    Unit* boneStormtarget;
    LocationVector coldflameLastPos;
    uint64_t coldflameTarget;
    std::vector<uint64_t> boneSpikeImmune;

    // Spells
    CreatureAISpells* boneSliceSpell;
    CreatureAISpells* boneStormSpell;
    CreatureAISpells* boneSpikeGraveyardSpell;
    CreatureAISpells* coldflameNormalSpell;
    CreatureAISpells* coldflameBoneStormSpell;
    CreatureAISpells* berserkSpell;

    uint32_t boneStormDuration;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cold Flame
class ColdflameAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ColdflameAI)
    explicit ColdflameAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();     
        coldflameTriggerSpell = addAISpell(SPELL_COLDFLAME_SUMMON, 0.0f, TARGET_SOURCE);
        coldflameTriggerSpell->mIsTriggered = true;
    }

    void OnLoad() override
    {
        getCreature()->setVisible(false);
    }

    void OnSummon(Unit* summoner) override
    {
        if (!mInstance || !summoner->isCreature())
            return;

        if (summoner->HasAura(SPELL_BONE_STORM))
        {
            // Bonestorm X Pattern
            if (LordMarrowgarAI* marrowgarAI = static_cast<LordMarrowgarAI*>(static_cast<Creature*>(summoner)->GetScript()))
            {
                LocationVector const* ownerPos = marrowgarAI->GetLastColdflamePosition();
                float angle = ownerPos->o / M_PI_FLOAT * 180.0f;
                MoveTeleport(ownerPos->x , ownerPos->y, ownerPos->z, ownerPos->o);
                // Store last Coldflame Position and add 90 degree to create x pattern
                LocationVector nextPos;
                nextPos.x = ownerPos->x;
                nextPos.y = ownerPos->y;
                nextPos.z = ownerPos->z;
                nextPos.o = angle + 90 * M_PI_FLOAT / 180.0f;

                marrowgarAI->SetLastColdflamePosition(nextPos);
            }
        }
        // Random target Case
        else
        {
            Unit* target = mInstance->GetInstance()->GetUnit(static_cast<Creature*>(summoner)->GetScript()->GetCreatureData64(DATA_COLDFLAME_GUID));
            if (!target)
            {
                getCreature()->Despawn(100, 0);
                return;
            }        
            MoveTeleport(summoner->GetPosition());

            getCreature()->SetOrientation(getCreature()->calcAngle(summoner->GetPositionX(), summoner->GetPositionY(), target->GetPositionX(), target->GetPositionY()) * M_PI_FLOAT / 180.0f);
        }        
        MoveTeleport(summoner->GetPositionX(), summoner->GetPositionY(),summoner->GetPositionZ(), getCreature()->GetOrientation());
        scriptEvents.addEvent(EVENT_COLDFLAME_TRIGGER, 500);
    }

    void AIUpdate() override
    {
        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        if (scriptEvents.getFinishedEvent() == EVENT_COLDFLAME_TRIGGER)
        {
            LocationVector newPos;
            newPos = getCreature()->GetPosition();

            float angle = newPos.o;
            float destx, desty, destz;
            destx = newPos.x + 5.0f * std::cos(angle);
            desty = newPos.y + 5.0f * std::sin(angle);
            destz = newPos.z;

            newPos.x = destx;
            newPos.y = desty;
            newPos.z = destz;

            MoveTeleport(newPos.x, newPos.y, newPos.z, newPos.o);
            _castAISpell(coldflameTriggerSpell);         
            scriptEvents.addEvent(EVENT_COLDFLAME_TRIGGER, 500);
        }
    }

protected:
    // Common
    InstanceScript* mInstance;

    // Unit
    Unit* summoner;

    //Spells
    CreatureAISpells* coldflameTriggerSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Bone Spike
class BoneSpikeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BoneSpikeAI)
    explicit BoneSpikeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        // Common
        hasTrappedUnit = false;
        summon = nullptr;

        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
    }

    void OnSummon(Unit* summoner) override
    {
        summon = summoner;
        // Make our Creature in Combat otherwise on Died Script wont trigger
        getCreature()->GetAIInterface()->setAiScriptType(AI_SCRIPT_AGRO);

        getCreature()->castSpell(summoner, SPELL_IMPALED, false);
        summoner->castSpell(getCreature(), SPELL_RIDE_VEHICLE, true);
        scriptEvents.addEvent(EVENT_FAIL_BONED, 8000);
        hasTrappedUnit = true;
    }

    void OnTargetDied(Unit* pTarget) override
    {
        getCreature()->Despawn(100, 0);
        pTarget->RemoveAura(SPELL_IMPALED);
    }

    void OnDied(Unit* pTarget) override
    {       
        if (summon)
            summon->RemoveAura(SPELL_IMPALED);
         
        getCreature()->Despawn(100, 0);
    }

    void AIUpdate() override
    {
        if (!hasTrappedUnit)
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        if (scriptEvents.getFinishedEvent() == EVENT_FAIL_BONED)
            if (mInstance)
                mInstance->setData(DATA_BONED_ACHIEVEMENT, uint32_t(false));
    }

protected:
    // Common
    InstanceScript* mInstance;

    // Summon
    Unit* summon;

    bool hasTrappedUnit;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Storm
class BoneStorm : public SpellScript
{
public:
    void onAuraCreate(Aura* aur) override
    {
        // set duration here
        int32_t duration = 20000;
        if(aur->GetUnitCaster()->isCreature())
            duration = static_cast<Creature*>(aur->GetUnitCaster())->GetScript()->RAID_MODE<uint32_t>(20000, 30000, 20000, 30000);

        aur->setOriginalDuration(duration);
        aur->setMaxDuration(duration);
        aur->setTimeLeft(duration);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Storm Damage
class BoneStormDamage : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg) override
    {
        if (effIndex != EFF_INDEX_0 || spell->GetUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        auto distance = spell->GetUnitTarget()->GetDistance2dSq(spell->getCaster());
        // If target is closer than 5 yards, do full damage
        if (distance <= 5.0f)
            distance = 1.0f;

        *dmg = float2int32(*dmg / distance);
        return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Spike Graveyard
class BoneSpikeGraveyard : public SpellScript
{
    public:
        virtual void onAuraApply(Aura* aur)
        {
            aur->removeAuraEffect(EFF_INDEX_1);

            if (Creature* marrowgar = static_cast<Creature*>(aur->GetUnitCaster()))
            {
                CreatureAIScript* marrowgarAI = marrowgar->GetScript();
                if (!marrowgarAI)
                    return;

                uint8_t boneSpikeCount = uint8_t(aur->GetUnitCaster()->GetMapMgr()->pInstance->m_difficulty & 1 ? 3 : 1);
                std::list<Unit*> targets;

                targets.clear();

                uint8_t target = 0;
                for (target; target < boneSpikeCount; ++target)
                    targets.push_back(GetRandomTargetNotMainTank(marrowgar));

                if (targets.empty())
                    return;

                uint32 i = 0;
                for (std::list<Unit*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr, ++i)
                {
                    Unit* target = *itr;
                    target->castSpell(target, BoneSpikeSummonId[i], true);
                }


                std::vector<uint32_t> emoteVector;
                emoteVector.clear();
                emoteVector.push_back(SAY_MARR_BONESPIKE_1);// Stick Around
                emoteVector.push_back(SAY_MARR_BONESPIKE_2);// The only Escape is Darkness 
                emoteVector.push_back(SAY_MARR_BONESPIKE_3);// More Bones for the offering

                marrowgarAI->sendRandomDBChatMessage(emoteVector);  
            }
        }

        virtual SpellCastResult onCanCast(Spell* spell, uint32_t* parameter1, uint32_t* parameter2)
        {          
            if (Creature* marrowgar = static_cast<Creature*>(spell->getUnitCaster()))

                if(GetRandomTargetNotMainTank(marrowgar))
                    return SpellCastResult::SPELL_CAST_SUCCESS;

            return SpellCastResult::SPELL_FAILED_SUCCESS;
        }

        Unit* GetRandomTargetNotMainTank(Creature* caster)
        {
            Unit* target = nullptr;
            std::vector<Player*> players;

            Unit* mt = caster->GetAIInterface()->GetMostHated();
            if (mt == nullptr || !mt->isPlayer())
                return 0;

            for (const auto& itr : caster->getInRangePlayersSet())
            {
                Player* obj = static_cast<Player*>(itr);
                if (obj != mt && CheckTarget(obj, caster->GetScript()))
                    players.push_back(obj);
            }

            if (players.size())
                target = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];

            return target;
        }

        bool CheckTarget(Unit* target, CreatureAIScript* creatureAI)
        {
            if (target->getObjectTypeId() != TYPEID_PLAYER)
                return false;

            if (target->HasAura(SPELL_IMPALED))
                return false;

            // Check if it is one of the tanks soaking Bone Slice
            for (uint32 i = 0; i < MAX_BONE_SPIKE_IMMUNE; ++i)
                if (target->getGuid() == creatureAI->GetCreatureData64(DATA_SPIKE_IMMUNE + i))
                    return false;

            return true;
        }        
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame
class Coldflame : public SpellScript
{
public:
    virtual void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets)
    {
        if (effectIndex != EFF_INDEX_0)
            return;

        effectTargets->clear();

        Unit* coldflametarget = nullptr;
        Creature* pCreature = nullptr;

        if(spell->getUnitCaster()->isCreature())
            pCreature = static_cast<Creature*>(spell->getUnitCaster());

        // select any unit but not the tank 
        if (pCreature)
        {
            coldflametarget = pCreature->GetScript()->getBestPlayerTarget(TargetFilter_NotCurrent);
            if (!coldflametarget)
                coldflametarget = pCreature->GetScript()->getBestPlayerTarget(TargetFilter_Aggroed);

            if (coldflametarget)
            {
                pCreature->GetScript()->SetCreatureData64(DATA_COLDFLAME_GUID, coldflametarget->getGuid());
                effectTargets->push_back(coldflametarget->getGuid());
            }
        }
    }

    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effectIndex)
    {
        if (effectIndex != EFF_INDEX_0)
            return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

        spell->getUnitCaster()->castSpell(spell->GetUnitTarget(), spell->damage, true);

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame Bone Storm
class ColdflameBonestorm : public SpellScript
{
public:
    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effectIndex)
    {
        if (effectIndex != EFF_INDEX_0)
            return SpellScriptExecuteState::EXECUTE_NOT_HANDLED;

        for (uint8 i = 0; i < 4; ++i)
            spell->getUnitCaster()->castSpell(spell->GetUnitTarget(), (spell->damage + i), true);

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame Damage
class ColdflameDamage: public SpellScript
{
public:
    virtual void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets)
    {
        if (effectIndex != EFF_INDEX_0)
            return;

        effectTargets->clear();

        Unit* target = nullptr;
        std::vector<Player*> players;
        for (const auto& itr : spell->getUnitCaster()->getInRangePlayersSet())
        {
            auto target = static_cast<Player*>(itr);

            if(CanBeAppliedOn(target, spell))
                effectTargets->push_back(target->getGuid());
        }
    }

    bool CanBeAppliedOn(Unit* target, Spell* spell)
    {
        if (target->HasAura(SPELL_IMPALED))
            return false;

        if (target->GetDistance2dSq(spell->getUnitCaster()) > static_cast<float>(spell->getSpellInfo()->getEffectRadiusIndex(EFF_INDEX_0)) )
            return false;

        return true;
    }

    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effectIndex)
    {
        if (effectIndex == EFF_INDEX_2)
            return SpellScriptExecuteState::EXECUTE_PREVENT;

        return SpellScriptExecuteState::EXECUTE_OK;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Slice
class BoneSlice : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg) override
    {
        if (effIndex != EFF_INDEX_0 || spell->GetUnitTarget() == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        if (!targetCount)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;
        
        *dmg = float2int32(*dmg / (float)targetCount);
        return SpellScriptEffectDamage::DAMAGE_FULL_RECALCULATION;
    }

    virtual SpellCastResult onCanCast(Spell* spell, uint32_t* parameter1, uint32_t* parameter2)
    {
        targetCount = 0;
        static_cast<Creature*>(spell->getUnitCaster())->GetScript()->DoAction(ACTION_CLEAR_SPIKE_IMMUNITIES);

        return SpellCastResult::SPELL_CAST_SUCCESS;
    }

    virtual void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets)
    {
        if (effectIndex != EFF_INDEX_0)
            return;

        targetCount = effectTargets->size();        
    }

    virtual void afterSpellEffect(Spell* spell, uint8_t effIndex)
    {
        if (effIndex != EFF_INDEX_0)
            return;

        if (spell->GetUnitTarget())
        {
            static_cast<Creature*>(spell->getUnitCaster())->GetScript()->SetCreatureData64(DATA_SPIKE_IMMUNE, spell->GetUnitTarget()->getGuid());
        }
    }

    uint32_t targetCount;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Lady Deathwhisper
class LadyDeathwhisperAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LadyDeathwhisperAI)
        explicit LadyDeathwhisperAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();
       
        dominateMindCount = RAID_MODE<uint8_t>(0, 1, 1, 3);
        introDone = false;

        // Scripted Spells not autocastet
        shadowChannelingSpell           = addAISpell(SPELL_SHADOW_CHANNELING, 0.0f, TARGET_SELF);
        manaBarrierSpell                = addAISpell(SPELL_MANA_BARRIER, 0.0f, TARGET_SELF);
        manaBarrierSpell->mIsTriggered = true;

        deathAndDecaySpell              = addAISpell(SPELL_DEATH_AND_DECAY, 0.0f, TARGET_CUSTOM);
        dominateMindHeroSpell           = addAISpell(SPELL_DOMINATE_MIND_H, 0.0f, TARGET_CUSTOM);
        shadowBoltSpell                 = addAISpell(SPELL_SHADOW_BOLT, 0.0f, TARGET_CUSTOM);
        frostBoltSpell                  = addAISpell(SPELL_FROSTBOLT, 0.0f, TARGET_RANDOM_SINGLE);
        frostBoltVolleySpell            = addAISpell(SPELL_FROSTBOLT_VOLLEY, 0.0f, TARGET_ATTACKING);
        touchOfInsignifcanceSpell       = addAISpell(SPELL_TOUCH_OF_INSIGNIFICANCE, 0.0f, TARGET_ATTACKING);
        summonShadeSpell                = addAISpell(SPELL_SUMMON_SHADE, 0.0f, TARGET_CUSTOM);
        berserkSpell                    = addAISpell(SPELL_BERSERK, 0.0f, TARGET_SELF);

        // Messages
        addEmoteForEvent(Event_OnCombatStart, SAY_LADY_AGGRO);     
        addEmoteForEvent(Event_OnTargetDied, SAY_LADY_DEAD);            
        addEmoteForEvent(Event_OnDied, SAY_LADY_DEATH);            
    }

    void OnLoad()
    {
        if (!_isInCombat() && !getScriptPhase() == PHASE_INTRO)
            return;

        ///\todo Add SpellImmunities
        sendDBChatMessage(SAY_LADY_INTRO_1);
        setScriptPhase(PHASE_INTRO);
        scriptEvents.addEvent(EVENT_INTRO_2, 11000, PHASE_INTRO);
        scriptEvents.addEvent(EVENT_INTRO_3, 21000, PHASE_INTRO);
        scriptEvents.addEvent(EVENT_INTRO_4, 31500, PHASE_INTRO);
        scriptEvents.addEvent(EVENT_INTRO_5, 39500, PHASE_INTRO);
        scriptEvents.addEvent(EVENT_INTRO_6, 48500, PHASE_INTRO);
        scriptEvents.addEvent(EVENT_INTRO_7, 58000, PHASE_INTRO);
        _castAISpell(shadowChannelingSpell);
        introDone = true;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();       
        setScriptPhase(PHASE_ONE);

        // common events
        scriptEvents.addEvent(EVENT_BERSERK, 600000);
        scriptEvents.addEvent(EVENT_DEATH_AND_DECAY, 10000);

        // phase one events
        scriptEvents.addEvent(EVENT_P1_SUMMON_WAVE, 5000, PHASE_ONE);
        scriptEvents.addEvent(EVENT_P1_SHADOW_BOLT, Util::getRandomUInt(5500, 6000), PHASE_ONE);
        scriptEvents.addEvent(EVENT_P1_EMPOWER_CULTIST, Util::getRandomUInt(20000, 30000), PHASE_ONE);

        if (mInstance->GetDifficulty() != MODE_NORMAL_10MEN)
            scriptEvents.addEvent(EVENT_DOMINATE_MIND_H, 27000);

        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
        _setMeleeDisabled(true);
        setRooted(true);

        getCreature()->RemoveAura(SPELL_SHADOW_CHANNELING);
        _castAISpell(manaBarrierSpell);
    }

    void OnCombatStop(Unit* /*_target*/) override
    {
        Reset();
    }

    ///\ todo Health Decreases visualy
    void DamageTaken(Unit* _attacker, uint32* damage) override
    {
        // When Lady Deathwhsiper has her mana Barrier dont deal damage to her instead reduce her mana.
        // phase transition
        if (getScriptPhase() == PHASE_ONE && *damage > uint32(getCreature()->getPower(POWER_TYPE_MANA)))
        {
            sendDBChatMessage(SAY_LADY_PHASE_2);
            sendDBChatMessage(SAY_LADY_PHASE_2_EMOTE);
            setRooted(false);
            *damage -= getCreature()->getPower(POWER_TYPE_MANA);
            getCreature()->setPower(POWER_TYPE_MANA, 0);
            getCreature()->RemoveAura(SPELL_MANA_BARRIER);
            setScriptPhase(PHASE_TWO);
            scriptEvents.addEvent(EVENT_P2_FROSTBOLT, Util::getRandomUInt(10000, 12000), PHASE_TWO);
            scriptEvents.addEvent(EVENT_P2_FROSTBOLT_VOLLEY, Util::getRandomUInt(19000, 21000), PHASE_TWO);
            scriptEvents.addEvent(EVENT_P2_TOUCH_OF_INSIGNIFICANCE, Util::getRandomUInt(6000, 9000), PHASE_TWO);
            scriptEvents.addEvent(EVENT_P2_SUMMON_SHADE, Util::getRandomUInt(12000, 15000), PHASE_TWO);
            // on heroic mode Lady Deathwhisper is immune to taunt effects in phase 2 and continues summoning adds
            if (_isHeroic())
            {
                ///\todo Add SpellImmunities
                scriptEvents.addEvent(EVENT_P2_SUMMON_WAVE, 45000, PHASE_TWO);
            }
        }
        else if (getCreature()->HasAura(SPELL_MANA_BARRIER))
        {
            getCreature()->setPower(POWER_TYPE_MANA,(getCreature()->getPower(POWER_TYPE_MANA) - *damage));
            *damage = 0;
        }
    }

    void Reset()
    {
        getCreature()->setPower(POWER_TYPE_MANA, getCreature()->getMaxPower(POWER_TYPE_MANA));

        ///\todo Add SpellImmunities
        setScriptPhase(PHASE_ONE);
        scriptEvents.resetEvents();

        waveCounter = 0;
        nextVengefulShadeTargetGUID = 0;

        DeleteSummons();

        _castAISpell(shadowChannelingSpell);
        getCreature()->RemoveAllAuras();
    }

    void AIUpdate() override
    {
        if (!_isInCombat() && !getScriptPhase() == PHASE_INTRO)
            return;

        scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

        if (getCreature()->GetAIInterface()->getAiState() == AI_STATE_CASTING)
            return;

        while (uint32_t eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
            case EVENT_INTRO_2:
                sendDBChatMessage(SAY_LADY_INTRO_2);
                break;
            case EVENT_INTRO_3:
                sendDBChatMessage(SAY_LADY_INTRO_3);
                break;
            case EVENT_INTRO_4:
                sendDBChatMessage(SAY_LADY_INTRO_4);
                break;
            case EVENT_INTRO_5:
                sendDBChatMessage(SAY_LADY_INTRO_5);
                break;
            case EVENT_INTRO_6:
                sendDBChatMessage(SAY_LADY_INTRO_6);
                break;
            case EVENT_INTRO_7:
                sendDBChatMessage(SAY_LADY_INTRO_7);
                setScriptPhase(PHASE_ONE);
                break;
            case EVENT_DEATH_AND_DECAY:
                if (Unit* target = getBestPlayerTarget())
                {
                    printf("EVENT_DEATH_AND_DECAY \n");
                    deathAndDecaySpell->setCustomTarget(target);
                    _castAISpell(deathAndDecaySpell);
                }
                scriptEvents.addEvent(EVENT_DEATH_AND_DECAY, Util::getRandomUInt(22000, 30000));
                break;
            case EVENT_DOMINATE_MIND_H:
                printf("EVENT_DOMINATE_MIND_H \n");
                sendDBChatMessage(SAY_LADY_DOMINATE_MIND);
                for (uint8 i = 0; i < dominateMindCount; i++)
                    if (Unit* target = getBestPlayerTarget(TargetFilter_NotCurrent))
                    {
                        dominateMindHeroSpell->setCustomTarget(target);
                        _castAISpell(dominateMindHeroSpell);
                    }
                scriptEvents.addEvent(EVENT_DOMINATE_MIND_H, Util::getRandomUInt(40000, 45000));
                break;
            case EVENT_P1_SUMMON_WAVE:
                printf("EVENT_P1_SUMMON_WAVE \n");
                SummonWavePhaseOne();
                scriptEvents.addEvent(EVENT_P1_SUMMON_WAVE, _isHeroic() ? 45000 : 60000, PHASE_ONE);
                break;
            case EVENT_P1_SHADOW_BOLT:
                if (Unit* target = getBestPlayerTarget())
                {
                    shadowBoltSpell->setCustomTarget(target);
                    _castAISpell(shadowBoltSpell);
                }
                scriptEvents.addEvent(EVENT_P1_SHADOW_BOLT, Util::getRandomUInt(5000, 8000), PHASE_ONE);
                break;
            case EVENT_P1_REANIMATE_CULTIST:
                printf("EVENT_P1_REANIMATE_CULTIST \n");
                ReanimateCultist();
                break;
            case EVENT_P1_EMPOWER_CULTIST:
                printf("EVENT_P1_EMPOWER_CULTIST \n");
                EmpowerCultist();
                scriptEvents.addEvent(EVENT_P1_EMPOWER_CULTIST, Util::getRandomUInt(18000, 25000));
                break;
            case EVENT_P2_FROSTBOLT:
                printf("EVENT_P2_FROSTBOLT \n");
                _castAISpell(frostBoltSpell);
                scriptEvents.addEvent(EVENT_P2_FROSTBOLT, Util::getRandomUInt(10000, 11000), PHASE_TWO);
                break;
            case EVENT_P2_FROSTBOLT_VOLLEY:
                printf("EVENT_P2_FROSTBOLT_VOLLEY \n");
                _castAISpell(frostBoltVolleySpell);
                scriptEvents.addEvent(EVENT_P2_FROSTBOLT_VOLLEY, Util::getRandomUInt(13000, 15000), PHASE_TWO);
                break;
            case EVENT_P2_TOUCH_OF_INSIGNIFICANCE:
                printf("EVENT_P2_TOUCH_OF_INSIGNIFICANCE \n");
                _castAISpell(touchOfInsignifcanceSpell);
                scriptEvents.addEvent(EVENT_P2_TOUCH_OF_INSIGNIFICANCE, Util::getRandomUInt(9000, 13000), PHASE_TWO);
                break;
            case EVENT_P2_SUMMON_SHADE:
                printf("EVENT_P2_SUMMON_SHADE \n");
                if (Unit* shadeTarget = getBestPlayerTarget(TargetFilter_NotCurrent))
                {
                    summonShadeSpell->setCustomTarget(shadeTarget);
                    nextVengefulShadeTargetGUID = shadeTarget->getGuid();
                    _castAISpell(summonShadeSpell);
                }
                scriptEvents.addEvent(EVENT_P2_SUMMON_SHADE, Util::getRandomUInt(18000, 23000), PHASE_TWO);
                break;
            case EVENT_P2_SUMMON_WAVE:
                printf("EVENT_P2_SUMMON_WAVE \n");
                SummonWavePhaseTwo();
                scriptEvents.addEvent(EVENT_P2_SUMMON_WAVE, 45000, PHASE_TWO);
                break;
            case EVENT_BERSERK:
                _castAISpell(berserkSpell);
                sendDBChatMessage(SAY_LADY_BERSERK);
                break;
            }
        }

        // We should not melee attack when barrier is up
        if (getCreature()->HasAura(SPELL_MANA_BARRIER))
        {
            _setMeleeDisabled(true);
            return;
        }

        _setMeleeDisabled(false);
    }

    // summoning function for first phase
    void SummonWavePhaseOne()
    {    
        uint8 addIndex1 = waveCounter & 1;
        uint8 addIndex2 = uint8(addIndex1 ^ 1);

        // Todo summon Darnavan when weekly quest is active
        if (waveCounter)
            Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex1 * 3]);

        Summon(SummonEntries[addIndex2], LadyDeathwhisperSummonPositions[addIndex1 * 3 + 1]);
        Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex1 * 3 + 2]);

        if (mInstance->GetDifficulty() == MODE_NORMAL_25MEN || mInstance->GetDifficulty() == MODE_HEROIC_25MEN)
        {
            Summon(SummonEntries[addIndex2], LadyDeathwhisperSummonPositions[addIndex2 * 3]);
            Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex2 * 3 + 1]);
            Summon(SummonEntries[addIndex2], LadyDeathwhisperSummonPositions[addIndex2 * 3 + 2]);
            Summon(SummonEntries[Util::getRandomUInt(0, 1)], LadyDeathwhisperSummonPositions[6]);
        }
        ++waveCounter;
    }

    // summoning function for second phase
    void SummonWavePhaseTwo()
    {       
        if (mInstance->GetDifficulty() == MODE_NORMAL_25MEN || mInstance->GetDifficulty() == MODE_HEROIC_25MEN)
        {
            uint8 addIndex1 = waveCounter & 1;
            Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex1 * 3]);
            Summon(SummonEntries[addIndex1 ^ 1], LadyDeathwhisperSummonPositions[addIndex1 * 3 + 1]);
            Summon(SummonEntries[addIndex1], LadyDeathwhisperSummonPositions[addIndex1 * 3 + 2]);
        }
        else
            Summon(SummonEntries[Util::getRandomUInt(0, 1)], LadyDeathwhisperSummonPositions[6]);
        ++waveCounter;
    }

    void Summon(uint32 entry, const LocationVector& pos)
    {
        Creature* summon = spawnCreature(entry, pos);

        if (summon)
            summons.push_back(summon);
    }

    void DeleteSummons()
    {
        if (summons.size())
            for (auto itr = summons.begin(); itr != summons.end();)
                (*itr)->Despawn(0, 0);
    }

    void ReanimateCultist()
    {

    }

    void EmpowerCultist()
    {
        
    }

protected:
    // Common
    InstanceScript* mInstance;
    uint64_t nextVengefulShadeTargetGUID;
    std::deque<uint64_t> reanimationQueue;
    uint32_t waveCounter;
    uint8_t dominateMindCount;
    bool introDone;
    std::list<Creature*> summons;

    // Spells
    CreatureAISpells* shadowChannelingSpell;
    CreatureAISpells* manaBarrierSpell;
    CreatureAISpells* deathAndDecaySpell;
    CreatureAISpells* dominateMindHeroSpell;
    CreatureAISpells* shadowBoltSpell;
    CreatureAISpells* frostBoltSpell;
    CreatureAISpells* frostBoltVolleySpell;
    CreatureAISpells* touchOfInsignifcanceSpell;
    CreatureAISpells* summonShadeSpell;
    CreatureAISpells* berserkSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cult Adherent
class CultAdherentAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CultAdherentAI)
        explicit CultAdherentAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        temporalVisualSpell         = addAISpell(SPELL_TELEPORT_VISUAL, 0.0f, TARGET_SELF);
        frostFeverSpell             = addAISpell(SPELL_FROST_FEVER, 100.0f, TARGET_ATTACKING, 0, Util::getRandomUInt(10000, 12000));
        deathchillSpell             = addAISpell(SPELL_DEATHCHILL_BLAST, 100.0f, TARGET_ATTACKING, 0, Util::getRandomUInt(14000, 16000));
        curseOfTorporSpell          = addAISpell(SPELL_CURSE_OF_TORPOR, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(14000, 16000));
        shroudOfTheOccultSpell      = addAISpell(SPELL_SHORUD_OF_THE_OCCULT, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(32000, 39000));
        cultistDarkMartyrdomSpell   = addAISpell(SPELL_DARK_MARTYRDOM_ADHERENT, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(18000, 32000));
    }

    void OnLoad()
    {
        _castAISpell(temporalVisualSpell);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {

    }

    void OnCombatStop(Unit* /*_target*/) override
    {
    
    }

    void AIUpdate() override
    {

    }

protected:
    // Common
    InstanceScript* mInstance;

    // Spells
    CreatureAISpells* temporalVisualSpell;
    CreatureAISpells* frostFeverSpell;
    CreatureAISpells* deathchillSpell;
    CreatureAISpells* curseOfTorporSpell;
    CreatureAISpells* shroudOfTheOccultSpell;
    CreatureAISpells* cultistDarkMartyrdomSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cult Fanatic
class CultFanaticAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CultFanaticAI)
        explicit CultFanaticAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        temporalVisualSpell         = addAISpell(SPELL_TELEPORT_VISUAL, 0.0f, TARGET_SELF);
        necroticStrikeSpell         = addAISpell(SPELL_NECROTIC_STRIKE, 100.0f, TARGET_ATTACKING, 0, Util::getRandomUInt(10000, 12000));
        shadowCleaveSpell           = addAISpell(SPELL_SHADOW_CLEAVE, 100.0f, TARGET_ATTACKING, 0 , Util::getRandomUInt(14000, 16000));
        vampireMightSpell           = addAISpell(SPELL_VAMPIRIC_MIGHT, 100.0f, TARGET_SELF, 0, Util::getRandomUInt(20000, 27000));
        darkMartyrdomSpell          = addAISpell(SPELL_DARK_MARTYRDOM_ADHERENT, 100.0f, TARGET_SELF, 0 , Util::getRandomUInt(18000, 32000));
    }

    void OnLoad()
    {
        _castAISpell(temporalVisualSpell);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {

    }

    void OnCombatStop(Unit* /*_target*/) override
    {
     
    }

    void AIUpdate() override
    {

    }

protected:
    // Common
    InstanceScript* mInstance;

    // Spells
    CreatureAISpells* temporalVisualSpell;
    CreatureAISpells* necroticStrikeSpell;
    CreatureAISpells* shadowCleaveSpell;
    CreatureAISpells* vampireMightSpell;
    CreatureAISpells* darkMartyrdomSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Boss: Valithria Dreamwalker
// ...
//
//
//
//

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Gunship Battle Alliance
class MuradinGossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), 14500);
        menu.addItem(GOSSIP_ICON_CHAT, 446, 1);     // I'll take the flight.
        menu.sendGossipPacket(plr);

    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        IceCrownCitadelScript* pInstance = (IceCrownCitadelScript*)pPlayer->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        if (!pObject->isCreature())
            return;

        Creature* muradin = static_cast<Creature*>(pObject);

        if (muradin->m_transportData.transportGuid)
            printf("On Transport \n");
        else
            printf("Not on Transport \n");

        if (pInstance->AllianceTransporterAllianceShip)
            pInstance->StartFlyShip(pInstance->AllianceTransporterAllianceShip);

        GossipMenu::senGossipComplete(pPlayer);
    }
};

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

    //Bosses
    mgr->register_creature_script(CN_LORD_MARROWGAR, &LordMarrowgarAI::Create);
    mgr->register_creature_script(CN_LADY_DEATHWHISPER, &LadyDeathwhisperAI::Create);
    //mgr->register_creature_script(CN_VALITHRIA_DREAMWALKER, &ValithriaDreamwalkerAI::Create);

    //Spell Bone Storm
    mgr->register_spell_script(boneStormIds, new BoneStormDamage);
    mgr->register_spell_script(SPELL_BONE_STORM, new BoneStorm);

    // Spell Bone Spike Graveyard
    mgr->register_spell_script(SPELL_BONE_SPIKE_GRAVEYARD, new BoneSpikeGraveyard);

    // Spell Coldflame
    mgr->register_spell_script(SPELL_COLDFLAME_NORMAL, new Coldflame);
    mgr->register_spell_script(SPELL_COLDFLAME_DAMAGE, new ColdflameDamage);
    mgr->register_spell_script(SPELL_COLDFLAME_BONE_STORM, new ColdflameBonestorm);

    // Spell Bone Slice
    mgr->register_spell_script(SPELL_BONE_SLICE, new BoneSlice);

    //Gossips
    GossipScript* MuradinGossipScript = new MuradinGossip();
    mgr->register_creature_gossip(NPC_GB_MURADIN_BRONZEBEARD, MuradinGossipScript);

    //Misc
    mgr->register_creature_script(NPC_COLDFLAME, &ColdflameAI::Create);
    mgr->register_creature_script(NPC_BONE_SPIKE, &BoneSpikeAI::Create);

    mgr->register_creature_script(NPC_CULT_FANATIC, CultFanaticAI::Create);
    mgr->register_creature_script(NPC_CULT_ADHERENT, CultAdherentAI::Create);
}
