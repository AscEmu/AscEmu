/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <algorithm>
#include <unordered_set>
#include <set>

#include "VisibilitySystem.hpp"

#include "Objects/Object.hpp"
#include "Objects/Units/Unit.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/GameObject.h"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/DynamicObject.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Map/Maps/WorldMap.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// visibility
//////////////////////////////////////////////////////////////////////////////////////////
namespace visibility
{
    namespace
    {
        bool gridHasPhysicalObjects(const GridChunk& g)
        {
            for (const auto& owners : g.owners)
            {
                if (!owners.empty())
                    return true;
            }
            return false;
        }

        bool gridHasOnlyAutonomousResidents(const GridChunk& g)
        {
            bool hasPhysicalObjects = false;

            for (const auto& owners : g.owners)
            {
                for (ObjectHandle h : owners)
                {
                    hasPhysicalObjects = true;
                    if (!g.autonomousResidents.contains(h))
                        return false;
                }
            }

            return hasPhysicalObjects;
        }

        bool gridHasSubscriptions(const GridChunk& g)
        {
            for (const auto& cellPtr : g.cells)
            {
                if (!cellPtr)
                    continue;

                const auto& cell = *cellPtr;
                if (!cell.viewers.empty() || !cell.activators.empty())
                    return true;
            }

            return false;
        }

        bool gridIsCompletelyEmpty(const GridChunk& g)
        {
            return g.activeCells <= 0
                && !gridHasPhysicalObjects(g)
                && !gridHasSubscriptions(g)
                && g.autonomousResidents.empty();
        }

        void addUnique(std::vector<ObjectHandle>& vec, ObjectHandle h)
        {
            if (std::find(vec.begin(), vec.end(), h) == vec.end())
                vec.push_back(h);
        }

        bool isPublishActive(const PubState& pub) noexcept
        {
            return pub.mode != PublishMode::None;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// ctor
    //////////////////////////////////////////////////////////////////////////////////////////
    VisibilitySystem::VisibilitySystem(SpatialIndex& spatialIndex, Config cfg)
        : m_config(cfg), m_spatialIndex(spatialIndex)
    {
    }

    void VisibilitySystem::setDefaultSubscriptionRadius(int radiusCells) noexcept
    {
        const int radius = std::max(1, radiusCells);
        m_config.defaultViewerRadius = radius;
        m_config.defaultActivatorRadius = radius;
    }

    std::uint64_t VisibilitySystem::nextCandidateQueryStamp() const
    {
        ++m_candidateQueryStamp;
        if (m_candidateQueryStamp == 0)
            m_candidateQueryStamp = 1;

        return m_candidateQueryStamp;
    }

    bool VisibilitySystem::markCandidateOnce(ObjectHandle h, std::uint64_t stamp) const
    {
        const ObjectSlot* slot = nullptr;
        if (!m_spatialIndex.tryGet(h, slot))
            return false;

        if (slot->visibilityQueryStamp == stamp)
            return false;

        slot->visibilityQueryStamp = stamp;
        return true;
    }

    void VisibilitySystem::hideObjectFromKnownViewers(const WoWGuid& objectGuid)
    {
        const uint64_t objectRaw = objectGuid.getRawGuid();
        if (!objectRaw)
            return;

        std::vector<WoWGuid> viewers;
        {
            auto it = m_seenBy.find(objectRaw);
            if (it != m_seenBy.end())
            {
                viewers.reserve(it->second.size());
                for (uint64_t viewerRaw : it->second)
                    viewers.emplace_back(viewerRaw);
            }
        }

        for (const WoWGuid& viewerGuid : viewers)
            emitHiddenOnce(viewerGuid, objectGuid);
    }

    bool VisibilitySystem::isWithinInterestReach(const ObjectSlot& viewer, const ObjectSlot& obj, int objGid, int objLcid) const
    {
        if (!viewer.sub.active || !viewer.sub.viewer)
            return false;

        const int d = cellChebDistGlobal(viewer.sub.gid, viewer.sub.lcx, viewer.sub.lcy, objGid, objLcid);

        /// Normal subscription ring.
        if (d <= viewer.sub.radius)
            return true;

        if (!isPublishActive(obj.pub))
            return false;

        if (obj.pub.playersOnly && !isPlayerBackedViewer(&viewer))
            return false;

        if (obj.pub.mode == PublishMode::GridWide)
            return d <= m_config.gridWidePublishCells;

        /// CellRadius is treated as a total announce radius, not as an additive
        /// extension over the viewer ring. This keeps rules such as player->player
        /// 4 cells deterministic regardless of the configured default viewer ring.
        if (obj.pub.mode == PublishMode::CellRadius)
            return obj.pub.extraCells > 0 && d <= std::max(viewer.sub.radius, obj.pub.extraCells);

        return false;
    }

    /// Check whether publish-range alone keeps an object visible to a viewer.
    bool VisibilitySystem::isPublishedVisibleFor(const ObjectSlot* obj, const ObjectSlot* viewer, int objGid, int objLcid) const
    {
        if (!obj || !viewer)
            return false;

        const int d = cellChebDistGlobal(viewer->sub.gid, viewer->sub.lcx, viewer->sub.lcy, objGid, objLcid);
        if (d <= viewer->sub.radius)
            return false;

        return isWithinInterestReach(*viewer, *obj, objGid, objLcid);
    }

    /// Hide all objects in given cells for a viewer unless publish keeps them visible
    void VisibilitySystem::hideObjectsInCellsForViewer(ObjectHandle viewerH, const std::vector<CellRef>& cells)
    {
        const ObjectSlot* viewer = nullptr;

        if (!m_spatialIndex.tryGet(viewerH, viewer) || !viewer->sub.active || !viewer->sub.viewer)
            return;

        const auto stamp = nextCandidateQueryStamp();

        for (auto [gid, lcid] : cells)
        {
            const GridChunk* gp = m_spatialIndex.tryGetGrid(gid);
            if (!gp)
                continue;
            const CellChunk* cp = m_spatialIndex.tryGetCell(*gp, lcid);
            if (!cp)
                continue;
            for (size_t ci = 0; ci < static_cast<size_t>(Container::Count); ++ci)
            {
                for (auto oh : cp->byContainer[ci])
                {
                    if (!markCandidateOnce(oh, stamp))
                        continue;

                    const ObjectSlot* obj = nullptr;
                    if (!m_spatialIndex.tryGet(oh, obj))
                        continue;

                    const int objL = SpatialIndex::packCellFromPos(obj->pos);
                    const int objG = SpatialIndex::packGridFromPos(obj->pos);

                    if (isPublishedVisibleFor(obj, viewer, objG, objL))
                        continue;

                    emitHiddenOnce(viewer->meta.guid, obj->meta.guid);
                }
            }
        }
    }

    Unit* VisibilitySystem::resolveVisibilityRuleUnit(const ObjectSlot* viewer) const
    {
        if (!viewer || !viewer->native)
            return nullptr;

        auto* viewpoint = static_cast<Object*>(viewer->native);

        if (Unit* unit = viewpoint->ToUnit())
        {
            if (unit->m_playerControler && unit->m_playerControler->IsInWorld())
                return unit->m_playerControler;
            return unit;
        }

        if (auto* dyn = dynamic_cast<DynamicObject*>(viewpoint))
        {
            if (dyn->getDynamicType() != DYNAMIC_OBJECT_FARSIGHT_FOCUS)
                return nullptr;

            const WoWGuid casterGuid(dyn->getCasterGuid());
            const ObjectHandle casterH = m_spatialIndex.handleByGuid(casterGuid);
            const ObjectSlot* caster = nullptr;
            if (!casterH.id || !m_spatialIndex.tryGet(casterH, caster) || !caster || !caster->native)
                return nullptr;

            return static_cast<Object*>(caster->native)->ToUnit();
        }

        return nullptr;
    }

    bool VisibilitySystem::isPlayerBackedViewer(const ObjectSlot* viewer) const
    {
        Unit* ruleUnit = resolveVisibilityRuleUnit(viewer);
        return ruleUnit && ruleUnit->isPlayer();
    }

    float VisibilitySystem::getPublishedVisibilityDistanceSq(const ObjectSlot& obj) const noexcept
    {
        int rangeCells = 0;

        switch (obj.pub.mode)
        {
            case PublishMode::CellRadius:
                rangeCells = obj.pub.extraCells;
                break;
            case PublishMode::GridWide:
                rangeCells = m_config.gridWidePublishCells;
                break;
            case PublishMode::None:
            default:
                return 0.0f;
        }

        if (rangeCells <= 0)
            return 0.0f;

        const float distance = Cell::Size * static_cast<float>(rangeCells);
        return distance * distance;
    }

    bool VisibilitySystem::isVisibleToViewer(const ObjectSlot* obj, const ObjectSlot* viewer, int objGid, int objLcid) const
    {
        if (!obj || !viewer || !viewer->native || !obj->native)
            return false;

        if (!isWithinInterestReach(*viewer, *obj, objGid, objLcid))
            return false;

        Unit* ruleUnit = resolveVisibilityRuleUnit(viewer);
        auto* viewpoint = static_cast<Object*>(viewer->native);
        auto* object = static_cast<Object*>(obj->native);
        const float publishedVisibilityDistanceSq = getPublishedVisibilityDistanceSq(*obj);
        return ruleUnit ? ruleUnit->canSeeFrom(object, viewpoint, publishedVisibilityDistanceSq) : false;
    }

    bool VisibilitySystem::shouldRefreshViewerInterest(const ObjectSlot& viewer, const SpatialMoveResult& move) const
    {
        if (move.cellChanged)
            return true;

        if (!viewer.sub.lastInterestRefreshValid)
            return true;

        const float dx = move.newPos.x - viewer.sub.lastInterestRefreshPos.x;
        const float dy = move.newPos.y - viewer.sub.lastInterestRefreshPos.y;
        const float threshold = std::max(0.0f, m_config.viewerInterestRefreshDistance);
        return (dx * dx + dy * dy) >= (threshold * threshold);
    }

    void VisibilitySystem::markViewerInterestRefreshed(ObjectSlot& viewer, const LocationVector& pos)
    {
        viewer.sub.lastInterestRefreshPos = pos;
        viewer.sub.lastInterestRefreshValid = true;
    }

    void VisibilitySystem::refreshViewerInterest(ObjectHandle viewerH, ObjectSlot& viewer)
    {
        if (!viewer.sub.viewer)
            return;

        proximitySweepViewer(viewerH);
        emitPublishedForViewer(viewerH, /*isSubscribe*/true);
        markViewerInterestRefreshed(viewer, viewer.pos);
    }

    bool VisibilitySystem::shouldRefreshMovedObjectInterest(const ObjectSlot& object, const SpatialMoveResult& move) const
    {
        if (move.cellChanged)
            return true;

        if (!object.lastObjectInterestRefreshValid)
            return true;

        const float dx = move.newPos.x - object.lastObjectInterestRefreshPos.x;
        const float dy = move.newPos.y - object.lastObjectInterestRefreshPos.y;
        const float threshold = std::max(0.0f, m_config.movedObjectInterestRefreshDistance);
        return (dx * dx + dy * dy) >= (threshold * threshold);
    }

    void VisibilitySystem::markMovedObjectInterestRefreshed(ObjectSlot& object, const LocationVector& pos)
    {
        object.lastObjectInterestRefreshPos = pos;
        object.lastObjectInterestRefreshValid = true;
    }

    void VisibilitySystem::refreshMovedObjectInterest(ObjectHandle objectH, ObjectSlot& object)
    {
        std::vector<ObjectHandle> viewers;
        viewers.reserve(256);
        collectCandidateViewersForObject(objectH, viewers);

        for (auto viewerH : viewers)
            updateVisibilityPair(viewerH, objectH);

        markMovedObjectInterestRefreshed(object, object.pos);
    }

    void VisibilitySystem::updateVisibilityPair(ObjectHandle viewerH, ObjectHandle objH)
    {
        ++m_performanceCounters.pairChecks;
        const ObjectSlot* viewer = nullptr;
        const ObjectSlot* obj = nullptr;
        if (!m_spatialIndex.tryGet(viewerH, viewer) || !m_spatialIndex.tryGet(objH, obj))
            return;

        if (!viewer->sub.active || !viewer->sub.viewer)
            return;

        const int objGid = SpatialIndex::packGridFromPos(obj->pos);
        const int objLcid = SpatialIndex::packCellFromPos(obj->pos);
        if (isVisibleToViewer(obj, viewer, objGid, objLcid))
            emitVisibleOnce(viewer->meta.guid, obj->meta.guid);
        else
            emitHiddenOnce(viewer->meta.guid, obj->meta.guid);
    }

    void VisibilitySystem::collectCandidateViewersForObject(ObjectHandle objH, std::vector<ObjectHandle>& out) const
    {
        out.clear();

        const ObjectSlot* obj = nullptr;
        if (!m_spatialIndex.tryGet(objH, obj))
            return;

        const int objGid = SpatialIndex::packGridFromPos(obj->pos);
        const int objLcid = SpatialIndex::packCellFromPos(obj->pos);
        auto [objLcx, objLcy] = unpackCellId2(objLcid);

        const auto stamp = nextCandidateQueryStamp();

        auto addViewer = [&](ObjectHandle vh)
        {
            if (!markCandidateOnce(vh, stamp))
                return;

            const ObjectSlot* viewer = nullptr;
            if (!m_spatialIndex.tryGet(vh, viewer))
                return;

            if (!viewer->sub.active || !viewer->sub.viewer)
                return;

            if (isWithinInterestReach(*viewer, *obj, objGid, objLcid))
                out.push_back(vh);
        };

        int candidateRadius = 0;
        if (isPublishActive(obj->pub))
        {
            if (obj->pub.mode == PublishMode::GridWide)
                candidateRadius = std::max(candidateRadius, m_config.gridWidePublishCells);
            else if (obj->pub.mode == PublishMode::CellRadius)
                candidateRadius = std::max(candidateRadius, obj->pub.extraCells);
        }

        /// Normal objects only need the object's current cell because viewer rings
        /// are materialized as cell viewers. Published objects need a wider
        /// candidate scan so stationary publishers can announce to later viewers.
        if (candidateRadius <= 0)
        {
            if (const GridChunk* gp = m_spatialIndex.tryGetGrid(objGid))
            {
                if (const CellChunk* cp = m_spatialIndex.tryGetCell(*gp, objLcid))
                {
                    for (auto vh : cp->viewers)
                        addViewer(vh);
                }
            }
            return;
        }

        m_spatialIndex.forEachRingCell(objGid, objLcx, objLcy, candidateRadius, [&](int gid, int lcid)
        {
            const GridChunk* gp = m_spatialIndex.tryGetGrid(gid);
            if (!gp)
                return;
            const CellChunk* cp = m_spatialIndex.tryGetCell(*gp, lcid);
            if (!cp)
                return;
            for (auto vh : cp->viewers)
                addViewer(vh);
        });
    }

    bool VisibilitySystem::isPlayerSpatialPublisher(const ObjectSlot& obj) const
    {
        return obj.meta.container == Container::Players &&
            obj.pub.mode == PublishMode::CellRadius &&
            obj.pub.playersOnly &&
            obj.pub.extraCells > 0;
    }

    bool VisibilitySystem::isBucketedPublisher(const ObjectSlot& obj) const
    {
        if (!isPublishActive(obj.pub))
            return false;

        /// Player->Player publishing is resolved through the SpatialIndex around
        /// the viewer/object. Keeping every player in a global publisher list would
        /// make viewer refreshes scale with total map population.
        return !isPlayerSpatialPublisher(obj);
    }

    void VisibilitySystem::addPublishBucket(ObjectHandle who, const ObjectSlot& obj)
    {
        if (!isBucketedPublisher(obj))
            return;

        if (obj.pub.mode == PublishMode::GridWide)
        {
            addUnique(m_gridWidePublishers, who);
            return;
        }

        if (obj.pub.mode == PublishMode::CellRadius)
            addUnique(m_cellRadiusPublishers, who);
    }

    void VisibilitySystem::removePublishBucket(ObjectHandle who)
    {
        auto removeFrom = [&](std::vector<ObjectHandle>& v, const char* name)
        {
            const auto before = std::count(v.begin(), v.end(), who);
            if (before > 1)
                sLogger.warning("vis sanity: duplicate handle in {} publish bucket id={} gen={} count={}", name, who.id, who.gen, before);

            v.erase(std::remove(v.begin(), v.end(), who), v.end());

            if (std::find(v.begin(), v.end(), who) != v.end())
                sLogger.warning("vis sanity: handle remained in {} publish bucket after removal id={} gen={}", name, who.id, who.gen);
        };

        removeFrom(m_gridWidePublishers, "gridWide");
        removeFrom(m_cellRadiusPublishers, "cellRadius");
    }

    bool VisibilitySystem::isInPublishBucket(ObjectHandle who) const
    {
        return std::find(m_gridWidePublishers.begin(), m_gridWidePublishers.end(), who) != m_gridWidePublishers.end()
            || std::find(m_cellRadiusPublishers.begin(), m_cellRadiusPublishers.end(), who) != m_cellRadiusPublishers.end();
    }

    bool VisibilitySystem::hasCellSubscription(ObjectHandle who) const
    {
        for (const auto& [gid, grid] : m_spatialIndex.gridPointersSnapshot())
        {
            if (!grid)
                continue;

            const auto& g = *grid;
            for (const auto& cellPtr : g.cells)
            {
                if (!cellPtr)
                    continue;

                const auto& cell = *cellPtr;
                if (std::find(cell.viewers.begin(), cell.viewers.end(), who) != cell.viewers.end())
                    return true;
                if (std::find(cell.activators.begin(), cell.activators.end(), who) != cell.activators.end())
                    return true;
            }
        }
        return false;
    }

    void VisibilitySystem::collectPlayerPublishersAroundCell(int gid, int lcx, int lcy, int radiusCells,
        std::vector<ObjectHandle>& out, std::uint64_t stamp) const
    {
        if (radiusCells <= 0)
            return;

        m_spatialIndex.forEachRingCell(gid, lcx, lcy, radiusCells, [&](int cgid, int clcid)
        {
            const GridChunk* gp = m_spatialIndex.tryGetGrid(cgid);
            if (!gp)
                return;

            const CellChunk* cp = m_spatialIndex.tryGetCell(*gp, clcid);
            if (!cp)
                return;
            const auto& players = cp->byContainer[static_cast<std::size_t>(Container::Players)];
            for (auto oh : players)
            {
                if (markCandidateOnce(oh, stamp))
                    out.push_back(oh);
            }
        });
    }

    void VisibilitySystem::reconcilePublishedCandidateForViewerMove(ObjectHandle viewerH, const ObjectSlot& viewer, ObjectHandle objH,
        int oldGid, int oldLcid, int newGid, int newLcid)
    {
        const ObjectSlot* obj = nullptr;
        if (!m_spatialIndex.tryGet(objH, obj))
            return;
        if (!isPublishActive(obj->pub))
            return;
        if (obj->pub.playersOnly && !isPlayerBackedViewer(&viewer))
            return;

        auto [oldLcx, oldLcy] = unpackCellId2(oldLcid);
        auto [newLcx, newLcy] = unpackCellId2(newLcid);

        const int objG = SpatialIndex::packGridFromPos(obj->pos);
        const int objL = SpatialIndex::packCellFromPos(obj->pos);

        const int oldD = cellChebDistGlobal(oldGid, oldLcx, oldLcy, objG, objL);
        const int newD = cellChebDistGlobal(newGid, newLcx, newLcy, objG, objL);

        const bool oldRing = oldD <= viewer.sub.radius;
        const bool newRing = newD <= viewer.sub.radius;
        /// isWithinInterestReach uses the viewer's current subscription position.
        /// For the old side, reproduce the same reach rule with the old distance.
        bool oldPublished = false;
        if (!oldRing)
        {
            if (obj->pub.mode == PublishMode::GridWide)
                oldPublished = oldD <= m_config.gridWidePublishCells;
            else if (obj->pub.mode == PublishMode::CellRadius)
                oldPublished = obj->pub.extraCells > 0 && oldD <= std::max(viewer.sub.radius, obj->pub.extraCells);
        }

        bool newPublished = false;
        if (!newRing)
        {
            if (obj->pub.mode == PublishMode::GridWide)
                newPublished = newD <= m_config.gridWidePublishCells;
            else if (obj->pub.mode == PublishMode::CellRadius)
                newPublished = obj->pub.extraCells > 0 && newD <= std::max(viewer.sub.radius, obj->pub.extraCells);
        }

        if (!oldPublished && newPublished)
        {
            updateVisibilityPair(viewerH, objH);
            return;
        }

        if (oldPublished && !newPublished && !newRing)
        {
            emitHiddenOnce(viewer.meta.guid, obj->meta.guid);
            return;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Visible tracking helpers
    //////////////////////////////////////////////////////////////////////////////////////////
    void VisibilitySystem::collectViewersOf(const WoWGuid& object, std::vector<WoWGuid>& out) const
    {
        const uint64_t oraw = object.getRawGuid();
        auto it = m_seenBy.find(oraw);
        if (it == m_seenBy.end())
            return;
        out.reserve(out.size() + it->second.size());
        for (uint64_t vraw : it->second)
            out.emplace_back(WoWGuid(vraw));
    }

    void VisibilitySystem::collectVisibleObjectsForViewer(const WoWGuid& viewer, std::vector<WoWGuid>& out) const
    {
        const uint64_t vraw = viewer.getRawGuid();
        auto it = m_visibleNow.find(vraw);
        if (it == m_visibleNow.end())
            return;

        out.reserve(out.size() + it->second.size());
        for (uint64_t oraw : it->second)
            out.emplace_back(WoWGuid(oraw));
    }

    void VisibilitySystem::clearVisibleObjectsForViewer(const WoWGuid& viewer, std::vector<WoWGuid>& out)
    {
        const uint64_t vraw = viewer.getRawGuid();
        auto it = m_visibleNow.find(vraw);
        if (it == m_visibleNow.end())
            return;

        out.reserve(out.size() + it->second.size());
        for (uint64_t raw : it->second)
        {
            out.emplace_back(WoWGuid(raw));

            if (auto jt = m_seenBy.find(raw); jt != m_seenBy.end())
            {
                jt->second.erase(vraw);
                if (jt->second.empty())
                    m_seenBy.erase(jt);
            }
        }

        m_visibleNow.erase(it);
    }

    void VisibilitySystem::resetViewerForRelocation(ObjectHandle who, std::vector<WoWGuid>& out)
    {
        ObjectSlot* s = nullptr;
        if (!m_spatialIndex.tryGet(who, s) || !s)
            return;

        const WoWGuid viewerGuid = s->meta.guid;

        /// First remove the tracked visible-pair state. The caller sends the
        /// actual OutOfRange packet immediately from the returned GUID list.
        clearVisibleObjectsForViewer(viewerGuid, out);

        if (!s->sub.active)
            return;

        const int gid = s->sub.gid;
        const int lcx = s->sub.lcx;
        const int lcy = s->sub.lcy;
        const int radius = s->sub.radius;

        /// Remove stale subscriptions synchronously before applying the new role state.
        /// here: Release Spirit / same-map teleport changes position immediately,
        /// so leaving the old viewer ring in cells until later can recreate stale
        /// creatures from the death location.
        if (s->sub.viewer)
            unsubscribeRing(who, gid, lcx, lcy, radius, /*asPlayer*/true);

        if (s->sub.activator)
            unsubscribeRing(who, gid, lcx, lcy, radius, /*asPlayer*/false);

        s->sub = {};
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Metrics
    //////////////////////////////////////////////////////////////////////////////////////////
    VisSnapshot VisibilitySystem::snapshot() const
    {
        VisSnapshot s;
        s.grids = m_spatialIndex.gridCount();
        s.poolLive = m_spatialIndex.liveCount();
        s.guidIndex = m_spatialIndex.guidCount();
        s.gridWidePublishers = m_gridWidePublishers.size();
        s.cellRadiusPublishers = m_cellRadiusPublishers.size();

        for (auto const& [gid, grid] : m_spatialIndex.gridPointersSnapshot())
        {
            if (!grid)
                continue;

            const auto& g = *grid;
            s.cells += static_cast<std::size_t>(std::count_if(g.cells.begin(), g.cells.end(), [](const auto& cell) { return static_cast<bool>(cell); }));

            const int active = g.activeCells;
            if (active > 0 || isGridPinned(gid))
                ++s.activeGrids;

            if (active > 0)
                s.activeCells += static_cast<size_t>(active);

            for (size_t i = 0; i < s.owners.size(); ++i)
            {
                s.owners[i] += g.owners[i].size();
                for (auto h : g.owners[i])
                {
                    const ObjectSlot* slot = nullptr;
                    if (!m_spatialIndex.tryGet(h, slot))
                        continue;

                    s.nearCacheGuidsCapacity += slot->nearCache.guids.capacity();
                    s.nearCacheStampsCapacity += slot->nearCache.stamps.capacity();
                    s.nearCacheCachedGuids += slot->nearCache.guids.size();
                }
            }

            for (auto const& cellPtr : g.cells)
            {
                if (!cellPtr)
                    continue;

                const auto& cell = *cellPtr;
                s.cellViewers += cell.viewers.size();
                s.cellActivators += cell.activators.size();
            }
        }

        {
            s.visibleViewers = m_visibleNow.size();
            for (auto const& [viewer, objects] : m_visibleNow)
                s.visiblePairs += objects.size();

            s.seenObjects = m_seenBy.size();
            for (auto const& [object, viewers] : m_seenBy)
                s.seenPairs += viewers.size();
        }

        return s;
    }

    void VisibilitySystem::logMemoryDiagnostics(const char* reason) const
    {
        const auto s = snapshot();
        const auto idx = [](Container c) { return static_cast<std::size_t>(c); };

        sLogger.warning(
            "vismem: reason={} grids={} activeGrids={} cells={} activeCells={} poolLive={} guidIndex={} "
            "owners[coro={},cre={},dyn={},go={},plr={},trans={},pet={},unk={}] "
            "cellViewers={} cellActivators={} visibleViewers={} visiblePairs={} seenObjects={} seenPairs={} "
            "publishers[gridWide={},cellRadius={}] nearCache[cachedGuids={},guidCap={},stampCap={}]",
            reason ? reason : "",
            s.grids,
            s.activeGrids,
            s.cells,
            s.activeCells,
            s.poolLive,
            s.guidIndex,
            s.owners[idx(Container::Corpses)],
            s.owners[idx(Container::Creatures)],
            s.owners[idx(Container::DynamicObjects)],
            s.owners[idx(Container::GameObjects)],
            s.owners[idx(Container::Players)],
            s.owners[idx(Container::Transporter)],
            s.owners[idx(Container::Pets)],
            s.owners[idx(Container::Unknown)],
            s.cellViewers,
            s.cellActivators,
            s.visibleViewers,
            s.visiblePairs,
            s.seenObjects,
            s.seenPairs,
            s.gridWidePublishers,
            s.cellRadiusPublishers,
            s.nearCacheCachedGuids,
            s.nearCacheGuidsCapacity,
            s.nearCacheStampsCapacity);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Spatial lifecycle event entry points
    //////////////////////////////////////////////////////////////////////////////////////////
    void VisibilitySystem::onObjectAdded(ObjectHandle h)
    {
        const ObjectSlot* s = nullptr;
        if (!m_spatialIndex.tryGet(h, s))
            return;

        notifySpawn(h);
    }

    void VisibilitySystem::onObjectRemoving(ObjectHandle h)
    {
        ObjectSlot* s = nullptr;
        if (!m_spatialIndex.tryGet(h, s))
            return;

        hideObjectFromKnownViewers(s->meta.guid);

        if (s->interest.autonomous)
            setAutonomousResidency(h, false);

        if (s->sub.active)
        {
            const auto cells = m_spatialIndex.buildRingCells(s->sub.gid, s->sub.lcx, s->sub.lcy, s->sub.radius);

            if (s->sub.viewer)
            {
                hideObjectsInCellsForViewer(h, cells);
                emitPublishedForViewer(h, /*isSubscribe*/false);
                unsubscribeRing(h, s->sub.gid, s->sub.lcx, s->sub.lcy, s->sub.radius, /*asPlayer*/true);
            }

            if (s->sub.activator)
                unsubscribeRing(h, s->sub.gid, s->sub.lcx, s->sub.lcy, s->sub.radius, /*asPlayer*/false);

            if (hasCellSubscription(h))
                sLogger.warning("vis sanity: handle still subscribed after unsubscribeRing during remove guid={} id={} gen={}", s->meta.guid.getRawGuid(), h.id, h.gen);

            s->sub = {};
        }

        notifyDespawn(h);
        removePublishBucket(h);

        if (isInPublishBucket(h))
            sLogger.warning("vis sanity: handle still in publish bucket after onObjectRemoving guid={} id={} gen={}", s->meta.guid.getRawGuid(), h.id, h.gen);
    }

    void VisibilitySystem::onObjectMoved(ObjectHandle h, const SpatialMoveResult& move)
    {
        if (!move.valid)
            return;

        ObjectSlot* s = nullptr;
        if (!m_spatialIndex.tryGet(h, s))
            return;

        if (move.gridChanged && s->interest.autonomous)
            moveAutonomousResidency(h, move.oldGrid, move.newGrid);

        if (s->sub.active)
        {
            if (move.cellChanged)
            {
                auto [lcx, lcy] = worldToLocal(move.newPos);

                std::vector<CellRef> toUnsub;
                std::vector<CellRef> toSub;

                const int radius = s->sub.radius;
                constexpr int cellsPerGrid = Cell::CellsPerTile;
                constexpr int totalCellsPerAxis = Terrain::TilesCount * cellsPerGrid;

                auto [oldGx, oldGy] = unpackGridId(s->sub.gid);
                auto [newGx, newGy] = unpackGridId(move.newGrid);
                const int oldCenterX = oldGx * cellsPerGrid + s->sub.lcx;
                const int oldCenterY = oldGy * cellsPerGrid + s->sub.lcy;
                const int newCenterX = newGx * cellsPerGrid + lcx;
                const int newCenterY = newGy * cellsPerGrid + lcy;
                const int dx = newCenterX - oldCenterX;
                const int dy = newCenterY - oldCenterY;

                const bool singleCellStep =
                    std::abs(dx) <= 1 && std::abs(dy) <= 1 && (dx != 0 || dy != 0);
                const bool awayFromMapEdge =
                    oldCenterX - radius >= 0 && oldCenterX + radius < totalCellsPerAxis &&
                    oldCenterY - radius >= 0 && oldCenterY + radius < totalCellsPerAxis &&
                    newCenterX - radius >= 0 && newCenterX + radius < totalCellsPerAxis &&
                    newCenterY - radius >= 0 && newCenterY + radius < totalCellsPerAxis;

                if (singleCellStep && awayFromMapEdge)
                {
                    const std::size_t stripReserve = static_cast<std::size_t>(4 * radius + 2);
                    toUnsub.reserve(stripReserve);
                    toSub.reserve(stripReserve);

                    auto makeCellRef = [](int globalX, int globalY) -> CellRef
                    {
                        const int gx = globalX / Cell::CellsPerTile;
                        const int gy = globalY / Cell::CellsPerTile;
                        const int cx = globalX % Cell::CellsPerTile;
                        const int cy = globalY % Cell::CellsPerTile;
                        return { packGridId(gx, gy), packCellId(cx, cy) };
                    };

                    auto addUniqueCell = [](std::vector<CellRef>& cells, CellRef ref)
                    {
                        if (std::find(cells.begin(), cells.end(), ref) == cells.end())
                            cells.push_back(ref);
                    };

                    if (dx != 0)
                    {
                        const int oldStripX = dx > 0 ? oldCenterX - radius : oldCenterX + radius;
                        const int newStripX = dx > 0 ? newCenterX + radius : newCenterX - radius;

                        for (int y = oldCenterY - radius; y <= oldCenterY + radius; ++y)
                            addUniqueCell(toUnsub, makeCellRef(oldStripX, y));
                        for (int y = newCenterY - radius; y <= newCenterY + radius; ++y)
                            addUniqueCell(toSub, makeCellRef(newStripX, y));
                    }

                    if (dy != 0)
                    {
                        const int oldStripY = dy > 0 ? oldCenterY - radius : oldCenterY + radius;
                        const int newStripY = dy > 0 ? newCenterY + radius : newCenterY - radius;

                        for (int x = oldCenterX - radius; x <= oldCenterX + radius; ++x)
                            addUniqueCell(toUnsub, makeCellRef(x, oldStripY));
                        for (int x = newCenterX - radius; x <= newCenterX + radius; ++x)
                            addUniqueCell(toSub, makeCellRef(x, newStripY));
                    }
                }
                else
                {
                    /// Teleports, multi-cell moves and map-edge transitions use the
                    /// general path so the existing clamping semantics stay intact.
                    const auto oldCells = m_spatialIndex.buildRingCells(
                        s->sub.gid, s->sub.lcx, s->sub.lcy, radius);
                    const auto newCells = m_spatialIndex.buildRingCells(
                        move.newGrid, lcx, lcy, radius);

                    toUnsub.reserve(oldCells.size());
                    toSub.reserve(newCells.size());

                    std::set_difference(oldCells.begin(), oldCells.end(),
                        newCells.begin(), newCells.end(),
                        std::back_inserter(toUnsub));
                    std::set_difference(newCells.begin(), newCells.end(),
                        oldCells.begin(), oldCells.end(),
                        std::back_inserter(toSub));
                }

                /// Add new cells before removing old ones so activators cannot
                /// temporarily drop activeCells to zero while crossing a border.
                for (auto [gid, lcid] : toSub)
                {
                    if (s->sub.activator)
                    {
                        cellSubscribe(h, gid, lcid, false);
                    }
                    if (s->sub.viewer)
                    {
                        cellSubscribe(h, gid, lcid, true);
                    }
                }

                for (auto [gid, lcid] : toUnsub)
                {
                    if (s->sub.viewer)
                    {
                        cellUnsubscribe(h, gid, lcid, true);
                    }
                    if (s->sub.activator)
                    {
                        cellUnsubscribe(h, gid, lcid, false);
                    }
                }

                const int oldG = s->sub.gid;
                const int oldLcx = s->sub.lcx;
                const int oldLcy = s->sub.lcy;
                s->sub.gid = move.newGrid;
                s->sub.lcx = lcx;
                s->sub.lcy = lcy;

                if (s->sub.viewer)
                    hideObjectsInCellsForViewer(h, toUnsub);

                if (s->sub.viewer && shouldRefreshViewerInterest(*s, move))
                    refreshViewerInterest(h, *s);

                if (s->sub.viewer)
                    reconcilePublishedForViewerMove(h, oldG, packCellId(oldLcx, oldLcy), move.newGrid, packCellId(lcx, lcy));
            }
            else if (s->sub.viewer && shouldRefreshViewerInterest(*s, move))
            {
                refreshViewerInterest(h, *s);
            }
        }

        if (move.gridChanged)
        {
            for (auto& f : m_eventHub.onGridChanged)
                f(m_spatialIndex.meta(h).guid, move.oldGrid, move.newGrid);
        }

        if (move.cellChanged)
        {
            notifyCrossCellMove(h, move.oldGrid, move.oldCell, move.newGrid, move.newCell);
            emitPublishedForPublisherMove(h, move.oldGrid, move.oldCell, move.newGrid, move.newCell);
        }

        if (shouldRefreshMovedObjectInterest(*s, move))
            refreshMovedObjectInterest(h, *s);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Subscriptions (public API)
    //////////////////////////////////////////////////////////////////////////////////////////
    void VisibilitySystem::subscribeViewer(ObjectHandle who, int radiusCells)
    {
        ObjectSlot* s = nullptr;
        if (!m_spatialIndex.tryGet(who, s))
            return;

        const int gid = SpatialIndex::packGridFromPos(s->pos);
        auto [lcx, lcy] = worldToLocal(s->pos);

        const bool addViewer = s->sub.active && !s->sub.viewer;

        if (!s->sub.active)
        {
            s->sub = { radiusCells, true, false, true, gid, lcx, lcy };
            const auto cells = m_spatialIndex.buildRingCells(gid, lcx, lcy, radiusCells);

            for (auto [ngid, nlcid] : cells)
                cellSubscribe(who, ngid, nlcid, /*asPlayer*/true);

            refreshViewerInterest(who, *s);
            return;
        }

        if (radiusCells > s->sub.radius)
        {
            const auto oldCells = m_spatialIndex.buildRingCells(s->sub.gid, s->sub.lcx, s->sub.lcy, s->sub.radius);
            const auto newCells = m_spatialIndex.buildRingCells(s->sub.gid, s->sub.lcx, s->sub.lcy, radiusCells);

            std::vector<CellRef> toAdd;
            std::set_difference(newCells.begin(), newCells.end(),
                oldCells.begin(), oldCells.end(),
                std::back_inserter(toAdd));

            for (auto [ngid, nlcid] : toAdd)
            {
                if (s->sub.activator)
                    cellSubscribe(who, ngid, nlcid, /*asPlayer*/false);
                cellSubscribe(who, ngid, nlcid, /*asPlayer*/true);
            }

            s->sub.radius = radiusCells;
        }

        if (addViewer)
        {
            const auto cells = m_spatialIndex.buildRingCells(s->sub.gid, s->sub.lcx, s->sub.lcy, s->sub.radius);
            for (auto [ngid, nlcid] : cells)
                cellSubscribe(who, ngid, nlcid, /*asPlayer*/true);
            s->sub.viewer = true;
        }

        if (s->sub.viewer)
            refreshViewerInterest(who, *s);
    }

    void VisibilitySystem::subscribeActivator(ObjectHandle who, int radiusCells)
    {
        ObjectSlot* s = nullptr;
        if (!m_spatialIndex.tryGet(who, s))
            return;

        const int gid = SpatialIndex::packGridFromPos(s->pos);
        auto [lcx, lcy] = worldToLocal(s->pos);

        const bool addActivator = s->sub.active && !s->sub.activator;

        if (!s->sub.active)
        {
            s->sub = { radiusCells, false, true, true, gid, lcx, lcy };
            const auto cells = m_spatialIndex.buildRingCells(gid, lcx, lcy, radiusCells);

            for (auto [ngid, nlcid] : cells)
                cellSubscribe(who, ngid, nlcid, /*asPlayer*/false);
            return;
        }

        if (radiusCells > s->sub.radius)
        {
            const auto oldCells = m_spatialIndex.buildRingCells(s->sub.gid, s->sub.lcx, s->sub.lcy, s->sub.radius);
            const auto newCells = m_spatialIndex.buildRingCells(s->sub.gid, s->sub.lcx, s->sub.lcy, radiusCells);

            std::vector<CellRef> toAdd;
            std::set_difference(newCells.begin(), newCells.end(),
                oldCells.begin(), oldCells.end(),
                std::back_inserter(toAdd));

            for (auto [ngid, nlcid] : toAdd)
            {
                cellSubscribe(who, ngid, nlcid, /*asPlayer*/false);
                if (s->sub.viewer)
                    cellSubscribe(who, ngid, nlcid, /*asPlayer*/true);
            }

            s->sub.radius = radiusCells;
        }

        if (addActivator)
        {
            const auto cells = m_spatialIndex.buildRingCells(s->sub.gid, s->sub.lcx, s->sub.lcy, s->sub.radius);
            for (auto [ngid, nlcid] : cells)
                cellSubscribe(who, ngid, nlcid, /*asPlayer*/false);
            s->sub.activator = true;
        }
    }

    void VisibilitySystem::unsubscribe(ObjectHandle who)
    {
        ObjectSlot* s = nullptr;
        if (!m_spatialIndex.tryGet(who, s) || !s->sub.active)
            return;

        const auto cells = m_spatialIndex.buildRingCells(s->sub.gid, s->sub.lcx, s->sub.lcy, s->sub.radius);
        if (s->sub.viewer)
        {
            hideObjectsInCellsForViewer(who, cells);
            emitPublishedForViewer(who, /*isSubscribe*/false);
            unsubscribeRing(who, s->sub.gid, s->sub.lcx, s->sub.lcy, s->sub.radius, /*asPlayer*/true);
        }

        if (s->sub.activator)
            unsubscribeRing(who, s->sub.gid, s->sub.lcx, s->sub.lcy, s->sub.radius, /*asPlayer*/false);

        s->sub.lastInterestRefreshValid = false;
        s->sub = {};
    }

    void VisibilitySystem::setViewerRole(ObjectHandle who, bool enabled, int radiusCells)
    {
        if (enabled)
        {
            subscribeViewer(who, radiusCells);
            return;
        }

        ObjectSlot* s = nullptr;
        if (!m_spatialIndex.tryGet(who, s) || !s->sub.active || !s->sub.viewer)
            return;

        const auto cells = m_spatialIndex.buildRingCells(s->sub.gid, s->sub.lcx, s->sub.lcy, s->sub.radius);
        hideObjectsInCellsForViewer(who, cells);
        emitPublishedForViewer(who, /*isSubscribe*/false);
        unsubscribeRing(who, s->sub.gid, s->sub.lcx, s->sub.lcy, s->sub.radius, /*asPlayer*/true);
        s->sub.viewer = false;
        s->sub.lastInterestRefreshValid = false;

        if (!s->sub.activator)
            s->sub = {};
    }

    void VisibilitySystem::setActivatorRole(ObjectHandle who, bool enabled, int radiusCells)
    {
        if (enabled)
        {
            subscribeActivator(who, radiusCells);
            return;
        }

        ObjectSlot* s = nullptr;
        if (!m_spatialIndex.tryGet(who, s) || !s->sub.active || !s->sub.activator)
            return;

        unsubscribeRing(who, s->sub.gid, s->sub.lcx, s->sub.lcy, s->sub.radius, /*asPlayer*/false);
        s->sub.activator = false;

        if (!s->sub.viewer)
        {
            s->sub.lastInterestRefreshValid = false;
            s->sub = {};
        }
    }

    void VisibilitySystem::ensureGridMaterialized(int gid)
    {
        if (gid < 0 || gid >= Terrain::TilesCount * Terrain::TilesCount)
            return;

        m_spatialIndex.getOrCreateGrid(gid);
    }

    void VisibilitySystem::setAutonomousResidency(ObjectHandle who, bool enabled)
    {
        ObjectSlot* slot = nullptr;
        if (!m_spatialIndex.tryGet(who, slot))
            return;

        const int gid = SpatialIndex::packGridFromPos(slot->pos);
        GridChunk& grid = m_spatialIndex.getOrCreateGrid(gid);

        if (enabled)
        {
            // Residency alone must not start an unload cycle: a newly materialized
            // autonomous-only grid has never loaded its normal spawn content. If the
            // grid was active before, the normal activator unsubscribe path already
            // owns the idle/unload timer.
            grid.autonomousResidents.insert(who);
        }
        else
        {
            grid.autonomousResidents.erase(who);

            if (grid.activeCells <= 0 && !gridHasSubscriptions(grid) && gridHasPhysicalObjects(grid) &&
                grid.idleSince.time_since_epoch().count() == 0)
            {
                grid.idleSince = std::chrono::steady_clock::now();
            }
        }
    }

    void VisibilitySystem::moveAutonomousResidency(ObjectHandle who, int oldGid, int newGid)
    {
        if (oldGid == newGid)
            return;

        // The SpatialIndex has already materialized and moved the object to newGid.
        // Register the destination first so the object is never temporarily without
        // a resident grid while crossing a grid boundary.
        GridChunk& newGrid = m_spatialIndex.getOrCreateGrid(newGid);
        newGrid.autonomousResidents.insert(who);

        if (GridChunk* oldGrid = m_spatialIndex.tryGetGrid(oldGid))
            oldGrid->autonomousResidents.erase(who);
    }

    void VisibilitySystem::activateGrid(int gid)
    {
        auto& g = m_spatialIndex.getOrCreateGrid(gid);
        if (g.activeCells > 0 || isGridPinned(gid))
            return;

        const int anyCell = packCellId(0, 0);
        ObjectHandle ghost{ 0u, 0u };
        cellSubscribe(ghost, gid, anyCell, /*asPlayer*/false);
        cellUnsubscribe(ghost, gid, anyCell, /*asPlayer*/false);
    }

    void VisibilitySystem::pinGrid(int gid)
    {
        if (gid < 0 || gid >= Terrain::TilesCount * Terrain::TilesCount)
            return;

        auto& refs = m_pinnedGridRefs[static_cast<std::size_t>(gid)];
        if (refs++ != 0)
            return;

        auto& g = m_spatialIndex.getOrCreateGrid(gid);
        g.idleSince = {};

        if (g.activeCells <= 0)
        {
            for (auto& cbEv : m_eventHub.onGridActivated)
                cbEv(gid);
        }
    }

    void VisibilitySystem::unpinGrid(int gid)
    {
        if (gid < 0 || gid >= Terrain::TilesCount * Terrain::TilesCount)
            return;

        auto& refs = m_pinnedGridRefs[static_cast<std::size_t>(gid)];
        if (refs == 0)
            return;

        if (--refs != 0)
            return;

        auto* gp = m_spatialIndex.tryGetGrid(gid);
        if (!gp)
            return;

        auto& g = *const_cast<GridChunk*>(gp);
        if (g.activeCells <= 0 && !gridHasSubscriptions(g))
        {
            g.idleSince = std::chrono::steady_clock::now();
            for (auto& cbEv : m_eventHub.onGridDeactivated)
                cbEv(gid);
        }
    }

    bool VisibilitySystem::isGridPinned(int gid) const
    {
        if (gid < 0 || gid >= Terrain::TilesCount * Terrain::TilesCount)
            return false;

        return m_pinnedGridRefs[static_cast<std::size_t>(gid)] != 0;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Interest profiles
    //////////////////////////////////////////////////////////////////////////////////////////
    InterestProfile VisibilitySystem::buildInterestProfile(Object* obj) const
    {
        InterestProfile profile;

        profile.viewerSubscribeCells = m_config.defaultViewerRadius;
        profile.activatorSubscribeCells = m_config.defaultActivatorRadius;

        if (!obj)
            return profile;

        // Autonomous residency is orthogonal to viewer/activator roles. Seed it before
        // the role-specific early returns so temporary states such as possession do not
        // accidentally clear an autonomous spawn's base lifecycle policy.
        if (!obj->isPlayer())
        {
            if (obj->isTransporter() || obj->GetTransport())
            {
                profile.autonomous = true;
            }
            else if (Creature* creature = obj->ToCreature())
            {
                if (WorldMap* map = creature->getWorldMap())
                    profile.autonomous = map->getSpawnManager().isAutonomousSpawn(creature->getGuid());
            }
        }

        if (obj->isPlayer())
        {
            profile.viewer = true;
            profile.activator = true;
            profile.publishMode = PublishMode::CellRadius;
            profile.publishCells = m_config.playerPublishCells;
            profile.publishPlayersOnly = true;
            return profile;
        }

        /// A player-controlled creature/pet is a real remote viewpoint. Keep this
        /// in the canonical interest profile instead of relying only on a temporary
        /// setViewerRole()/setActivatorRole() override; any later visibility refresh
        /// must preserve the remote viewer while possession/Eyes of the Beast is active.
        if (Unit* unit = obj->ToUnit(); unit && unit->m_playerControler)
        {
            profile.viewer = true;
            profile.activator = true;
            return profile;
        }

        if (obj->isTransporter())
        {
            profile.autonomous = true;
            profile.publishMode = PublishMode::GridWide;
            profile.publishCells = 0;
            profile.publishPlayersOnly = true;
            return profile;
        }

        if (obj->isGameObject())
        {
            GameObject* gameObject = obj->ToGameObject();
            const uint32_t overrides = gameObject->GetOverrides();
            GameObjectProperties const* properties = gameObject->GetGameObjectProperties();

            // The legacy MAPWIDE override is intentionally bounded to the visibility
            // system's grid-wide publish range instead of broadcasting across the map.
            if ((overrides & GAMEOBJECT_MAPWIDE) ||
                (properties && properties->isInfiniteGameObject()))
            {
                profile.publishMode = PublishMode::GridWide;
                profile.publishCells = 0;
                profile.publishPlayersOnly = true;
                return profile;
            }

            if ((overrides & GAMEOBJECT_AREAWIDE) ||
                (properties && properties->isLargeGameObject()))
            {
                profile.publishMode = PublishMode::CellRadius;
                profile.publishCells = std::max(m_config.largeGameObjectPublishCells,
                    m_config.defaultViewerRadius + 1);
                profile.publishPlayersOnly = true;
                return profile;
            }

            if (gameObject->getGoType() == GAMEOBJECT_TYPE_TRANSPORT ||
                gameObject->getGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT ||
                gameObject->getGoType() == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
            {
                profile.publishMode = PublishMode::GridWide;
                profile.publishCells = 0;
                profile.publishPlayersOnly = true;
                return profile;
            }
        }

        if (auto* dyn = dynamic_cast<DynamicObject*>(obj))
        {
            if (dyn->getDynamicType() == DYNAMIC_OBJECT_FARSIGHT_FOCUS)
            {
                profile.viewer = true;
                profile.activator = true;
                return profile;
            }
        }

        if (obj->isCorpse())
        {
            profile.activator = true;
            return profile;
        }

        return profile;
    }

    void VisibilitySystem::applyInterestProfile(ObjectHandle who, const InterestProfile& profile)
    {
        ObjectSlot* s = nullptr;
        if (!m_spatialIndex.tryGet(who, s))
            return;

        InterestProfile next = profile;
        next.viewerSubscribeCells = std::max(0, next.viewerSubscribeCells);
        next.activatorSubscribeCells = std::max(0, next.activatorSubscribeCells);
        next.publishCells = std::max(0, next.publishCells);

        if (next.publishMode == PublishMode::CellRadius && next.publishCells <= 0)
            next.publishMode = PublishMode::None;

        const auto sameProfile = [](const InterestProfile& a, const InterestProfile& b)
        {
            return a.viewer == b.viewer &&
                a.activator == b.activator &&
                a.autonomous == b.autonomous &&
                a.viewerSubscribeCells == b.viewerSubscribeCells &&
                a.activatorSubscribeCells == b.activatorSubscribeCells &&
                a.publishMode == b.publishMode &&
                a.publishCells == b.publishCells &&
                a.publishPlayersOnly == b.publishPlayersOnly;
        };

        const bool expectedPublishActive =
            next.publishMode == PublishMode::GridWide ||
            (next.publishMode == PublishMode::CellRadius && next.publishCells > 0);

        const int currentGrid = SpatialIndex::packGridFromPos(s->pos);
        const GridChunk* currentGridPtr = m_spatialIndex.tryGetGrid(currentGrid);
        const bool autonomousResident = currentGridPtr && currentGridPtr->autonomousResidents.contains(who);

        const bool runtimeMatches =
            s->sub.viewer == next.viewer &&
            s->sub.activator == next.activator &&
            autonomousResident == next.autonomous &&
            (!next.viewer || s->sub.radius == next.viewerSubscribeCells || next.activator) &&
            (!next.activator || s->sub.radius == next.activatorSubscribeCells || next.viewer) &&
            isPublishActive(s->pub) == expectedPublishActive &&
            (!expectedPublishActive ||
                (s->pub.mode == next.publishMode &&
                 s->pub.extraCells == next.publishCells &&
                 s->pub.playersOnly == next.publishPlayersOnly));

        if (sameProfile(s->interest, next) && runtimeMatches)
            return;

        /// Rebuild changed profiles so old roles cannot stay active.
        if (isPublishActive(s->pub))
            unpublish(who);

        if (s->sub.active)
            unsubscribe(who);

        /// Reacquire the slot after changing its runtime state.
        if (!m_spatialIndex.tryGet(who, s))
            return;

        setAutonomousResidency(who, next.autonomous);
        if (!m_spatialIndex.tryGet(who, s))
            return;

        s->interest = next;

        if (next.viewer)
            subscribeViewer(who, next.viewerSubscribeCells);

        if (next.activator)
            subscribeActivator(who, next.activatorSubscribeCells);

        if (expectedPublishActive)
            publish(who, next.publishCells, next.publishPlayersOnly);

        for (auto& cb : m_eventHub.onInterestProfileChanged)
            cb(s->meta.guid, next);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Publishing (public API)
    //////////////////////////////////////////////////////////////////////////////////////////
    void VisibilitySystem::publish(ObjectHandle who, int extraCells, bool playersOnly)
    {
        ObjectSlot* s = nullptr;
        if (!m_spatialIndex.tryGet(who, s))
            return;

        const bool wasActive = isPublishActive(s->pub);
        s->pub.extraCells = std::max(0, extraCells);
        s->pub.playersOnly = playersOnly;
        s->pub.mode = s->interest.publishMode != PublishMode::None ? s->interest.publishMode : PublishMode::CellRadius;

        if (s->pub.mode == PublishMode::CellRadius && s->pub.extraCells <= 0)
            s->pub.mode = PublishMode::None;

        if (!wasActive && isPublishActive(s->pub))
        {
            addPublishBucket(who, *s);
            notifyPublished(who, /*isSpawn*/true);
        }
        else if (wasActive && isPublishActive(s->pub))
        {
            removePublishBucket(who);
            addPublishBucket(who, *s);
        }
        else if (wasActive)
        {
            removePublishBucket(who);
        }
    }

    void VisibilitySystem::unpublish(ObjectHandle who)
    {
        ObjectSlot* s = nullptr;
        if (!m_spatialIndex.tryGet(who, s))
            return;

        if (isPublishActive(s->pub))
        {
            notifyPublished(who, /*isSpawn*/false);
            removePublishBucket(who);
        }
        s->pub = {};
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Tick
    //////////////////////////////////////////////////////////////////////////////////////////
    void VisibilitySystem::tick(std::chrono::milliseconds /*dt*/)
    {
        /// Idle/unload grids after extended inactivity.
        /// This scan is O(gridCount), so run it periodically instead of every map tick.
        auto now = std::chrono::steady_clock::now();
        if (m_lastGridUnloadScan.time_since_epoch().count() == 0 ||
            now - m_lastGridUnloadScan >= std::chrono::seconds(1))
        {
            m_lastGridUnloadScan = now;

            for (auto const& [gid, grid] : m_spatialIndex.gridPointersSnapshot())
            {
                if (!grid)
                    continue;

                auto& g = *grid;

                if (gridIsCompletelyEmpty(g))
                {
                    /// Keep sparse GridChunk nodes alive so cached grid pointers stay stable
                    /// for the lifetime of this WorldMap. There are at most
                    /// Terrain::TilesCount^2 grid chunks.
                    g.idleSince = {};
                    continue;
                }

                if (g.activeCells <= 0 &&
                    !isGridPinned(gid) &&
                    !gridHasSubscriptions(g) &&
                    g.idleSince.time_since_epoch().count() != 0 &&
                    now - g.idleSince >= m_config.cellUnloadDelay)
                {
                    const auto idleFor = now - g.idleSince;

                    /// A grid must never be unloaded while a viewer/activator still has
                    /// a subscription into it. Physical objects alone do not keep it
                    /// active and are removed by the map/spawn unload callback below.
                    for (auto& cb : m_eventHub.onGridUnload)
                        cb(gid);

                    if (g.activeCells > 0 ||
                        gridHasSubscriptions(g))
                    {
                        /// A callback or command reactivated/re-subscribed the grid.
                        /// Do not let the unload invalidate objects still referenced by
                        /// the visibility ring.
                        g.idleSince = {};
                    }
                    else if (!gridHasPhysicalObjects(g) || gridHasOnlyAutonomousResidents(g))
                    {
                        /// The normal grid content is fully unloaded. Autonomous residents
                        /// are allowed to remain attached and keep the structural grid alive
                        /// without making the grid active or causing repeated unload passes.
                        g.idleSince = {};
                        sLogger.debug("vis: UNLOAD grid={} activeCells=0 residentOnly={} idleFor={}ms",
                            gid,
                            gridHasPhysicalObjects(g) ? 1 : 0,
                            std::chrono::duration_cast<std::chrono::milliseconds>(idleFor).count());
                    }
                    else
                    {
                        /// Unload callback ran but some physical objects still remain.
                        /// Retry after the delay instead of spamming unload callbacks.
                        g.idleSince = now;
                        sLogger.warning("vis: UNLOAD incomplete grid={} activeCells=0 owners=1 idleFor={}ms",
                            gid,
                            std::chrono::duration_cast<std::chrono::milliseconds>(idleFor).count());
                    }
                }
            }
        }
    }



    //////////////////////////////////////////////////////////////////////////////////////////
    /// Subscription helpers (internal)
    //////////////////////////////////////////////////////////////////////////////////////////
    void VisibilitySystem::subscribeRing(ObjectHandle who, int baseGid, int lcx, int lcy, int r, bool asPlayer)
    {
        m_spatialIndex.forEachRingCell(baseGid, lcx, lcy, r, [&](int gid, int lcid)
        {
            cellSubscribe(who, gid, lcid, asPlayer);
        });
    }

    void VisibilitySystem::unsubscribeRing(ObjectHandle who, int baseGid, int lcx, int lcy, int r, bool asPlayer)
    {
        m_spatialIndex.forEachRingCell(baseGid, lcx, lcy, r, [&](int gid, int lcid)
        {
            cellUnsubscribe(who, gid, lcid, asPlayer);
        });

        if (asPlayer)
        {
            const ObjectSlot* s = nullptr;
            if (m_spatialIndex.tryGet(who, s) && s)
            {
                const uint64_t vraw = s->meta.guid.getRawGuid();
                if (auto it = m_visibleNow.find(vraw); it != m_visibleNow.end())
                {
                    for (uint64_t oraw : it->second)
                    {
                        if (auto jt = m_seenBy.find(oraw); jt != m_seenBy.end())
                        {
                            jt->second.erase(vraw);
                            if (jt->second.empty())
                                m_seenBy.erase(jt);
                        }
                    }
                    m_visibleNow.erase(it);
                }
            }
        }
    }

    void VisibilitySystem::cellSubscribe(ObjectHandle who, int gid, int lcid, bool asPlayer)
    {
        auto& g = m_spatialIndex.getOrCreateGrid(gid);
        auto& cb = m_spatialIndex.getOrCreateCell(g, lcid);

        bool activatorsWasEmpty = false;
        bool added = false;
        bool activatorAdded = false;

        {
            auto& list = asPlayer ? cb.viewers : cb.activators;

            if (!asPlayer)
                activatorsWasEmpty = cb.activators.empty();

            const auto oldSize = list.size();
            addUnique(list, who);
            added = list.size() != oldSize;
            activatorAdded = !asPlayer && added;
        }

        if (added)
            ++m_performanceCounters.ringSubscribes;

        if (activatorAdded && activatorsWasEmpty)
        {
            const int before = g.activeCells++;
            if (before == 0)
            {
                g.idleSince = {};

                if (!isGridPinned(gid))
                {
                    for (auto& cbEv : m_eventHub.onGridActivated)
                        cbEv(gid);
                }
            }
        }
    }

    void VisibilitySystem::cellUnsubscribe(ObjectHandle who, int gid, int lcid, bool asPlayer)
    {
        auto* gp = m_spatialIndex.tryGetGrid(gid);
        if (!gp)
            return;
        auto* cp = m_spatialIndex.tryGetCell(*gp, lcid);
        if (!cp)
            return;

        auto& g = *const_cast<GridChunk*>(gp);
        auto& cb = *const_cast<CellChunk*>(cp);

        bool activatorsWasNonEmpty = false;
        bool removed = false;
        bool activatorsNowEmpty = false;

        {
            auto& list = asPlayer ? cb.viewers : cb.activators;

            if (!asPlayer)
                activatorsWasNonEmpty = !cb.activators.empty();

            const auto oldSize = list.size();
            list.erase(std::remove(list.begin(), list.end(), who), list.end());
            removed = list.size() != oldSize;

            if (!asPlayer)
                activatorsNowEmpty = cb.activators.empty();
        }

        if (removed)
            ++m_performanceCounters.ringUnsubscribes;

        if (!asPlayer && removed && activatorsWasNonEmpty && activatorsNowEmpty)
        {
            const int after = --g.activeCells;
            if (after <= 0)
            {
                if (after < 0)
                {
                    sLogger.warning("vis: activeCells underflow on grid {}, resetting to 0", gid);
                    g.activeCells = 0;
                }

                if (isGridPinned(gid))
                {
                    g.idleSince = {};
                }
                else
                {
                    /// Physical spawned objects stay until the delayed grid unload
                    /// callback removes them.
                    g.idleSince = std::chrono::steady_clock::now();
                    for (auto& cbEv : m_eventHub.onGridDeactivated)
                        cbEv(gid);
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Publishing implementation
    //////////////////////////////////////////////////////////////////////////////////////////
    void VisibilitySystem::emitPublishedForViewer(ObjectHandle viewerH, bool isSubscribe)
    {
        const ObjectSlot* viewer = nullptr;
        if (!m_spatialIndex.tryGet(viewerH, viewer))
            return;
        if (!viewer->sub.active || !viewer->sub.viewer)
            return;

        const bool viewerIsPlayer = isPlayerBackedViewer(viewer);

        auto processCandidate = [&](ObjectHandle objH)
        {
            const ObjectSlot* obj = nullptr;
            if (!m_spatialIndex.tryGet(objH, obj))
                return;
            if (!isPublishActive(obj->pub))
                return;
            if (obj->pub.playersOnly && !viewerIsPlayer)
                return;

            const int objG = SpatialIndex::packGridFromPos(obj->pos);
            const int objL = SpatialIndex::packCellFromPos(obj->pos);
            const int dist = cellChebDistGlobal(viewer->sub.gid, viewer->sub.lcx, viewer->sub.lcy, objG, objL);
            if (dist <= viewer->sub.radius)
                return; // normal ring handles this pair

            if (isSubscribe)
                updateVisibilityPair(viewerH, objH);
            else
                emitHiddenOnce(viewer->meta.guid, obj->meta.guid);
        };

        const auto stamp = nextCandidateQueryStamp();

        auto processOnce = [&](ObjectHandle objH)
        {
            if (!markCandidateOnce(objH, stamp))
                return;
            processCandidate(objH);
        };

        /// Player publishers are queried locally instead of from a global list.
        if (viewerIsPlayer)
        {
            std::vector<ObjectHandle> players;
            players.reserve(128);
            const int playerPublishCells = m_config.playerPublishCells;
            collectPlayerPublishersAroundCell(viewer->sub.gid, viewer->sub.lcx, viewer->sub.lcy,
                playerPublishCells, players, stamp);
            for (auto objH : players)
                processCandidate(objH);
        }

        /// Rare custom publishers are kept in small buckets.
        for (auto objH : m_gridWidePublishers)
            processOnce(objH);

        for (auto objH : m_cellRadiusPublishers)
            processOnce(objH);
    }

    void VisibilitySystem::emitPublishedForPublisherMove(ObjectHandle pubH, int oldGid, int oldLcid, int newGid, int newLcid)
    {
        const ObjectSlot* obj = nullptr;
        if (!m_spatialIndex.tryGet(pubH, obj))
            return;
        if (!isPublishActive(obj->pub))
            return;

        const bool playersOnly = obj->pub.playersOnly;

        const int C = Cell::CellsPerTile;
        const int publishCells = (obj->pub.mode == PublishMode::GridWide)
            ? m_config.gridWidePublishCells
            : obj->pub.extraCells;
        const int gridsRadius = (publishCells + C - 1) / C;

        auto collectViewersAround = [&](int baseGid, std::uint64_t stamp, std::vector<ObjectHandle>& out)
            {
                auto [bgx, bgy] = unpackGridId(baseGid);
                for (int dgy = -gridsRadius; dgy <= gridsRadius; ++dgy)
                {
                    for (int dgx = -gridsRadius; dgx <= gridsRadius; ++dgx)
                    {
                        const int gx = std::clamp(bgx + dgx, 0, Terrain::TilesCount - 1);
                        const int gy = std::clamp(bgy + dgy, 0, Terrain::TilesCount - 1);
                        const int gid = packGridId(gx, gy);

                        const GridChunk* gp = m_spatialIndex.tryGetGrid(gid);
                        if (!gp)
                            continue;
                        for (auto vh : gp->owners[static_cast<std::size_t>(Container::Players)])
                            if (markCandidateOnce(vh, stamp))
                                out.push_back(vh);
                    }
                }
            };

        const auto stamp = nextCandidateQueryStamp();
        std::vector<ObjectHandle> viewers;
        viewers.reserve(256);
        collectViewersAround(oldGid, stamp, viewers);
        collectViewersAround(newGid, stamp, viewers);

        for (auto vh : viewers)
        {
            const ObjectSlot* viewer = nullptr;
            if (!m_spatialIndex.tryGet(vh, viewer))
                continue;
            if (!viewer->sub.active || !viewer->sub.viewer)
                continue;
            if (playersOnly && !isPlayerBackedViewer(viewer))
                continue;

            const auto inRing = [&](int d) { return d <= viewer->sub.radius; };
            const auto inPub = [&](int gid, int d)
            {
                if (inRing(d))
                    return false;

                if (obj->pub.mode == PublishMode::GridWide)
                    return d <= m_config.gridWidePublishCells;

                return obj->pub.extraCells > 0 && d <= std::max(viewer->sub.radius, obj->pub.extraCells);
            };

            const int oldDist = cellChebDistGlobal(viewer->sub.gid, viewer->sub.lcx, viewer->sub.lcy, oldGid, oldLcid);
            const int newDist = cellChebDistGlobal(viewer->sub.gid, viewer->sub.lcx, viewer->sub.lcy, newGid, newLcid);

            const bool oldPub = inPub(oldGid, oldDist);
            const bool newPub = inPub(newGid, newDist);
            const bool newRing = inRing(newDist);
            const bool oldRing = inRing(oldDist);

            if (!oldPub && !oldRing && newPub)
            {
                emitVisibleOnce(viewer->meta.guid, obj->meta.guid);
                continue;
            }
            if (oldPub && !newPub && !newRing)
            {
                emitHiddenOnce(viewer->meta.guid, obj->meta.guid);
                continue;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Notifications (spawn/despawn/move/publish)
    //////////////////////////////////////////////////////////////////////////////////////////
    void VisibilitySystem::notifySpawn(ObjectHandle h)
    {
        std::vector<ObjectHandle> viewers;
        collectCandidateViewersForObject(h, viewers);

        for (auto viewerH : viewers)
            updateVisibilityPair(viewerH, h);
    }

    void VisibilitySystem::notifyDespawn(ObjectHandle h)
    {
        const ObjectSlot* obj = nullptr;
        if (!m_spatialIndex.tryGet(h, obj))
            return;

        std::vector<ObjectHandle> viewers;
        collectCandidateViewersForObject(h, viewers);

        for (auto viewerH : viewers)
        {
            const ObjectSlot* viewer = nullptr;
            if (!m_spatialIndex.tryGet(viewerH, viewer))
                continue;

            emitHiddenOnce(viewer->meta.guid, obj->meta.guid);
        }
    }

    void VisibilitySystem::notifyPublished(ObjectHandle objH, bool isSpawn)
    {
        const ObjectSlot* obj = nullptr;
        if (!m_spatialIndex.tryGet(objH, obj))
            return;
        if (!isPublishActive(obj->pub))
            return;

        std::vector<ObjectHandle> viewers;
        collectCandidateViewersForObject(objH, viewers);

        for (auto viewerH : viewers)
        {
            if (isSpawn)
            {
                updateVisibilityPair(viewerH, objH);
            }
            else
            {
                const ObjectSlot* viewer = nullptr;
                if (!m_spatialIndex.tryGet(viewerH, viewer))
                    continue;
                emitHiddenOnce(viewer->meta.guid, obj->meta.guid);
            }
        }
    }

    void VisibilitySystem::notifyCrossCellMove(ObjectHandle objH, int oldGid, int oldLcid, int newGid, int newLcid)
    {
        if (oldGid == newGid && oldLcid == newLcid)
            return;

        const ObjectSlot* obj = nullptr;
        if (!m_spatialIndex.tryGet(objH, obj))
            return;

        const auto stamp = nextCandidateQueryStamp();

        auto processCellViewers = [&](int gid, int lcid)
        {
            const GridChunk* grid = m_spatialIndex.tryGetGrid(gid);
            if (!grid)
                return;

            const CellChunk* cell = m_spatialIndex.tryGetCell(*grid, lcid);
            if (!cell)
                return;
            for (const auto viewerH : cell->viewers)
            {
                if (!markCandidateOnce(viewerH, stamp))
                    continue;

                const ObjectSlot* viewer = nullptr;
                if (!m_spatialIndex.tryGet(viewerH, viewer) || !viewer->native || !viewer->sub.active || !viewer->sub.viewer)
                    continue;

                updateVisibilityPair(viewerH, objH);
            }
        };

        processCellViewers(oldGid, oldLcid);
        processCellViewers(newGid, newLcid);
    }

    void VisibilitySystem::reconcilePublishedForViewerMove(ObjectHandle viewerH,
        int oldGid, int oldLcid, int newGid, int newLcid)
    {
        const ObjectSlot* viewer = nullptr;
        if (!m_spatialIndex.tryGet(viewerH, viewer) || !viewer->sub.active || !viewer->sub.viewer)
            return;

        const bool viewerIsPlayer = isPlayerBackedViewer(viewer);
        const auto stamp = nextCandidateQueryStamp();

        auto processOnce = [&](ObjectHandle objH)
        {
            if (!markCandidateOnce(objH, stamp))
                return;
            reconcilePublishedCandidateForViewerMove(viewerH, *viewer, objH, oldGid, oldLcid, newGid, newLcid);
        };

        if (viewerIsPlayer)
        {
            auto [oldLcx, oldLcy] = unpackCellId2(oldLcid);
            auto [newLcx, newLcy] = unpackCellId2(newLcid);
            const int playerPublishCells = m_config.playerPublishCells;

            std::vector<ObjectHandle> players;
            players.reserve(128);
            collectPlayerPublishersAroundCell(oldGid, oldLcx, oldLcy, playerPublishCells, players, stamp);
            collectPlayerPublishersAroundCell(newGid, newLcx, newLcy, playerPublishCells, players, stamp);

            for (auto objH : players)
                reconcilePublishedCandidateForViewerMove(viewerH, *viewer, objH, oldGid, oldLcid, newGid, newLcid);
        }

        for (auto objH : m_gridWidePublishers)
            processOnce(objH);

        for (auto objH : m_cellRadiusPublishers)
            processOnce(objH);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Emit visible/hidden (deduped)
    //////////////////////////////////////////////////////////////////////////////////////////
    void VisibilitySystem::emitVisibleOnce(const WoWGuid& viewer, const WoWGuid& object)
    {
        bool shouldEmit = false;
        {
            const uint64_t vraw = viewer.getRawGuid();
            const uint64_t oraw = object.getRawGuid();
            auto& set = m_visibleNow[vraw];
            if (set.insert(oraw).second)
            {
                m_seenBy[oraw].insert(vraw);
                shouldEmit = true;
            }
        }

        if (shouldEmit)
        {
            for (auto& f : m_eventHub.onVisible)
                f(viewer, object);
        }
    }

    void VisibilitySystem::emitHiddenOnce(const WoWGuid& viewer, const WoWGuid& object)
    {
        bool shouldEmit = false;
        {
            const uint64_t vraw = viewer.getRawGuid();
            const uint64_t oraw = object.getRawGuid();
            if (auto it = m_visibleNow.find(vraw); it != m_visibleNow.end())
                shouldEmit = it->second.erase(oraw) > 0;

            if (shouldEmit)
            {
                if (auto jt = m_seenBy.find(oraw); jt != m_seenBy.end())
                {
                    jt->second.erase(vraw);
                    if (jt->second.empty())
                        m_seenBy.erase(jt);
                }
            }
        }

        if (shouldEmit)
        {
            for (auto& f : m_eventHub.onHidden)
                f(viewer, object);
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Proximity sweeps
    //////////////////////////////////////////////////////////////////////////////////////////
    void VisibilitySystem::proximitySweepViewer(ObjectHandle viewerH)
    {
        const ObjectSlot* vs = nullptr;
        if (!m_spatialIndex.tryGet(viewerH, vs))
            return;
        if (!vs->sub.active || !vs->sub.viewer)
            return;

        const int baseGid = vs->sub.gid;
        const int lcx = vs->sub.lcx;
        const int lcy = vs->sub.lcy;
        const int R = vs->sub.radius;

        Unit* viewerUnit = resolveVisibilityRuleUnit(vs);
        auto* viewpoint = static_cast<Object*>(vs->native);
        if (!viewerUnit || !viewpoint)
            return;

        const auto stamp = nextCandidateQueryStamp();

        m_spatialIndex.forEachRingCell(baseGid, lcx, lcy, R, [&](int gid, int lcid)
        {
            const GridChunk* gp = m_spatialIndex.tryGetGrid(gid); if (!gp) return;
            const CellChunk* cp = m_spatialIndex.tryGetCell(*gp, lcid); if (!cp) return;
            for (std::size_t ci = 0; ci < static_cast<std::size_t>(Container::Count); ++ci)
            {
                for (auto oh : cp->byContainer[ci])
                {
                    if (!markCandidateOnce(oh, stamp))
                        continue;

                    const ObjectSlot* os = nullptr;
                    if (!m_spatialIndex.tryGet(oh, os) || !os->native)
                        continue;

                    auto* obj = static_cast<Object*>(os->native);
                    const float publishedVisibilityDistanceSq = getPublishedVisibilityDistanceSq(*os);
                    const bool visibleNow = viewerUnit->canSeeFrom(obj, viewpoint, publishedVisibilityDistanceSq);

                    if (visibleNow)
            {
                emitVisibleOnce(vs->meta.guid, os->meta.guid);
            }
                    else            emitHiddenOnce(vs->meta.guid, os->meta.guid);
                }
            }
        });
    }

    void VisibilitySystem::proximityAffectViewersForObject(ObjectHandle objH)
    {
        std::vector<ObjectHandle> viewers;
        collectCandidateViewersForObject(objH, viewers);

        for (auto vh : viewers)
            updateVisibilityPair(vh, objH);
    }

    void VisibilitySystem::refreshObjectVisibility(ObjectHandle objH)
    {
        ObjectSlot* slot = nullptr;
        if (!m_spatialIndex.tryGet(objH, slot) || !slot->native)
            return;

        /// Refresh viewers that may be affected by this object.
        proximityAffectViewersForObject(objH);

        /// If the object is a viewer, refresh what it can see as well.
        if (slot->sub.active && slot->sub.viewer)
        {
            proximitySweepViewer(objH);
            emitPublishedForViewer(objH, /*isSubscribe*/true);
        }
    }


} /// namespace visibility
