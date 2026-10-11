/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Objects/AreaTrigger.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Unit.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Storage/WDB/WDBStructures.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

AreaTrigger::AreaTrigger(uint64_t guid, uint32_t triggerId) : m_triggerId(triggerId)
{
    m_objectTypeId = TYPEID_AREATRIGGER;
    m_valuesCount = getSizeOfStructure(WoWObject);
    m_uint32Values = reinterpret_cast<uint32_t*>(wow_data_ptr = new uint8_t[sizeof(WoWObject)]{});
    m_updateMask.SetCount(getSizeOfStructure(WoWObject));
    setOType(TYPE_OBJECT);
    setGuid(guid);
    setEntry(triggerId);
    setScale(1.0f);
}

AreaTrigger::~AreaTrigger()
{
    delete[] wow_data_ptr;
    wow_data_ptr = nullptr;
    m_uint32Values = nullptr;
}

bool AreaTrigger::create(WorldMap* map, uint32_t mapId, float x, float y, float z, float radius, float boxX, float boxY, float boxZ, float orientation)
{
    if (!map)
        return false;

    Object::_Create(mapId, x, y, z, orientation);

    m_radius = std::max(0.0f, radius);
    m_boxX = std::max(0.0f, boxX);
    m_boxY = std::max(0.0f, boxY);
    m_boxZ = std::max(0.0f, boxZ);
    m_boxOrientation = orientation;

    if (m_radius > 0.0f)
        m_searchRadius = m_radius;
    else
        m_searchRadius = 0.5f * std::sqrt(m_boxX * m_boxX + m_boxY * m_boxY);

    // Broken/zero-sized data must never become a map-wide trigger. One yard is
    // only a spatial-query floor; contains() still rejects a zero-sized shape.
    m_searchRadius = std::max(1.0f, m_searchRadius);
    return true;
}

bool AreaTrigger::containsShape(uint32_t mapId, float x, float y, float z, float radius,
    float boxX, float boxY, float boxZ, float orientation, Unit const* target)
{
    if (!target || !target->IsInWorld() || target->GetMapId() != mapId)
        return false;

    const float dx = target->GetPositionX() - x;
    const float dy = target->GetPositionY() - y;
    const float dz = target->GetPositionZ() - z;

    radius = std::max(0.0f, radius);
    if (radius > 0.0f)
        return (dx * dx + dy * dy + dz * dz) <= (radius * radius);

    boxX = std::max(0.0f, boxX);
    boxY = std::max(0.0f, boxY);
    boxZ = std::max(0.0f, boxZ);
    if (boxX <= 0.0f || boxY <= 0.0f || boxZ <= 0.0f)
        return false;

    const float sinO = std::sin(orientation);
    const float cosO = std::cos(orientation);
    const float localX = dx * cosO + dy * sinO;
    const float localY = -dx * sinO + dy * cosO;

    return std::fabs(localX) <= (boxX * 0.5f) &&
           std::fabs(localY) <= (boxY * 0.5f) &&
           std::fabs(dz) <= (boxZ * 0.5f);
}

bool AreaTrigger::contains(Unit const* target) const
{
    if (!target || !IsInWorld() || target->getWorldMap() != getWorldMap())
        return false;

    return containsShape(GetMapId(), GetPositionX(), GetPositionY(), GetPositionZ(), m_radius,
        m_boxX, m_boxY, m_boxZ, m_boxOrientation, target);
}

bool AreaTrigger::contains(WDB::Structures::AreaTriggerEntry const& entry, Unit const* target)
{
    return containsShape(entry.mapId, entry.x, entry.y, entry.z, entry.boxRadius,
        entry.boxX, entry.boxY, entry.boxZ, entry.boxOrientation, target);
}

void AreaTrigger::considerTarget(Unit* target)
{
    if (!target || !IsInWorld())
        return;

    const uint64_t targetGuid = target->getGuid();
    const bool wasInside = m_insideUnits.contains(targetGuid);
    const bool isInside = contains(target);

    if (isInside == wasInside)
        return;

    if (isInside)
    {
        m_insideUnits.insert(targetGuid);
        target->addAreaTriggerTarget(getGuid());
        handleEnter(target);
    }
    else
    {
        m_insideUnits.erase(targetGuid);
        target->removeAreaTriggerTarget(getGuid());
        handleExit(target);
    }
}

void AreaTrigger::removeTarget(Unit* target)
{
    if (!target)
        return;

    if (m_insideUnits.erase(target->getGuid()) == 0)
        return;

    target->removeAreaTriggerTarget(getGuid());
    handleExit(target);
}

void AreaTrigger::refreshCurrentTargets()
{
    if (!IsInWorld() || m_insideUnits.empty())
        return;

    const std::vector<uint64_t> targets(m_insideUnits.begin(), m_insideUnits.end());
    for (uint64_t targetGuid : targets)
    {
        if (Unit* target = getWorldMapUnit(targetGuid))
            considerTarget(target);
        else
            m_insideUnits.erase(targetGuid);
    }
}

void AreaTrigger::handleEnter(Unit* target)
{
    if (!target->isPlayer())
        return;

    Player* player = static_cast<Player*>(target);
    // Prevent repeated enter processing until the matching exit clears the id.
    if (!player->activateAreaTrigger(m_triggerId))
        return;

    if (WorldSession* session = player->getSession())
        session->processAreaTrigger(m_triggerId);
}

void AreaTrigger::handleExit(Unit* target)
{
    if (target->isPlayer())
        static_cast<Player*>(target)->deactivateAreaTrigger(m_triggerId);
}

void AreaTrigger::onPreDetachFromWorld()
{
    if (!IsInWorld())
        return;

    const std::vector<uint64_t> targets(m_insideUnits.begin(), m_insideUnits.end());
    for (uint64_t targetGuid : targets)
    {
        if (Unit* target = getWorldMapUnit(targetGuid))
        {
            target->removeAreaTriggerTarget(getGuid());
            handleExit(target);
        }
    }
    m_insideUnits.clear();
}
