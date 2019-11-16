/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

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
                return true;
        }

        return false;
    }


    bool IsEmpty() const
    {
        if (creature == nullptr)
            return true;

        if (creature->getVehicleComponent() == nullptr)
            return true;

        if (creature->getVehicleComponent()->GetPassengerCount() > 0)
            return false;

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
