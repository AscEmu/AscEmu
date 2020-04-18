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
 *
 */

#include "StdAfx.h"
#include "WorldStatesHandler.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Objects/ObjectMgr.h"

void WorldStatesHandler::SetWorldStateForZone(uint32_t zone, uint32_t /*area*/, uint32_t field, uint32_t value)
{
    std::unordered_map< uint32_t, std::unordered_map< uint32_t, uint32_t > >::iterator itr = worldstates.find(zone);
    if (itr == worldstates.end())
        return;

    std::unordered_map< uint32_t, uint32_t >::iterator itr2 = itr->second.find(field);
    if (itr2 == itr->second.end())
        return;

    itr2->second = value;

    if (observer != NULL)
        observer->onWorldStateUpdate(zone, field, value);
}

uint32_t WorldStatesHandler::GetWorldStateForZone(uint32_t zone, uint32_t /*area*/, uint32_t field) const
{
    std::unordered_map< uint32_t, std::unordered_map< uint32_t, uint32_t > >::const_iterator itr = worldstates.find(zone);
    if (itr == worldstates.end())
        return 0;

    std::unordered_map< uint32_t, uint32_t >::const_iterator itr2 = itr->second.find(field);
    if (itr2 == itr->second.end())
        return 0;

    return itr2->second;
}

void WorldStatesHandler::BuildInitWorldStatesForZone(uint32_t zone, uint32_t area, WorldPacket &data) const
{
    data << uint32_t(map);
    data << uint32_t(zone);
    data << uint32_t(area);

    std::unordered_map< uint32_t, std::unordered_map< uint32_t, uint32_t > >::const_iterator itr = worldstates.find(zone);

    if (itr != worldstates.end())
    {
        data << uint16_t(2 + itr->second.size());

        for (std::unordered_map< uint32_t, uint32_t >::const_iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); ++itr2)
        {
            data << uint32_t(itr2->first);
            data << uint32_t(itr2->second);
        }

    }
    else
    {
        data << uint16_t(2);
    }

#if VERSION_STRING > TBC
    data << uint32_t(3191);
    data << uint32_t(worldConfig.arena.arenaSeason);
    data << uint32_t(3901);
    data << uint32_t(worldConfig.arena.arenaProgress);
#endif
}

void WorldStatesHandler::InitWorldStates(std::multimap< uint32_t, WorldState > *states)
{
    if (states == NULL)
        return;

    for (std::multimap< uint32_t, WorldState >::iterator itr = states->begin(); itr != states->end(); ++itr)
    {
        uint32_t zone = itr->first;
        worldstates[zone];
        worldstates[zone].insert(std::pair< uint32_t, uint32_t >(itr->second.field, itr->second.value));
    }
}
