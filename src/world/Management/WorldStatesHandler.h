/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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

#ifndef WORLDSTATEHANDLER_H
#define WORLDSTATEHANDLER_H

#include "WorldPacket.h"
#include <unordered_map>

struct WorldState;

class SERVER_DECL WorldStatesHandler
{
    public:

        class SERVER_DECL WorldStatesObserver
        {
            public:
                virtual ~WorldStatesObserver(){}
                virtual void onWorldStateUpdate(uint32_t zone, uint32_t field, uint32_t value) = 0;
        };

        WorldStatesHandler(uint32_t mapid)
        {
            map = mapid;
            observer = NULL;
        }

        ~WorldStatesHandler(){}


        //////////////////////////////////////////////////////////////////////////////////////////
        /// Sets the specified worldstate's value for the specified zone
        /// \param  uint32_t zone  -  the zone where we set the worldstate
        /// \param  uint32_t area  -  the area where we set the worldstate
        /// \param  uint32_t field -  the worldstate field we are setting
        /// \param  uint32_t value -  the value we assign to the field
        ///
        /// \return none
        ////////////////////////////////////////////////////////////////////////////////////////////
        void SetWorldStateForZone(uint32_t zone, uint32_t area, uint32_t field, uint32_t value);


        ////////////////////////////////////////////////////////////////////////////////////////////
        /// Returns the value of the worldstate field queried.
        /// \param   uint32_t zone  -  the zone where we are querying
        /// \param  uint32_t area  -  the area where we querying
        /// \param  uint32_t field -  the field that we querying
        ///
        /// \return the value of the queried field. 0 even if there is no such field.
        ////////////////////////////////////////////////////////////////////////////////////////////
        uint32_t GetWorldStateForZone(uint32_t zone, uint32_t area, uint32_t field) const;


        ////////////////////////////////////////////////////////////////////////////////////////////
        /// Builds the initial worldstates packet, that tells the client what worldstates exist
        /// \param  uint32_t zone        -  The zone we are building the packet for
        /// \param  uint32_t area        -  The area we are building the packet for
        /// \param  WorldPacket &data  -  The packet we will fill with the worldstates data
        ///
        /// \return none
        ////////////////////////////////////////////////////////////////////////////////////////////
        void BuildInitWorldStatesForZone(uint32_t zone, uint32_t area, WorldPacket &data) const;


        ////////////////////////////////////////////////////////////////////////////////////////////
        /// Sets up this worldstate handler with the initial data
        /// \param  std::multimap< uint32_t, WorldState > *states  -  The source of the initial data
        ///
        /// \return none
        ////////////////////////////////////////////////////////////////////////////////////////////
        void InitWorldStates(std::multimap<uint32_t, WorldState> *states);

        void setObserver(WorldStatesObserver* pObserver){ this->observer = pObserver; }

    private:

        std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t>> worldstates;
        uint32_t map;
        WorldStatesObserver* observer;
};

#endif      //WORLDSTATEHANDLER_H
