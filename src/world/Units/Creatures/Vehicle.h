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


#ifndef VEHICLE_H
#define VEHICLE_H

#include "Storage/DBC/DBCStores.h"
#include "Units/Summons/SummonHandler.h"
#include <array>

///\todo vehicle movementflags? Why didn't we use the normal movementflags and handle vehicles like normal units/creatures?
enum VehicleFlags
{
    VEHICLE_FLAG_NO_STRAFE                       = 0x00000001,           // Sets MOVEFLAG2_NO_STRAFE
    VEHICLE_FLAG_NO_JUMPING                      = 0x00000002,           // Sets MOVEFLAG2_NO_JUMPING
    VEHICLE_FLAG_FULLSPEEDTURNING                = 0x00000004,           // Sets MOVEFLAG2_FULLSPEEDTURNING
    VEHICLE_FLAG_ALLOW_PITCHING                  = 0x00000010,           // Sets MOVEFLAG2_ALLOW_PITCHING
    VEHICLE_FLAG_FULLSPEEDPITCHING               = 0x00000020,           // Sets MOVEFLAG2_FULLSPEEDPITCHING
    VEHICLE_FLAG_CUSTOM_PITCH                    = 0x00000040,           // If set use pitchMin and pitchMax from DBC, otherwise pitchMin = -pi/2, pitchMax = pi/2
    VEHICLE_FLAG_ADJUST_AIM_ANGLE                = 0x00000400,           // Lua_IsVehicleAimAngleAdjustable
    VEHICLE_FLAG_ADJUST_AIM_POWER                = 0x00000800,           // Lua_IsVehicleAimPowerAdjustable
};


//////////////////////////////////////////////////////////////////////////////////////////
/// Implements the seat functionality for Vehicles
//////////////////////////////////////////////////////////////////////////////////////////
class VehicleSeat
{
    public:

    VehicleSeat(DBC::Structures::VehicleSeatEntry const* info);


        //////////////////////////////////////////////////////////////////////////////////////////
        /// Tells if the seat is occupied or not.
        /// \param none
        /// \return true if the seat is occupied, false if the seat is empty.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool HasPassenger() const
        {
            if (passenger == 0)
                return false;
            else
                return true;
        }


        //////////////////////////////////////////////////////////////////////////////////////////
        /// Adds a passenger to the seat
        /// \param uint64 passenger_guid  -  GUID of the passenger
        /// \return true on success, false on failure.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool AddPassenger(uint64 passenger_guid)
        {
            if (HasPassenger())
                return false;
            else
                passenger = passenger_guid;

            return true;
        }


        //////////////////////////////////////////////////////////////////////////////////////////
        /// Returns the GUID of the passenger in the seat
        /// \param none
        /// \return the GUID of the passenger in the seat, if any
        /// \return 0 otherwise
        //////////////////////////////////////////////////////////////////////////////////////////
        uint64 GetPassengerGUID() const
        {
            return passenger;
        }


        //////////////////////////////////////////////////////////////////////////////////////////
        /// Removes the passenger from the seat
        /// \param none
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void RemovePassenger()
        {
            passenger = 0;
        }


        //////////////////////////////////////////////////////////////////////////////////////////
        /// Retrieves the seat information structure
        /// \param none
        /// \return a pointer to a VehicleSeatEntry structure.
        //////////////////////////////////////////////////////////////////////////////////////////
        DBC::Structures::VehicleSeatEntry const* GetSeatInfo() const
        {
            return seat_info;
        }


        //////////////////////////////////////////////////////////////////////////////////////////
        /// Tells if the seat is usable for passengers
        /// \param none
        /// \ return true if the seat is usable, false otherwise.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool Usable() const;


        //////////////////////////////////////////////////////////////////////////////////////////
        /// Tells if the seat can control the vehicle
        /// \param none
        /// \return true if the seat can control, false otherwise.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool Controller() const;


        //////////////////////////////////////////////////////////////////////////////////////////
        /// Tells if the seat hides the passenger
        /// \param none
        /// \return true if the passenger is hidden in this seat, false otherwise
        //////////////////////////////////////////////////////////////////////////////////////////
        bool HidesPassenger() const;

    private:

        uint64 passenger;              // GUID of the passenger
        DBC::Structures::VehicleSeatEntry const* seat_info;   // Seat info structure
};


//////////////////////////////////////////////////////////////////////////////////////////
///class Vehicle
/// Implements vehicles in the game
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL Vehicle
{
    public:

        Vehicle();
        ~Vehicle();

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Loads and sets up the Vehicle
        /// \param Unit* owner           -  Pointer to the Unit this vehicle belongs to
        /// \param uint32 creature_entry -  Host creature of the Vehicle
        /// \param uint32 vehicleid      -  Index of Vehicle.dbc
        ///
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void Load(Unit* owner, uint32 creature_entry, uint32 vehicleid);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Tells if the Vehicle has at least 1 empty seat.
        /// \param none
        /// \return true if there's at least 1 empty seat, false otherwise.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool HasEmptySeat();

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Adds a passenger to the vehicle
        /// \param Unit* passenger  -  Passenger to add
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void AddPassenger(Unit* passenger);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Adds the passenger to the specified seat if possible
        /// \param Unit* passenger  -  Pointer to the passenger we want to add
        /// \param uint32 seatid    -  The id of the seat we want the passenger to be added
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void AddPassengerToSeat(Unit* passenger, uint32 seatid);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Remove the passenger from the vehicle
        /// \param Unit* passenger  -  Passenger to remove.
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void EjectPassenger(Unit* passenger);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Ejects the passenger from this seat.
        /// \param uint32 seatid  -  Identifier of the seat we want to evict
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void EjectPassengerFromSeat(uint32 seatid);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Ejects all passengers from the vehicle
        /// \param none
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void EjectAllPassengers();

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Moves the passenger to the specified seat
        /// \param Unit* passenger  -  The passenger we want to move
        /// \param uint32 seat      -  The seat where we want to move this passenger to
        ///
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void MovePassengerToSeat(Unit* passenger, uint32 seat);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Moves the specified passenger to the next seat
        /// \param Unit* passenger  -  Pointer to the passnger we want to move
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void MovePassengerToNextSeat(Unit* passenger);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Moves the specified passenger to the previous seat
        /// \param Unit* passenger  -  Pointer to the passenger we want to move
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void MovePassengerToPrevSeat(Unit* passenger);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Retrieves the seat entry ID for this passenger
        /// \param Unit* passenger  -  Pointer to the passenger
        /// \return the entry ID of the seat this passenger occupies
        //////////////////////////////////////////////////////////////////////////////////////////
        uint32 GetSeatEntryForPassenger(Unit* passenger);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// boolean controller aura
        /// \param bool
        /// \return true if unit has aura.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool IsControler(Unit* aura);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Moves the passengers on/in the vehicle to the specified coordinates
        /// \param float x  -  destination X coordinate
        /// \param float y  -  destination Y coordinate
        /// \param float z  -  destination Z coordinate (height)
        /// \param float o  -  destination orientation
        ///
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void MovePassengers(float x, float y, float z, float o);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Tells the number of passengers currently in the vehicle
        /// \param none
        /// \return the number of passengers.
        //////////////////////////////////////////////////////////////////////////////////////////
        uint32 GetPassengerCount() const;

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Retrieves a pointer to the owner of the vehicle.
        /// The owner is the unit which has this vehicle component.
        /// \param none
        /// \return a pointer to the unit which owns this vehicle component.
        //////////////////////////////////////////////////////////////////////////////////////////
        Unit* GetOwner() const{ return owner; }

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Retrieves the Move2 flags for this Vehicle
        /// \param none
        /// \return the extra movement flags for this vehicle
        //////////////////////////////////////////////////////////////////////////////////////////
        uint16 GetMoveFlags2() const;

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Installs all accessories for this vehicle (turrets for example)
        /// \param none
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void InstallAccessories();

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Removes all installed vehicle accessories
        /// \param none
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
        void RemoveAccessories();

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Tells if this vehicle has the specified accessory attached to it
        /// \param uint64 guid  -  GUID of the accessory
        /// \return true if the vehicle has the accessory attached, false otherwise.
        //////////////////////////////////////////////////////////////////////////////////////////
        bool HasAccessoryWithGUID(uint64 guid);

        DBC::Structures::VehicleEntry const* GetVehicleInfo() { return vehicle_info; }
        uint32 GetPassengerSeatId(uint64 guid);

    private:

        std::array<VehicleSeat*, MAX_VEHICLE_SEATS> seats;
        std::vector<uint64> installed_accessories;

        uint32 creature_entry;
        Unit* owner;

        DBC::Structures::VehicleEntry const* vehicle_info;

        uint32 passengercount;
        uint32 freeseats;
};

struct VehicleAccessoryEntry
{
    uint32 accessory_entry;
    uint32 seat;
};

#endif      // _VEHICLE_H
