/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <unordered_map>
#include <memory>
#include <list>
#include <algorithm>
#include <cmath>
#include <array>
#include <vector>
#include <chrono>
#include <cstdint>
#include "VisibilityTypes.hpp"
#include "SpatialIndex.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Objects/GameObject.h"
#include "Objects/Object.hpp"

class Unit;

//////////////////////////////////////////////////////////////////////////////////////////
/// Visibility namespace
//////////////////////////////////////////////////////////////////////////////////////////
namespace visibility
{
    //////////////////////////////////////////////////////////////////////////////////////////
    /// Aggregated visibility metrics snapshot. Intended for diagnostics and profiling.
    //////////////////////////////////////////////////////////////////////////////////////////
    struct VisibilityPerformanceCounters
    {
        std::uint64_t pairChecks = 0;
        std::uint64_t ringSubscribes = 0;
        std::uint64_t ringUnsubscribes = 0;
    };

    struct VisSnapshot
    {
        size_t grids = 0;                                        ///< number of allocated grid chunks
        size_t activeGrids = 0;                                  ///< grids active through activators or persistent pins
        size_t cells = 0;                                        ///< allocated cell chunks
        size_t activeCells = 0;                                  ///< active cell references from activators
        size_t poolLive = 0;                                     ///< number of live objects in the pool
        size_t guidIndex = 0;                                    ///< number of GUIDs indexed
        size_t cellViewers = 0;                                  ///< total handles in CellChunk::viewers
        size_t cellActivators = 0;                               ///< total handles in CellChunk::activators
        size_t visibleViewers = 0;                               ///< m_visibleNow viewer entries
        size_t visiblePairs = 0;                                 ///< total visible pairs
        size_t seenObjects = 0;                                  ///< m_seenBy object entries
        size_t seenPairs = 0;                                    ///< total reverse visible pairs
        size_t gridWidePublishers = 0;                           ///< grid-wide publisher bucket size
        size_t cellRadiusPublishers = 0;                         ///< custom cell-radius publisher bucket size
        size_t nearCacheGuidsCapacity = 0;                       ///< retained NearCache guid capacity on live slots
        size_t nearCacheStampsCapacity = 0;                      ///< retained NearCache stamp capacity on live slots
        size_t nearCacheCachedGuids = 0;                         ///< currently cached near guid count
        std::array<size_t, static_cast<size_t>(visibility::Container::Count)> owners{}; ///< per-owner counters
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Visibility manager for a single map instance. Handles spatial indexing, subscriptions
    /// (viewers/activators), publishing, and event emission. Thread ownership: map thread unless
    /// otherwise stated (see Drain/FlushPendingMoves).
    //////////////////////////////////////////////////////////////////////////////////////////
    class VisibilitySystem
    {
    public:

        explicit VisibilitySystem(SpatialIndex& spatialIndex, Config cfg = {});
        ~VisibilitySystem() = default;

        VisibilitySystem(const VisibilitySystem&) = delete;
        VisibilitySystem& operator=(const VisibilitySystem&) = delete;
        VisibilitySystem(VisibilitySystem&&) = delete;
        VisibilitySystem& operator=(VisibilitySystem&&) = delete;

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Metrics
        //////////////////////////////////////////////////////////////////////////////////////////
        /// Returns a diagnostic snapshot of relevant metrics.
        VisSnapshot snapshot() const;
        /// Logs the diagnostic snapshot with a reason label.
        void logMemoryDiagnostics(const char* reason) const;
        VisibilityPerformanceCounters takePerformanceCounters() noexcept
        {
            const auto counters = m_performanceCounters;
            m_performanceCounters = {};
            return counters;
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Object spatial lifecycle events
        //////////////////////////////////////////////////////////////////////////////////////////
        /// Called after SpatialIndex::addObject() inserted an object into the spatial index.
        void onObjectAdded(ObjectHandle h);
        /// Called before SpatialIndex::removeObject() removes an object from the spatial index.
        void onObjectRemoving(ObjectHandle h);
        /// Called after SpatialIndex::moveObject() updated position/cell membership.
        void onObjectMoved(ObjectHandle h, const SpatialMoveResult& move);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Visibility subscriptions
        //////////////////////////////////////////////////////////////////////////////////////////
        /// Subscribes a viewer to a ring of cells around its current position.
        void subscribeViewer(ObjectHandle who, int radiusCells);
        /// Subscribes an activator (non-player publisher) to a ring of cells.
        void subscribeActivator(ObjectHandle who, int radiusCells);
        /// Removes any subscription of this handle.
        void unsubscribe(ObjectHandle who);

        /// Runtime role changes. These are used by possess/unpossess, vehicles,
        /// scripts, and any later system that turns an already-spawned object into
        /// a viewer or activator without reattaching it to the map.
        void setViewerRole(ObjectHandle who, bool enabled, int radiusCells);
        void setActivatorRole(ObjectHandle who, bool enabled, int radiusCells);

        /// Triggers one activation/deactivation cycle for a grid.
        void activateGrid(int gid);

        /// Persistent grid activation. Multiple users may pin the same grid; every
        /// pin must be paired with an unpin before the grid can become idle again.
        void pinGrid(int gid);
        void unpinGrid(int gid);
        bool isGridPinned(int gid) const;

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Interest profiles
        //////////////////////////////////////////////////////////////////////////////////////////
        /// Updates the default viewer/activator radius used by newly attached objects.
        void setDefaultSubscriptionRadius(int radiusCells) noexcept;
        /// Builds the default interest profile for an object type.
        InterestProfile buildInterestProfile(Object* obj) const;
        /// Applies viewer/activator subscriptions and publishing for an object.
        void applyInterestProfile(ObjectHandle who, const InterestProfile& profile);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Visibility publishing
        //////////////////////////////////////////////////////////////////////////////////////////
        /// Publishes an object to viewers within the larger of the normal viewer radius and extraCells.
        void publish(ObjectHandle who, int extraCells, bool playersOnly = true);
        /// Stops publishing the given object.
        void unpublish(ObjectHandle who);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Tick
        //////////////////////////////////////////////////////////////////////////////////////////
        /// Advances internal time and performs budgeted work.
        void tick(std::chrono::milliseconds dt);

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Events (callbacks will be executed on the map thread)
        //////////////////////////////////////////////////////////////////////////////////////////
        void onBecameVisible(VisibleCb cb) { m_eventHub.onVisible.push_back(std::move(cb)); }
        void onBecameHidden(HiddenCb cb) { m_eventHub.onHidden.push_back(std::move(cb)); }
        void onGridChanged(GridMoveCb cb) { m_eventHub.onGridChanged.push_back(std::move(cb)); }
        void onGridActivated(GridEventCb cb) { m_eventHub.onGridActivated.push_back(std::move(cb)); }
        void onGridDeactivated(GridEventCb cb) { m_eventHub.onGridDeactivated.push_back(std::move(cb)); }
        void onGridUnload(GridEventCb cb) { m_eventHub.onGridUnload.push_back(std::move(cb)); }

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Index helpers
        //////////////////////////////////////////////////////////////////////////////////////////
        /// Rebuilds viewer proximity based on current position.
        void proximitySweepViewer(ObjectHandle viewerH);
        /// Reconciles viewers affected by a specific object's movement.
        void proximityAffectViewersForObject(ObjectHandle objH);
        /// Re-evaluates visibility for an object after non-positional visibility state changes
        /// such as stealth, invisibility, death state, GM invis, faction/phase changes, etc.
        void refreshObjectVisibility(ObjectHandle objH);

        /// Collect all viewers currently seeing an object.
        void collectViewersOf(const WoWGuid& object, std::vector<WoWGuid>& out) const;
        /// Collect all objects currently visible for a viewer. Used when a temporary
        /// viewer source such as possession/farsight is removed and the recipient
        /// player's client-side visible cache must be cleaned explicitly.
        void collectVisibleObjectsForViewer(const WoWGuid& viewer, std::vector<WoWGuid>& out) const;
        /// Removes all tracked visible objects for a viewer and returns the removed
        /// object GUIDs. Used for hard client visibility resets on same-map
        /// teleports/repop such as Release Spirit.
        void clearVisibleObjectsForViewer(const WoWGuid& viewer, std::vector<WoWGuid>& out);

        /// Clear a viewer before a same-map relocation.
        void resetViewerForRelocation(ObjectHandle who, std::vector<WoWGuid>& out);

    private:
        Config m_config{};
        SpatialIndex& m_spatialIndex;
        VisibilityPerformanceCounters m_performanceCounters{};
        EventHub m_eventHub{};

        /// Rare non-player publishers are kept in small buckets.
        std::vector<ObjectHandle> m_gridWidePublishers;
        std::vector<ObjectHandle> m_cellRadiusPublishers;

        /// Scratch stamp used to dedupe candidates without allocations.
        mutable std::uint64_t m_candidateQueryStamp{ 0 };

        /// Grid unload checks only need to run from time to time.
        std::chrono::steady_clock::time_point m_lastGridUnloadScan{};

        /// References that keep scripted grids active.
        std::array<uint32_t, static_cast<std::size_t>(Terrain::TilesCount * Terrain::TilesCount)> m_pinnedGridRefs{};

        /// Subscriptions
        void subscribeRing(ObjectHandle who, int gid, int lcx, int lcy, int r, bool asPlayer);
        void unsubscribeRing(ObjectHandle who, int baseGid, int lcx, int lcy, int r, bool asPlayer);
        void cellSubscribe(ObjectHandle who, int gid, int lcid, bool asPlayer);
        void cellUnsubscribe(ObjectHandle who, int gid, int lcid, bool asPlayer);

        /// Publish
        void emitPublishedForViewer(ObjectHandle viewerH, bool isSubscribe);
        void emitPublishedForPublisherMove(ObjectHandle pubH, int oldGid, int oldLcid, int newGid, int newLcid);

        bool isPlayerSpatialPublisher(const ObjectSlot& obj) const;
        bool isBucketedPublisher(const ObjectSlot& obj) const;
        void addPublishBucket(ObjectHandle who, const ObjectSlot& obj);
        void removePublishBucket(ObjectHandle who);
        bool isInPublishBucket(ObjectHandle who) const;
        bool hasCellSubscription(ObjectHandle who) const;
        std::uint64_t nextCandidateQueryStamp() const;
        bool markCandidateOnce(ObjectHandle h, std::uint64_t stamp) const;
        void collectPlayerPublishersAroundCell(int gid, int lcx, int lcy, int radiusCells,
            std::vector<ObjectHandle>& out, std::uint64_t stamp) const;
        void reconcilePublishedCandidateForViewerMove(ObjectHandle viewerH, const ObjectSlot& viewer, ObjectHandle objH,
            int oldGid, int oldLcid, int newGid, int newLcid);

        /// Notifications
        void notifySpawn(ObjectHandle h);
        void notifyDespawn(ObjectHandle h);
        void notifyPublished(ObjectHandle objH, bool isSpawn);
        void notifyCrossCellMove(ObjectHandle objH, int oldGid, int oldLcid, int newGid, int newLcid);

        void reconcilePublishedForViewerMove(ObjectHandle viewerH, int oldGid, int oldLcid, int newGid, int newLcid);

        /// Central visibility rule / candidate helpers
        bool isWithinInterestReach(const ObjectSlot& viewer, const ObjectSlot& obj, int objGid, int objLcid) const;
        void updateVisibilityPair(ObjectHandle viewerH, ObjectHandle objH);
        void collectCandidateViewersForObject(ObjectHandle objH, std::vector<ObjectHandle>& out) const;

        /// Object update (emit once semantics)
        void emitVisibleOnce(const WoWGuid& viewer, const WoWGuid& object);
        void emitHiddenOnce(const WoWGuid& viewer, const WoWGuid& object);


        bool isPublishedVisibleFor(const ObjectSlot* obj, const ObjectSlot* viewer, int objGid, int objLcid) const;
        void hideObjectsInCellsForViewer(ObjectHandle viewerH, const std::vector<CellRef>& cells);
        void hideObjectFromKnownViewers(const WoWGuid& objectGuid);
        bool isVisibleToViewer(const ObjectSlot* obj, const ObjectSlot* viewer, int objGid, int objLcid) const;
        float getPublishedVisibilityDistanceSq(const ObjectSlot& obj) const noexcept;
        Unit* resolveVisibilityRuleUnit(const ObjectSlot* viewer) const;
        bool isPlayerBackedViewer(const ObjectSlot* viewer) const;

        bool shouldRefreshViewerInterest(const ObjectSlot& viewer, const SpatialMoveResult& move) const;
        void markViewerInterestRefreshed(ObjectSlot& viewer, const LocationVector& pos);
        void refreshViewerInterest(ObjectHandle viewerH, ObjectSlot& viewer);

        bool shouldRefreshMovedObjectInterest(const ObjectSlot& object, const SpatialMoveResult& move) const;
        void markMovedObjectInterestRefreshed(ObjectSlot& object, const LocationVector& pos);
        void refreshMovedObjectInterest(ObjectHandle objectH, ObjectSlot& object);

        /// "visibleNow" tracking (viewer GUID -> set of object GUIDs)
        std::unordered_map<uint64_t, std::unordered_set<uint64_t>> m_visibleNow;
        /// reverse index (object GUID -> set of viewer GUIDs)
        std::unordered_map<uint64_t, std::unordered_set<uint64_t>> m_seenBy;
    };

} /// namespace visibility
