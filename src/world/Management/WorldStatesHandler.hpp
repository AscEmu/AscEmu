/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#include <map>
#include <memory>
#include <unordered_map>

struct WorldState;
class WorldPacket;

class SERVER_DECL WorldStatesHandler
{
public:
    class SERVER_DECL WorldStatesObserver
    {
    public:
        virtual ~WorldStatesObserver() {}
        virtual void onWorldStateUpdate(uint32_t _zone, uint32_t _field, uint32_t _value) = 0;
    };

    WorldStatesHandler(uint32_t _mapid);
    ~WorldStatesHandler() = default;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Sets the specified worldstate's value for the specified zone
    // \param  uint32_t zone  -  the zone where we set the worldstate
    // \param  uint32_t area  -  the area where we set the worldstate
    // \param  uint32_t field -  the worldstate field we are setting
    // \param  uint32_t value -  the value we assign to the field
    //
    // \return none
    ////////////////////////////////////////////////////////////////////////////////////////////
    void SetWorldStateForZone(uint32_t _zone, uint32_t _area, uint32_t _field, uint32_t _value);

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Returns the value of the worldstate field queried.
    // \param   uint32_t zone  -  the zone where we are querying
    // \param  uint32_t area  -  the area where we querying
    // \param  uint32_t field -  the field that we querying
    //
    // \return the value of the queried field. 0 even if there is no such field.
    ////////////////////////////////////////////////////////////////////////////////////////////
    uint32_t GetWorldStateForZone(uint32_t _zone, uint32_t _area, uint32_t _field) const;

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Builds the initial worldstates packet, that tells the client what worldstates exist
    // \param  uint32_t zone        -  The zone we are building the packet for
    // \param  uint32_t area        -  The area we are building the packet for
    // \param  WorldPacket &data  -  The packet we will fill with the worldstates data
    //
    // \return none
    ////////////////////////////////////////////////////////////////////////////////////////////
    void BuildInitWorldStatesForZone(uint32_t _zone, uint32_t _area, WorldPacket& _data) const;

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Sets up this worldstate handler with the initial data
    // \param  std::multimap< uint32_t, WorldState > *states  -  The source of the initial data
    //
    // \return none
    ////////////////////////////////////////////////////////////////////////////////////////////
    void InitWorldStates(std::multimap<uint32_t, WorldState> const* _states);

    void setObserver(WorldStatesObserver* _observer);

private:
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t>> m_worldStates;
    uint32_t m_map = 0;
    WorldStatesObserver* m_observer = nullptr;
};
