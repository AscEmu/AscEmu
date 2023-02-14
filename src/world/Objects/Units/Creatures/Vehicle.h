/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Transporter.hpp"
#include <array>

#ifdef FT_VEHICLES

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

class Unit;

//////////////////////////////////////////////////////////////////////////////////////////
/// Implements the seat functionality for Vehicles
//////////////////////////////////////////////////////////////////////////////////////////

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
    VehicleSeatAddon(float orientatonOffset, float exitX, float exitY, float exitZ, float exitO, uint8_t param) :
        SeatOrientationOffset(orientatonOffset), ExitParameterX(exitX), ExitParameterY(exitY), ExitParameterZ(exitZ),
        ExitParameterO(exitO), ExitParameter(VehicleExitParameters(param)) { }

    float SeatOrientationOffset = 0.f;
    float ExitParameterX = 0.f;
    float ExitParameterY = 0.f;
    float ExitParameterZ = 0.f;
    float ExitParameterO = 0.f;
    VehicleExitParameters ExitParameter = VehicleExitParameters::None;
};

struct VehicleSeat
{
public:
    explicit VehicleSeat(DBC::Structures::VehicleSeatEntry const* seatInfo, VehicleSeatAddon const* seatAddon) : _seatInfo(seatInfo), _seatAddon(seatAddon)
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

    DBC::Structures::VehicleSeatEntry const* _seatInfo;   // Seat info structure
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


//////////////////////////////////////////////////////////////////////////////////////////
///class Vehicle
/// Implements vehicles in the game
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL Vehicle : public TransportBase
{
protected:
    friend bool Unit::createVehicleKit(uint32_t id, uint32_t creatureEntry);
        Vehicle(Unit* unit, DBC::Structures::VehicleEntry const* vehInfo, uint32_t creatureEntry);

        friend void Unit::removeVehicleKit();
        ~Vehicle();

public:
    void initialize();
    void deactivate();
    void loadAllAccessories(bool evading);
    void loadAccessory(uint32_t entry, int8_t seatId, bool minion, uint8_t type, uint32_t summonTime);

    Unit* getBase() const { return _owner; }
    DBC::Structures::VehicleEntry const* getVehicleInfo() const { return _vehicleInfo; }
    uint32_t getEntry() const { return _creatureEntry; }

    bool hasEmptySeat(int8_t seatId) const;
    bool hasEmptySeat() const;
    Unit* getPassenger(int8_t seatId) const;
    SeatMap::const_iterator getNextEmptySeat(int8_t seatId, bool next) const;
    VehicleSeatAddon const* getSeatAddonForSeatOfPassenger(Unit const* passenger) const;
    uint8_t getAvailableSeatCount() const;

    bool addPassenger(Unit* passenger, int8_t seatId = -1);
    Vehicle* removePassenger(Unit* passenger);
    void relocatePassengers();
    void removeAllPassengers();
    bool isVehicleInUse() const;
    bool isControllableVehicle() const;
    bool isControler(Unit* _unit);

    void setLastShootPos(LocationVector const& pos) { _lastShootPos.ChangeCoords(pos); }
    LocationVector const& getLastShootPos() const { return _lastShootPos; }

    SeatMap Seats;

    DBC::Structures::VehicleSeatEntry const* getSeatForPassenger(Unit const* passenger) const;
    int8_t getSeatForNumberPassenger(Unit const* passenger) const;

    bool hasVehicleFlags(uint32_t flags) { return getVehicleInfo()->flags & flags; }

protected:
    uint32_t usableSeatNum;

private:
    SeatMap::iterator getSeatIteratorForPassenger(Unit* passenger);
    void initSeats();
    void initMovementFlags();
    void initVehiclePowerTypes();
    void applyAllImmunities();

    bool tryAddPassenger(Unit* passenger, SeatMap::iterator& Seat);

        /// This method transforms supplied transport offsets into global coordinates
        void CalculatePassengerPosition(float& x, float& y, float& z, float* o /*= nullptr*/) const override
        {
            TransportBase::CalculatePassengerPosition(x, y, z, o,
                getBase()->GetPositionX(), getBase()->GetPositionY(),
                getBase()->GetPositionZ(), getBase()->GetOrientation());
        }

        /// This method transforms supplied global coordinates into local offsets
        void CalculatePassengerOffset(float& x, float& y, float& z, float* o /*= nullptr*/) const override
        {
            TransportBase::CalculatePassengerOffset(x, y, z, o,
                getBase()->GetPositionX(), getBase()->GetPositionY(),
                getBase()->GetPositionZ(), getBase()->GetOrientation());
        }

        Unit* _owner;
        DBC::Structures::VehicleEntry const* _vehicleInfo;
        std::set<WoWGuid> _vehiclePlayers;

        uint32_t _creatureEntry;
        VehicleStatus _status;
        LocationVector _lastShootPos;
};
#endif
