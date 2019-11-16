/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Battleground/Battleground.h"
#include "Units/Creatures/Vehicle.h"

enum
{
    IOC_NUM_CONTROL_POINTS = 7,
    IOC_NUM_TELEPORTERS = 12,
    IOC_NUM_DOCKVEHICLES = 4,
    IOC_NUM_WORKSHOP_DEMOLISHERS = 4,
    IOC_NUM_GATES_PER_TEAM = 3,
    IOC_NUM_GRAVEYARDS = 7,

    IOC_NUM_REINFORCEMENTS = 300,
    IOC_POINTS_ON_KILL = 1,
};

enum IOCGOs
{
    IOC_FLAGPOLE = 195131,
    IOC_TELEPORTER_H_IN = 195313,
    IOC_TELEPORTER_H_OUT = 195314,
    IOC_TELEPORTER_A_OUT = 195315,
    IOC_TELEPORTER_A_IN = 195316,
    TELEPORTER_EFFECT_A = 195701,
    TELEPORTER_EFFECT_H = 195702,
    IOC_DYNAMIC_GATE_HORDE = 195491,
    IOC_DYNAMIC_GATE_ALLY = 195703
};

enum IOCSpells
{
    IOC_REFINERY_BONUS = 68719,
    IOC_QUARRY_BONUS = 68720
};

enum ControlPoints
{
    IOC_CONTROL_POINT_REFINERY = 0,
    IOC_CONTROL_POINT_QUARRY = 1,
    IOC_CONTROL_POINT_DOCKS = 2,
    IOC_CONTROL_POINT_HANGAR = 3,
    IOC_CONTROL_POINT_WORKSHOP = 4,
    IOC_CONTROL_POINT_ALLIANCE_KEEP = 5,
    IOC_CONTROL_POINT_HORDE_KEEP = 6
};

enum GraveYards
{
    IOC_GY_DOCKS = 0,
    IOC_GY_HANGAR = 1,
    IOC_GY_WORKSHOP = 2,
    IOC_GY_ALLIANCE_KEEP = 3,
    IOC_GY_HORDE_KEEP = 4,
    IOC_GY_ALLIANCE = 5,
    IOC_GY_HORDE = 6,
    IOC_GY_NONE = 7
};

enum ControlPointTypes
{
    IOC_SPAWN_TYPE_NEUTRAL = 0,
    IOC_SPAWN_TYPE_ALLIANCE_ASSAULT = 1,
    IOC_SPAWN_TYPE_HORDE_ASSAULT = 2,
    IOC_SPAWN_TYPE_ALLIANCE_CONTROLLED = 3,
    IOC_SPAWN_TYPE_HORDE_CONTROLLED = 4
};

struct IOCGraveyard
{
    uint32_t owner;
    Creature* spiritguide;

    IOCGraveyard()
    {
        owner = MAX_PLAYER_TEAMS;
        spiritguide = nullptr;
    }

    ~IOCGraveyard()
    {
        owner = MAX_PLAYER_TEAMS;
        spiritguide = nullptr;
    }
};

struct IOCTeleporter
{
    GameObject* teleporter;
    GameObject* effect;

    IOCTeleporter()
    {
        teleporter = nullptr;
        effect = nullptr;
    }

    ~IOCTeleporter()
    {
        teleporter = nullptr;
        effect = nullptr;
    }
};

struct IOCControlPoint
{
    GameObject* pole;
    GameObject* banner;
    GameObject* aura;
    ControlPointTypes state;
    uint32_t worldstate;

    IOCControlPoint()
    {
        pole = nullptr;
        banner = nullptr;
        aura = nullptr;
        state = IOC_SPAWN_TYPE_NEUTRAL;
        worldstate = 0;
    }

    ~IOCControlPoint()
    {
        pole = nullptr;
        banner = nullptr;
        aura = nullptr;
        state = IOC_SPAWN_TYPE_NEUTRAL;
        worldstate = 0;
    }
};

struct IOCGate
{
    GameObject* gate;
    GameObject* dyngate;

    IOCGate()
    {
        gate = nullptr;
        dyngate = nullptr;
    }

    ~IOCGate()
    {
        gate = nullptr;
        dyngate = nullptr;
    }
};

struct IOCVehicle
{
    Creature* creature;
    LocationVector baselocation;

    IOCVehicle()
    {
        creature = nullptr;
    }

    ~IOCVehicle()
    {
        creature = nullptr;
    }

    bool IsCloseToBaseLocation()
    {
        if (creature != nullptr)
        {
            if (creature->CalcDistance(baselocation) <= 10.0f)
            {
                return true;
            }
        }

        return false;
    }


    bool IsEmpty() const
    {
        if (creature == nullptr)
        {
            return true;
        }

        if (creature->getVehicleComponent() == nullptr)
        {
            return true;
        }

        if (creature->getVehicleComponent()->GetPassengerCount() > 0)
        {
            return false;
        }

        return true;
    }

    void Despawn()
    {
        if (creature != nullptr)
        {
            creature->Despawn(0, 0);
            creature = nullptr;
        }
    }
};

class IsleOfConquest : public CBattleground
{
public:

    IsleOfConquest(MapMgr* mgr, uint32_t id, uint32_t lgroup, uint32_t t);
    ~IsleOfConquest();

    static CBattleground* Create(MapMgr* m, uint32_t i, uint32_t l, uint32_t t) { return new IsleOfConquest(m, i, l, t); }

    void Init();
    void OnCreate() override;
    void OnStart() override;
    void OpenGates();
    void CloseGates();
    void SpawnControlPoint(uint32_t Id, uint32_t Type);
    void SpawnGraveyard(uint32_t id, uint32_t team);
    void Finish(uint32_t losingTeam);
    bool HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam) override;
    void HookOnAreaTrigger(Player* plr, uint32_t id) override;
    void HookOnPlayerDeath(Player* plr) override;
    void HookOnPlayerResurrect(Player* player) override;
    void HookOnPlayerKill(Player* /*plr*/, Player* /*pVictim*/) override {}
    void HookFlagDrop(Player* /*plr*/, GameObject* /*obj*/) override {}
    void HookOnFlagDrop(Player* /*plr*/) override {}
    void HookFlagStand(Player* /*plr*/, GameObject* /*obj*/) override {}
    bool HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell *pSpell) override;
    void HookOnMount(Player* /*plr*/)  override {}
    void HookGenerateLoot(Player* /*plr*/, Object* /*pCorpse*/) override {}
    void OnAddPlayer(Player* plr) override;
    void OnRemovePlayer(Player* plr) override;
    void HookOnShadowSight() override;
    void SetIsWeekend(bool isweekend) override;
    void HookOnUnitKill(Player* plr, Unit* pVictim) override;
    void HookOnUnitDied(Unit* victim) override;
    LocationVector GetStartingCoords(uint32_t Team) override;
    void AddReinforcements(uint32_t team, uint32_t amount);
    void RemoveReinforcements(uint32_t team, uint32_t amount);
    void UpdateResources();
    void HookOnHK(Player* plr) override;
    void AssaultControlPoint(Player* player, uint32_t id);
    void CaptureControlPoint(uint32_t id);
    bool HookHandleRepop(Player* plr) override;
    void BuildWorkshopVehicle(uint32_t delay);

    // Capture events
    void EventRefineryCaptured();
    void EventQuarryCaptured();
    void EventDocksCaptured();
    void EventHangarCaptured();
    void EventWorkshopCaptured();
    void EventAllianceKeepCaptured();
    void EventHordeKeepCaptured();

private:

    IOCTeleporter teleporter[IOC_NUM_TELEPORTERS];
    IOCControlPoint controlpoint[IOC_NUM_CONTROL_POINTS];
    IOCGate gates[MAX_PLAYER_TEAMS][IOC_NUM_GATES_PER_TEAM];
    IOCVehicle workshopvehicle[MAX_PLAYER_TEAMS];
    IOCVehicle workshopdemolisher[MAX_PLAYER_TEAMS][IOC_NUM_WORKSHOP_DEMOLISHERS];
    IOCVehicle dockvehicle[MAX_PLAYER_TEAMS][IOC_NUM_DOCKVEHICLES];
    IOCGraveyard graveyards[IOC_NUM_GRAVEYARDS];
    GameObject* towergates[MAX_PLAYER_TEAMS][2];
    Unit* generals[MAX_PLAYER_TEAMS];
    uint32_t m_reinforcements[MAX_PLAYER_TEAMS];
};
