/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Management/Battleground/Battleground.h"
#include "Units/Creatures/Vehicle.h"

const uint32 IOC_NUM_CONTROL_POINTS = 7;
const uint32 IOC_NUM_TELEPORTERS = 12;
const uint32 IOC_NUM_DOCKVEHICLES = 4;
const uint32 IOC_NUM_WORKSHOP_DEMOLISHERS = 4;
const uint32 IOC_NUM_GATES_PER_TEAM = 3;
const uint32 IOC_NUM_GRAVEYARDS = 7;

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
    uint32 owner;
    Creature* spiritguide;

    IOCGraveyard()
    {
        owner = MAX_PLAYER_TEAMS;
        spiritguide = NULL;
    }

    ~IOCGraveyard()
    {
        owner = MAX_PLAYER_TEAMS;
        spiritguide = NULL;
    }
};

struct IOCTeleporter
{
    GameObject* teleporter;
    GameObject* effect;

    IOCTeleporter()
    {
        teleporter = NULL;
        effect = NULL;
    }

    ~IOCTeleporter()
    {
        teleporter = NULL;
        effect = NULL;
    }
};

struct IOCControlPoint
{
    GameObject* pole;
    GameObject* banner;
    GameObject* aura;
    ControlPointTypes state;
    uint32 worldstate;

    IOCControlPoint()
    {
        pole = NULL;
        banner = NULL;
        aura = NULL;
        state = IOC_SPAWN_TYPE_NEUTRAL;
        worldstate = 0;
    }

    ~IOCControlPoint()
    {
        pole = NULL;
        banner = NULL;
        aura = NULL;
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
        gate = NULL;
        dyngate = NULL;
    }

    ~IOCGate()
    {
        gate = NULL;
        dyngate = NULL;
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

        if (creature->GetVehicleComponent() == nullptr)
        {
            return true;
        }

        if (creature->GetVehicleComponent()->GetPassengerCount() > 0)
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

        IsleOfConquest(MapMgr* mgr, uint32 id, uint32 lgroup, uint32 t);
        ~IsleOfConquest();

        static CBattleground* Create(MapMgr* m, uint32 i, uint32 l, uint32 t) { return new IsleOfConquest(m, i, l, t); }

        void Init();
        void OnCreate();
        void OnStart();
        void OpenGates();
        void CloseGates();
        void SpawnControlPoint(uint32 Id, uint32 Type);
        void SpawnGraveyard(uint32 id, uint32 team);
        void Finish(uint32 losingTeam);
        bool HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam) override;
        void HookOnAreaTrigger(Player* plr, uint32 id);
        void HookOnPlayerDeath(Player* plr);
        void HookOnPlayerResurrect(Player* player);
        void HookOnPlayerKill(Player* /*plr*/, Player* /*pVictim*/) override {}
        void HookFlagDrop(Player* /*plr*/, GameObject* /*obj*/) override {}
        void HookOnFlagDrop(Player* /*plr*/) override {}
        void HookFlagStand(Player* /*plr*/, GameObject* /*obj*/) override {}
        bool HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell *pSpell);
        void HookOnMount(Player* /*plr*/)  override {}
        void HookGenerateLoot(Player* /*plr*/, Object* /*pCorpse*/) override {}
        void OnAddPlayer(Player* plr);
        void OnRemovePlayer(Player* plr) override;
        void HookOnShadowSight();
        void SetIsWeekend(bool isweekend);
        void HookOnUnitKill(Player* plr, Unit* pVictim);
        void HookOnUnitDied(Unit* victim);
        LocationVector GetStartingCoords(uint32 Team);
        void AddReinforcements(uint32 team, uint32 amount);
        void RemoveReinforcements(uint32 team, uint32 amount);
        void UpdateResources();
        void HookOnHK(Player* plr);
        void AssaultControlPoint(Player* player, uint32 id);
        void CaptureControlPoint(uint32 id);
        bool HookHandleRepop(Player* plr);
        void BuildWorkshopVehicle(uint32 delay);

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
        uint32 m_reinforcements[MAX_PLAYER_TEAMS];
};
