/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "VehicleDefines.hpp"
#include "Objects/TransportBase.hpp"

#include <set>

#ifdef FT_VEHICLES

class WoWGuid;
class Unit;

namespace WDB::Structures
{
    struct VehicleEntry;
    struct VehicleSeatEntry;
}

class SERVER_DECL Vehicle : public TransportBase
{
public:
    Vehicle(Unit* unit, WDB::Structures::VehicleEntry const* vehInfo, uint32_t creatureEntry);
    ~Vehicle();

    void initialize();
    void deactivate();
    void loadAllAccessories(bool evading);
    void loadAccessory(uint32_t entry, int8_t seatId, bool minion, uint8_t type, uint32_t summonTime);

    Unit* getBase() const;
    WDB::Structures::VehicleEntry const* getVehicleInfo() const;
    uint32_t getEntry() const;

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

    void setLastShootPos(LocationVector const& pos);
    LocationVector const& getLastShootPos() const;

    SeatMap Seats;

    WDB::Structures::VehicleSeatEntry const* getSeatForPassenger(Unit const* passenger) const;
    int8_t getSeatForNumberPassenger(Unit const* passenger) const;

    bool hasVehicleFlags(uint32_t flags);

protected:
    uint32_t usableSeatNum;

private:
    SeatMap::iterator getSeatIteratorForPassenger(Unit* passenger);
    void initSeats();
    void initMovementFlags();
    void initVehiclePowerTypes();
    void applyAllImmunities();

    bool tryAddPassenger(Unit* passenger, SeatMap::iterator& Seat);

    void calculatePassengerPosition(float& x, float& y, float& z, float* o) override;

    void calculatePassengerOffset(float& x, float& y, float& z, float* o) override;

    Unit* _owner;
    WDB::Structures::VehicleEntry const* _vehicleInfo;
    std::set<WoWGuid> _vehiclePlayers;

    uint32_t _creatureEntry;
    VehicleStatus _status;
    LocationVector _lastShootPos;
};
#endif
