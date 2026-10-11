/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Object.hpp"
#include <unordered_set>

namespace WDB::Structures { struct AreaTriggerEntry; }
class Unit;

// Server-side fallback AreaTrigger entity. Forever client-data triggers are
// reported by the client through CMSG_AREATRIGGER and validated against
// AreaTrigger.db2. This runtime object is only needed for legacy trigger volumes
// that no longer exist in the Forever client-data store.
class SERVER_DECL AreaTrigger final : public Object
{
public:
    AreaTrigger(uint64_t guid, uint32_t triggerId);
    ~AreaTrigger() override;

    bool create(WorldMap* map, uint32_t mapId, float x, float y, float z, float radius, float boxX, float boxY, float boxZ, float orientation);

    void considerTarget(Unit* target);
    void removeTarget(Unit* target);
    void refreshCurrentTargets();

    uint32_t getTriggerId() const { return m_triggerId; }
    float getSearchRadius() const { return m_searchRadius; }
    bool contains(Unit const* target) const;
    static bool contains(WDB::Structures::AreaTriggerEntry const& entry, Unit const* target);

    void onPreDetachFromWorld() override;

private:
    static bool containsShape(uint32_t mapId, float x, float y, float z, float radius,
        float boxX, float boxY, float boxZ, float orientation, Unit const* target);

    void handleEnter(Unit* target);
    void handleExit(Unit* target);

    uint32_t m_triggerId = 0;
    float m_radius = 0.0f;
    float m_boxX = 0.0f;
    float m_boxY = 0.0f;
    float m_boxZ = 0.0f;
    float m_boxOrientation = 0.0f;
    float m_searchRadius = 0.0f;
    std::unordered_set<uint64_t> m_insideUnits;
};
