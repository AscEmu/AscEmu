/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WorldStatesHandler.hpp"

#include "ObjectMgr.hpp"
#include "WorldPacket.h"
#include "Server/World.h"

WorldStatesHandler::WorldStatesHandler(uint32_t _mapid) : m_map(_mapid) { }

void WorldStatesHandler::SetWorldStateForZone(uint32_t _zone, uint32_t /*area*/, uint32_t _field, uint32_t _value)
{
    const auto itr = m_worldStates.find(_zone);
    if (itr == m_worldStates.end())
        return;

    const auto itr2 = itr->second.find(_field);
    if (itr2 == itr->second.end())
        return;

    itr2->second = _value;

    if (m_observer != nullptr)
        m_observer->onWorldStateUpdate(_zone, _field, _value);
}

uint32_t WorldStatesHandler::GetWorldStateForZone(uint32_t _zone, uint32_t /*area*/, uint32_t _field) const
{
    const auto itr = m_worldStates.find(_zone);
    if (itr == m_worldStates.end())
        return 0;

    const auto itr2 = itr->second.find(_field);
    if (itr2 == itr->second.end())
        return 0;

    return itr2->second;
}

void WorldStatesHandler::BuildInitWorldStatesForZone(uint32_t _zone, uint32_t _area, WorldPacket& _data) const
{
    _data << uint32_t(m_map);
    _data << uint32_t(_zone);
    _data << uint32_t(_area);

#if VERSION_STRING > TBC
    const auto itr = m_worldStates.find(_zone);
    if (itr != m_worldStates.end())
    {
        _data << uint16_t(2 + itr->second.size());

        for (auto itr2 = itr->second.begin(); itr2 != itr->second.end(); ++itr2)
        {
            _data << uint32_t(itr2->first);
            _data << uint32_t(itr2->second);
        }
    }
    else
    {
        _data << uint16_t(2);
    }


    _data << uint32_t(3191);
    _data << uint32_t(worldConfig.arena.arenaSeason);
    _data << uint32_t(3901);
    _data << uint32_t(worldConfig.arena.arenaProgress);
#else
    uint32_t count = 0;
    size_t count_pos = _data.wpos();
    _data << uint16_t(0);

    {
        _data << uint32_t(3191);
        if (worldConfig.arena.arenaSeason > 6)
            _data << uint32_t(6);
        else
            _data << uint32_t(worldConfig.arena.arenaSeason);

        ++count;
    }

    _data.put<uint16_t>(count_pos, count);
#endif
}

void WorldStatesHandler::InitWorldStates(std::multimap<uint32_t, WorldState> const* _states)
{
    if (_states == nullptr)
        return;

    for (auto itr = _states->begin(); itr != _states->end(); ++itr)
    {
        uint32_t zone = itr->first;
        m_worldStates[zone];
        m_worldStates[zone].insert(std::pair(itr->second.field, itr->second.value));
    }
}

void WorldStatesHandler::setObserver(WorldStatesObserver* _observer){ m_observer = _observer; }
