/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LocationVector.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <cstdint>

enum VehicleStatus
{
    STATUS_NONE                     = 0,
    STATUS_INITALIZED               = 1,
    STATUS_DEACTIVATED              = 2
};

enum VehicleFlags
{
    VEHICLE_FLAG_NO_STRAFING        = 0x00000001,           // Sets MOVEFLAG2_NO_STRAFING
    VEHICLE_FLAG_NO_JUMPING         = 0x00000002,           // Sets MOVEFLAG2_NO_JUMPING
    VEHICLE_FLAG_FULLSPEED_TURNING  = 0x00000004,           // Sets MOVEFLAG2_FULLSPEED_TURNING
    VEHICLE_FLAG_ALLOW_PITCHING     = 0x00000010,           // Sets MOVEFLAG2_ALLOW_PITCHING
    VEHICLE_FLAG_FULLSPEED_PITCHING = 0x00000020,           // Sets MOVEFLAG2_FULLSPEED_PITCHING
    VEHICLE_FLAG_CUSTOM_PITCHING    = 0x00000040,           // If set use pitchMin and pitchMax from DBC, otherwise pitchMin = -pi/2, pitchMax = pi/2
    VEHICLE_FLAG_ADJUST_AIM_ANGLE   = 0x00000400,           // Lua_IsVehicleAimAngleAdjustable
    VEHICLE_FLAG_ADJUST_AIM_POWER   = 0x00000800,           // Lua_IsVehicleAimPowerAdjustable
    VEHICLE_FLAG_POSITION_FIXED     = 0x00200000            // Used for cannons, when they should be rooted
};

enum VehicleSpells
{
    VEHICLE_SPELL_RIDE_HARDCODED    = 46598,
    VEHICLE_SPELL_PARACHUTE         = 45472
};

enum class VehicleExitParameters
{
    None                            = 0, // ignore all Parameters
    Offset                          = 1, // use Offset values from Parameters
    Destination                     = 2, // use absolute destination from Parameters
    Maximum
};

struct PassengerInfo
{
    uint64_t guid;
    bool isUnselectable;

    void reset()
    {
        guid = 0;
        isUnselectable = false;
    }
};

struct VehicleSeatAddon
{
    VehicleSeatAddon() = default;
    VehicleSeatAddon(float orientatonOffset, LocationVector exitLv, uint8_t param) :
        seatOrientationOffset(orientatonOffset), exitLocation(exitLv), exitParameter(VehicleExitParameters(param)) { }

    float seatOrientationOffset = 0.f;
    LocationVector exitLocation = { 0.f, 0.f, 0.f, 0.f };
    VehicleExitParameters exitParameter = VehicleExitParameters::None;
};

namespace WDB::Structures
{
    struct VehicleSeatEntry;
}

struct VehicleSeat
{
public:
    explicit VehicleSeat(WDB::Structures::VehicleSeatEntry const* seatInfo, VehicleSeatAddon const* seatAddon) : _seatInfo(seatInfo), _seatAddon(seatAddon)
    {
        _passenger.reset();
    }

    bool isEmpty() const 
    { 
        if (_passenger.guid)
            return false;
        else
            return true;
    }

    WDB::Structures::VehicleSeatEntry const* _seatInfo;
    VehicleSeatAddon const* _seatAddon;
    PassengerInfo _passenger;
};

struct VehicleAccessory
{
    VehicleAccessory(uint32_t entry, int8_t seatId, bool isMinion, uint8_t summonType, uint32_t summonTime) :
        accessoryEntry(entry), isMinion(isMinion), summonTime(summonTime), seatId(seatId), summonedType(summonType) { }
    uint32_t accessoryEntry;
    bool isMinion;
    uint32_t summonTime;
    int8_t seatId;
    uint8_t summonedType;
};

typedef std::vector<VehicleAccessory> VehicleAccessoryList;
typedef std::unordered_map<uint32_t, VehicleSeatAddon> VehicleSeatAddonContainer;
typedef std::map<uint32_t, VehicleAccessoryList> VehicleAccessoryContainer;
typedef std::map<int8_t, VehicleSeat> SeatMap;
