/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WorldStatesHandler.hpp"

#include "ObjectMgr.hpp"
#include "Network/WorldPacket.hpp"
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

std::unordered_map<uint32_t, uint32_t> WorldStatesHandler::getWorldStatesForZone(uint32_t zone)
{
    const auto itr = m_worldStates.find(zone);
    if (itr == m_worldStates.end())
        return {};

    return itr->second;
}
