/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <shared_mutex>
#include <type_traits>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "VisibilityTypes.hpp"
#include "ObjectPool.hpp"

#include "Objects/Object.hpp"
#include "Objects/Units/Unit.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/GameObject.h"

namespace visibility
{
    struct SpatialPerformanceCounters
    {
        std::uint64_t queries = 0;
        std::uint64_t candidates = 0;
        std::uint64_t tryGets = 0;
        std::uint64_t moves = 0;
        std::uint64_t cellMoves = 0;
    };

    struct SpatialMoveResult
    {
        ObjectHandle handle{};
        bool valid{ false };
        bool moved{ false };
        bool cellChanged{ false };
        bool gridChanged{ false };

        int oldGrid{ 0 };
        int oldCell{ 0 };
        int newGrid{ 0 };
        int newCell{ 0 };

        LocationVector oldPos{};
        LocationVector newPos{};
    };

    class SpatialIndex
    {
    public:
        static constexpr std::size_t GridSlotCount = static_cast<std::size_t>(Terrain::TilesCount * Terrain::TilesCount);
        static constexpr std::size_t CellSlotCount = static_cast<std::size_t>(Cell::CellsPerTile * Cell::CellsPerTile);
        using GridStorage = std::array<std::unique_ptr<GridChunk>, GridSlotCount>;

        explicit SpatialIndex(std::size_t initialPoolCapacity = 0)
            : m_pool(initialPoolCapacity)
        {
        }

        SpatialIndex(const SpatialIndex&) = delete;
        SpatialIndex& operator=(const SpatialIndex&) = delete;
        SpatialIndex(SpatialIndex&&) = delete;
        SpatialIndex& operator=(SpatialIndex&&) = delete;

        /// Static spatial helpers.
        static int packGridFromPos(const LocationVector& p) noexcept { auto [gx, gy] = worldToGrid(p); return packGridId(gx, gy); }
        static int packCellFromPos(const LocationVector& p) noexcept { auto [cx, cy] = worldToLocal(p); return packCellId(cx, cy); }
        static int cellsForRadius(float radius) noexcept
        {
            if (radius <= 0.0f)
                return 0;

            return std::max(1, static_cast<int>(std::ceil(radius / Cell::Size)));
        }

        SpatialPerformanceCounters takePerformanceCounters() noexcept
        {
            const auto counters = m_performanceCounters;
            m_performanceCounters = {};
            return counters;
        }

        /// Spatial lifecycle. SpatialIndex is the owner of handle/slot/cell membership.
        ObjectHandle addObject(const ObjectMeta& meta, const LocationVector& pos, void* native = nullptr)
        {
            ObjectHandle h = m_pool.allocate();
            if (!h.id)
                return {};

            m_pool.init(h, meta, pos, native);
            attachToCell(h, pos);

            {
                std::unique_lock<std::shared_mutex> guard(m_guidToHandleMutex);
                m_guidToHandle[meta.guid.getRawGuid()] = h;
            }

            return h;
        }

        bool removeObject(ObjectHandle h)
        {
            ObjectSlot* slot = nullptr;
            if (!tryGet(h, slot))
                return false;

            const auto guid = slot->meta.guid.getRawGuid();
            detachFromCell(h, slot->pos);

            {
                std::unique_lock<std::shared_mutex> guard(m_guidToHandleMutex);
                m_guidToHandle.erase(guid);
            }

            m_pool.release(h);
            return true;
        }

        SpatialMoveResult moveObject(ObjectHandle h, const LocationVector& newPos)
        {
            SpatialMoveResult result;
            result.handle = h;

            ObjectSlot* slot = nullptr;
            if (!tryGet(h, slot))
                return result;

            ++m_performanceCounters.moves;
            result.valid = true;
            result.oldPos = slot->pos;
            result.newPos = newPos;
            result.oldGrid = packGridFromPos(result.oldPos);
            result.oldCell = packCellFromPos(result.oldPos);
            result.newGrid = packGridFromPos(newPos);
            result.newCell = packCellFromPos(newPos);
            result.gridChanged = result.oldGrid != result.newGrid;
            result.cellChanged = result.gridChanged || result.oldCell != result.newCell;
            if (result.cellChanged)
                ++m_performanceCounters.cellMoves;
            result.moved = true;

            slot->pos = newPos;
            slot->nearCache.invalidate();

            if (result.gridChanged)
                transferCell(h, result.oldGrid, result.newGrid, result.oldCell, result.newCell);
            else if (result.oldCell != result.newCell)
                reindexWithinGrid(h, result.oldGrid, result.oldCell, result.newCell);

            return result;
        }

        ObjectHandle handleByGuid(const WoWGuid& guid) const noexcept
        {
            /// Only the GUID lookup needs a lock during startup/map handoffs.
            std::shared_lock<std::shared_mutex> guard(m_guidToHandleMutex);
            auto it = m_guidToHandle.find(guid.getRawGuid());
            return it == m_guidToHandle.end() ? ObjectHandle{} : it->second;
        }

        /// Object handles resolve through this pool.
        ObjectHandle allocate() { return m_pool.allocate(); }
        bool release(ObjectHandle h) { return m_pool.release(h); }
        void init(ObjectHandle h, const ObjectMeta& meta, const LocationVector& pos, void* native = nullptr)
        {
            m_pool.init(h, meta, pos, native);
        }

        bool tryGet(ObjectHandle h, ObjectSlot*& out)
        {
            ++m_performanceCounters.tryGets;
            return m_pool.tryGet(h, out);
        }
        bool tryGet(ObjectHandle h, const ObjectSlot*& out) const
        {
            ++m_performanceCounters.tryGets;
            return m_pool.tryGet(h, out);
        }

        ObjectMeta& meta(ObjectHandle h) { return m_pool.meta(h); }
        const ObjectMeta& meta(ObjectHandle h) const { return m_pool.meta(h); }

        LocationVector& pos(ObjectHandle h) { return m_pool.pos(h); }
        const LocationVector& pos(ObjectHandle h) const { return m_pool.pos(h); }

        std::uint32_t liveCount() const { return m_pool.liveCount(); }
        std::uint32_t poolCapacity() const { return m_pool.capacity(); }

        void shrinkObjectPool(std::size_t minCapacity = 64, std::size_t maxFreeRatio = 4)
        {
            m_pool.shrinkToFit(minCapacity, maxFreeRatio);
        }

        std::size_t guidCount() const
        {
            std::shared_lock<std::shared_mutex> guard(m_guidToHandleMutex);
            return m_guidToHandle.size();
        }

        GridChunk& getOrCreateGrid(int gid)
        {
            if (gid < 0 || static_cast<std::size_t>(gid) >= GridSlotCount)
                throw std::out_of_range("SpatialIndex::getOrCreateGrid invalid grid id");
            auto& slot = m_grids[static_cast<std::size_t>(gid)];
            if (!slot)
            {
                slot = std::make_unique<GridChunk>();
                ++m_loadedGridCount;
            }
            return *slot;
        }

        GridChunk* tryGetGrid(int gid)
        {
            if (gid < 0 || static_cast<std::size_t>(gid) >= GridSlotCount)
                return nullptr;
            return m_grids[static_cast<std::size_t>(gid)].get();
        }

        const GridChunk* tryGetGrid(int gid) const
        {
            if (gid < 0 || static_cast<std::size_t>(gid) >= GridSlotCount)
                return nullptr;
            return m_grids[static_cast<std::size_t>(gid)].get();
        }

        CellChunk& getOrCreateCell(GridChunk& grid, int lcid)
        {
            if (lcid < 0 || static_cast<std::size_t>(lcid) >= CellSlotCount)
                throw std::out_of_range("SpatialIndex::getOrCreateCell invalid cell id");
            auto& slot = grid.cells[static_cast<std::size_t>(lcid)];
            if (!slot)
                slot = std::make_unique<CellChunk>();
            return *slot;
        }

        CellChunk* tryGetCell(GridChunk& grid, int lcid)
        {
            if (lcid < 0 || static_cast<std::size_t>(lcid) >= CellSlotCount)
                return nullptr;
            return grid.cells[static_cast<std::size_t>(lcid)].get();
        }

        const CellChunk* tryGetCell(const GridChunk& grid, int lcid) const
        {
            if (lcid < 0 || static_cast<std::size_t>(lcid) >= CellSlotCount)
                return nullptr;
            return grid.cells[static_cast<std::size_t>(lcid)].get();
        }

        std::vector<std::pair<int, GridChunk*>> gridPointersSnapshot()
        {
            std::vector<std::pair<int, GridChunk*>> out;
            out.reserve(m_loadedGridCount);
            for (std::size_t gid = 0; gid < m_grids.size(); ++gid)
            {
                if (m_grids[gid])
                    out.emplace_back(static_cast<int>(gid), m_grids[gid].get());
            }
            return out;
        }

        std::vector<std::pair<int, const GridChunk*>> gridPointersSnapshot() const
        {
            std::vector<std::pair<int, const GridChunk*>> out;
            out.reserve(m_loadedGridCount);
            for (std::size_t gid = 0; gid < m_grids.size(); ++gid)
            {
                if (m_grids[gid])
                    out.emplace_back(static_cast<int>(gid), m_grids[gid].get());
            }
            return out;
        }

        template<typename Fn>
        void forEachRingCell(int baseGid, int lcx, int lcy, int radiusCells, Fn&& fn) const
        {
            if (radiusCells < 0)
                return;

            constexpr int cellsPerGrid = Cell::CellsPerTile;
            constexpr int totalCellsPerAxis = Terrain::TilesCount * cellsPerGrid;

            auto [bgx, bgy] = unpackGridId(baseGid);
            const int centerX = bgx * cellsPerGrid + lcx;
            const int centerY = bgy * cellsPerGrid + lcy;
            const int minX = centerX - radiusCells;
            const int maxX = centerX + radiusCells;
            const int minY = centerY - radiusCells;
            const int maxY = centerY + radiusCells;

            /// Normal gameplay is far from the world edge. Walk the global cell
            /// coordinates directly so the hot path needs no temporary 512-entry
            /// arrays, bitsets, sorting or heap allocations.
            if (minX >= 0 && maxX < totalCellsPerAxis &&
                minY >= 0 && maxY < totalCellsPerAxis)
            {
                for (int mappedY = minY; mappedY <= maxY; ++mappedY)
                {
                    const int gy = mappedY / cellsPerGrid;
                    const int cy = mappedY % cellsPerGrid;

                    for (int mappedX = minX; mappedX <= maxX; ++mappedX)
                    {
                        const int gx = mappedX / cellsPerGrid;
                        const int cx = mappedX % cellsPerGrid;
                        fn(packGridId(gx, gy), packCellId(cx, cy));
                    }
                }
                return;
            }

            /// Preserve the historical map-edge behaviour: coordinates outside
            /// the map clamp to the edge grid while retaining their wrapped local
            /// cell coordinate. Edge traversal is rare, so keep this compatibility
            /// path simple and allocate only enough storage for the requested ring.
            auto collectEdgeAxis = [&](int center)
            {
                std::vector<int> cells;
                const int diameter = 2 * radiusCells + 1;
                cells.reserve(static_cast<std::size_t>(std::min(diameter, totalCellsPerAxis)));

                for (int delta = -radiusCells; delta <= radiusCells; ++delta)
                {
                    const int raw = center + delta;

                    int grid = raw / cellsPerGrid;
                    int local = raw % cellsPerGrid;
                    if (local < 0)
                    {
                        local += cellsPerGrid;
                        --grid;
                    }

                    grid = std::clamp(grid, 0, Terrain::TilesCount - 1);
                    const int mapped = grid * cellsPerGrid + local;

                    if (std::find(cells.begin(), cells.end(), mapped) == cells.end())
                        cells.push_back(mapped);
                }

                return cells;
            };

            const auto xCells = collectEdgeAxis(centerX);
            const auto yCells = collectEdgeAxis(centerY);

            for (const int mappedY : yCells)
            {
                const int gy = mappedY / cellsPerGrid;
                const int cy = mappedY % cellsPerGrid;

                for (const int mappedX : xCells)
                {
                    const int gx = mappedX / cellsPerGrid;
                    const int cx = mappedX % cellsPerGrid;
                    fn(packGridId(gx, gy), packCellId(cx, cy));
                }
            }
        }

        std::vector<CellRef> buildRingCells(int baseGid, int lcx, int lcy, int radiusCells) const
        {
            std::vector<CellRef> out;
            if (radiusCells < 0)
                return out;

            const int diameter = 2 * radiusCells + 1;
            out.reserve(static_cast<std::size_t>(diameter) * static_cast<std::size_t>(diameter));

            forEachRingCell(baseGid, lcx, lcy, radiusCells, [&](int gid, int lcid)
            {
                out.emplace_back(gid, lcid);
            });

            /// Keep the historical sorted contract for callers using set_difference.
            std::sort(out.begin(), out.end());
            return out;
        }


        //////////////////////////////////////////////////////////////////////////////////////////
        /// Spatial queries
        //////////////////////////////////////////////////////////////////////////////////////////
        void collectNearGuidsCached(const WoWGuid& centerGuid, int radiusCells, std::vector<WoWGuid>& out) const
        {
            ++m_performanceCounters.queries;
            auto h = handleByGuid(centerGuid);
            if (!h.id)
            {
                out.clear();
                return;
            }

            const ObjectSlot* centerSlot = nullptr;
            if (!tryGet(h, centerSlot))
            {
                out.clear();
                return;
            }

            const int gid = packGridFromPos(centerSlot->pos);
            auto [lcx, lcy] = worldToLocal(centerSlot->pos);

            bool valid = !centerSlot->nearCache.forceInvalidate
                && centerSlot->nearCache.centerGid == gid
                && centerSlot->nearCache.lcx == lcx
                && centerSlot->nearCache.lcy == lcy
                && centerSlot->nearCache.radius == radiusCells;

            if (valid)
            {
                std::size_t ok = 0;
                for (auto [cgid, clcid, cep] : centerSlot->nearCache.stamps)
                {
                    const GridChunk* grid = tryGetGrid(cgid);
                    if (!grid)
                    {
                        valid = false;
                        break;
                    }

                    const CellChunk* cell = tryGetCell(*grid, clcid);
                    if (!cell)
                    {
                        valid = false;
                        break;
                    }

                    const std::uint32_t now = cell->epoch;
                    if (now == cep)
                        ++ok;
                    else
                    {
                        valid = false;
                        break;
                    }
                }

                if (valid && ok == centerSlot->nearCache.stamps.size())
                {
                    out.clear();
                    out.reserve(centerSlot->nearCache.guids.size());
                    out.insert(out.end(), centerSlot->nearCache.guids.begin(), centerSlot->nearCache.guids.end());
                    return;
                }
            }

            const ObjectSlot* mutableSlot = nullptr;
            if (!tryGet(h, mutableSlot))
            {
                out.clear();
                return;
            }

            mutableSlot->nearCache.stamps.clear();
            mutableSlot->nearCache.guids.clear();

            std::unordered_set<std::uint64_t> dedupe;
            dedupe.reserve(256);
            dedupe.insert(centerGuid.getRawGuid());

            forEachRingCell(gid, lcx, lcy, radiusCells, [&](int cellGid, int cellLcid)
            {
                const GridChunk* grid = tryGetGrid(cellGid);
                if (!grid)
                    return;

                const CellChunk* cell = tryGetCell(*grid, cellLcid);
                if (!cell)
                    return;

                const std::uint32_t cellEpoch = cell->epoch;
                mutableSlot->nearCache.stamps.emplace_back(cellGid, cellLcid, cellEpoch);
                for (std::size_t container = 0; container < static_cast<std::size_t>(Container::Count); ++container)
                {
                    const auto& list = cell->byContainer[container];
                    m_performanceCounters.candidates += list.size();
                    for (auto objectHandle : list)
                    {
                        const ObjectSlot* objectSlot = nullptr;
                        if (!tryGet(objectHandle, objectSlot))
                            continue;

                        const std::uint64_t raw = objectSlot->meta.guid.getRawGuid();
                        if (dedupe.insert(raw).second)
                            mutableSlot->nearCache.guids.emplace_back(WoWGuid(raw));
                    }
                }
            });

            mutableSlot->nearCache.centerGid = gid;
            mutableSlot->nearCache.lcx = lcx;
            mutableSlot->nearCache.lcy = lcy;
            mutableSlot->nearCache.radius = radiusCells;
            mutableSlot->nearCache.forceInvalidate = false;
            mutableSlot->nearCache.trimExcessCapacity();

            out.clear();
            out.reserve(mutableSlot->nearCache.guids.size());
            out.insert(out.end(), mutableSlot->nearCache.guids.begin(), mutableSlot->nearCache.guids.end());
        }

        template<typename T>
        void collectGuidsInCell(int gid, int lcid, std::uint32_t entry, std::vector<WoWGuid>& out) const
        {
            ++m_performanceCounters.queries;
            const GridChunk* grid = tryGetGrid(gid);
            if (!grid)
                return;

            const CellChunk* cell = tryGetCell(*grid, lcid);
            if (!cell)
                return;

            constexpr std::size_t idx = static_cast<std::size_t>(TypeMap<T>::container);
            const auto& list = cell->byContainer[idx];
            out.reserve(out.size() + list.size());
            m_performanceCounters.candidates += list.size();

            for (auto h : list)
            {
                const ObjectSlot* slot = nullptr;
                if (!tryGet(h, slot) || !slot || !slot->native)
                    continue;

                T* obj = static_cast<T*>(slot->native);

                std::uint32_t objectEntry = 0;
                if constexpr (requires (T* t) { t->GetEntry(); })
                    objectEntry = obj->GetEntry();
                else if constexpr (requires (T* t) { t->getEntry(); })
                    objectEntry = obj->getEntry();

                if (entry == 0 || objectEntry == entry)
                    out.push_back(slot->meta.guid);
            }
        }

        template<typename T>
        void collectGuidsInCellsAroundPos(const LocationVector& center, int radiusCells, std::uint32_t entry, std::vector<WoWGuid>& out) const
        {
            const int baseGid = packGridFromPos(center);
            auto [lcx, lcy] = worldToLocal(center);

            forEachRingCell(baseGid, lcx, lcy, radiusCells, [&](int gid, int lcid)
            {
                collectGuidsInCell<T>(gid, lcid, entry, out);
            });
        }

        template<typename T>
        void collectByEntryInRange(const LocationVector& centerPos, std::list<T*>& out, std::uint32_t entry, float maxSearchRange) const
        {
            if (maxSearchRange < 0.0f)
                return;

            const float r2 = maxSearchRange * maxSearchRange;

            forEachContainerObjectInRange<T>(centerPos, maxSearchRange, [&](ObjectHandle h, const ObjectSlot& slot)
            {
                T* obj = static_cast<T*>(slot.native);
                if (!obj)
                    return;

                std::uint32_t objectEntry = 0;
                if constexpr (requires (T* t) { t->GetEntry(); })
                    objectEntry = obj->GetEntry();
                else if constexpr (requires (T* t) { t->getEntry(); })
                    objectEntry = obj->getEntry();

                if (entry != 0 && objectEntry != entry)
                    return;

                const float dx = slot.pos.x - centerPos.x;
                const float dy = slot.pos.y - centerPos.y;
                if ((dx * dx + dy * dy) <= r2)
                    out.push_back(obj);
            });
        }

        template<typename T>
        void collectObjectsInRange(const LocationVector& centerPos, float maxSearchRange, std::vector<T*>& out) const
        {
            if (maxSearchRange < 0.0f)
                return;

            const float r2 = maxSearchRange * maxSearchRange;

            if constexpr (std::is_same_v<T, Object>)
            {
                forEachAnyObjectInRange(centerPos, maxSearchRange, [&](ObjectHandle h, const ObjectSlot& slot)
                {
                    if (!slot.native)
                        return;

                    const float dx = slot.pos.x - centerPos.x;
                    const float dy = slot.pos.y - centerPos.y;
                    if ((dx * dx + dy * dy) <= r2)
                        out.push_back(static_cast<Object*>(slot.native));
                });
            }
            else
            {
                forEachContainerObjectInRange<T>(centerPos, maxSearchRange, [&](ObjectHandle h, const ObjectSlot& slot)
                {
                    if (!slot.native)
                        return;

                    const float dx = slot.pos.x - centerPos.x;
                    const float dy = slot.pos.y - centerPos.y;
                    if ((dx * dx + dy * dy) <= r2)
                        out.push_back(static_cast<T*>(slot.native));
                });
            }
        }

        void collectUnitsInRange(const LocationVector& centerPos, float maxSearchRange, std::vector<Unit*>& out) const
        {
            if (maxSearchRange < 0.0f)
                return;

            const float r2 = maxSearchRange * maxSearchRange;
            constexpr std::array<std::size_t, 3> unitContainers{
                static_cast<std::size_t>(Container::Players),
                static_cast<std::size_t>(Container::Creatures),
                static_cast<std::size_t>(Container::Pets)
            };

            forEachObjectInContainers(centerPos, maxSearchRange, unitContainers, [&](ObjectHandle h, const ObjectSlot& slot)
            {
                if (!slot.native)
                    return;

                const float dx = slot.pos.x - centerPos.x;
                const float dy = slot.pos.y - centerPos.y;
                if ((dx * dx + dy * dy) <= r2)
                    out.push_back(static_cast<Unit*>(slot.native));
            });
        }

        /// Allocation-free read-only range visitors for hot spatial queries.
        /// Spatial storage is WorldMap-thread owned. The callback must not attach,
        /// detach or spatially move objects while the current bucket is being iterated,
        /// because that could invalidate the vector traversal. Gameplay state may be inspected.
        template<typename T, typename Fn>
        void forEachObjectInRangeReadOnly(const LocationVector& centerPos, float maxSearchRange, Fn&& fn) const
        {
            if (maxSearchRange < 0.0f)
                return;

            const float r2 = maxSearchRange * maxSearchRange;

            auto visit = [&](ObjectHandle /*h*/, const ObjectSlot& slot)
            {
                if (!slot.native)
                    return;

                const float dx = slot.pos.x - centerPos.x;
                const float dy = slot.pos.y - centerPos.y;
                if ((dx * dx + dy * dy) <= r2)
                    fn(static_cast<T*>(slot.native));
            };

            if constexpr (std::is_same_v<T, Object>)
                forEachAnyObjectInRange(centerPos, maxSearchRange, visit);
            else
                forEachContainerObjectInRange<T>(centerPos, maxSearchRange, visit);
        }

        template<typename Fn>
        void forEachUnitInRangeReadOnly(const LocationVector& centerPos, float maxSearchRange, Fn&& fn) const
        {
            if (maxSearchRange < 0.0f)
                return;

            const float r2 = maxSearchRange * maxSearchRange;
            constexpr std::array<std::size_t, 3> unitContainers{
                static_cast<std::size_t>(Container::Players),
                static_cast<std::size_t>(Container::Creatures),
                static_cast<std::size_t>(Container::Pets)
            };

            forEachObjectInContainers(centerPos, maxSearchRange, unitContainers, [&](ObjectHandle /*h*/, const ObjectSlot& slot)
            {
                if (!slot.native)
                    return;

                const float dx = slot.pos.x - centerPos.x;
                const float dy = slot.pos.y - centerPos.y;
                if ((dx * dx + dy * dy) <= r2)
                    fn(static_cast<Unit*>(slot.native));
            });
        }

        template<typename T>
        T* findNearestByEntry(const LocationVector& centerPos, std::uint32_t entry, float maxSearchRange) const
        {
            if (maxSearchRange < 0.0f)
                return nullptr;

            const float r2 = maxSearchRange * maxSearchRange;
            T* best = nullptr;
            float bestD2 = std::numeric_limits<float>::max();

            forEachContainerObjectInRange<T>(centerPos, maxSearchRange, [&](ObjectHandle h, const ObjectSlot& slot)
            {
                T* obj = static_cast<T*>(slot.native);
                if (!obj)
                    return;

                std::uint32_t objectEntry = 0;
                if constexpr (requires (T* t) { t->GetEntry(); })
                    objectEntry = obj->GetEntry();
                else if constexpr (requires (T* t) { t->getEntry(); })
                    objectEntry = obj->getEntry();

                if (entry != 0 && objectEntry != entry)
                    return;

                const float dx = slot.pos.x - centerPos.x;
                const float dy = slot.pos.y - centerPos.y;
                const float d2 = dx * dx + dy * dy;
                if (d2 <= r2 && d2 < bestD2)
                {
                    bestD2 = d2;
                    best = obj;
                }
            });

            return best;
        }

        std::size_t gridCount() const noexcept
        {
            return m_loadedGridCount;
        }

    private:
        bool eraseTracked(std::vector<ObjectHandle>& handles, ObjectHandle h, std::uint32_t& trackedIndex, bool cellBucket)
        {
            std::size_t removeIndex = trackedIndex;

            /// Normal hot path: the slot already knows its exact vector position.
            /// Fall back to a linear repair lookup only if an older/stale state is
            /// encountered, keeping correctness while preserving O(1) normally.
            if (removeIndex >= handles.size() || handles[removeIndex] != h)
            {
                auto it = std::find(handles.begin(), handles.end(), h);
                if (it == handles.end())
                {
                    trackedIndex = ObjectSlot::InvalidSpatialBucketIndex;
                    return false;
                }

                removeIndex = static_cast<std::size_t>(it - handles.begin());
            }

            const std::size_t lastIndex = handles.size() - 1;
            const ObjectHandle moved = handles[lastIndex];

            if (removeIndex != lastIndex)
                handles[removeIndex] = moved;

            handles.pop_back();
            trackedIndex = ObjectSlot::InvalidSpatialBucketIndex;

            if (removeIndex != lastIndex)
            {
                ObjectSlot* movedSlot = nullptr;
                if (m_pool.tryGet(moved, movedSlot))
                {
                    auto& movedIndex = cellBucket ? movedSlot->cellBucketIndex : movedSlot->gridOwnerBucketIndex;
                    movedIndex = static_cast<std::uint32_t>(removeIndex);
                }
            }

            return true;
        }

        static bool trackedMembershipValid(const std::vector<ObjectHandle>& handles, ObjectHandle h, std::uint32_t index)
        {
            return index < handles.size() && handles[index] == h;
        }

        static void appendTracked(std::vector<ObjectHandle>& handles, ObjectHandle h, std::uint32_t& trackedIndex)
        {
            if (trackedMembershipValid(handles, h, trackedIndex))
                return;

            trackedIndex = static_cast<std::uint32_t>(handles.size());
            handles.push_back(h);
        }

        void attachToCell(ObjectHandle h, const LocationVector& pos)
        {
            ObjectSlot* slot = nullptr;
            if (!m_pool.tryGet(h, slot))
                return;

            const auto& meta = slot->meta;
            const int gid = packGridFromPos(pos);
            const int lcid = packCellFromPos(pos);

            auto& grid = getOrCreateGrid(gid);
            auto& cell = getOrCreateCell(grid, lcid);

            {
                auto& owners = grid.owners[static_cast<std::size_t>(meta.container)];

                appendTracked(owners, h, slot->gridOwnerBucketIndex);
            }

            {
                auto& bucket = cell.byContainer[static_cast<std::size_t>(meta.container)];

                appendTracked(bucket, h, slot->cellBucketIndex);

                ++cell.epoch;
            }
        }

        void detachFromCell(ObjectHandle h, const LocationVector& pos)
        {
            ObjectSlot* slot = nullptr;
            if (!m_pool.tryGet(h, slot))
                return;

            const auto& meta = slot->meta;
            const int gid = packGridFromPos(pos);
            const int lcid = packCellFromPos(pos);

            auto* grid = tryGetGrid(gid);
            if (!grid)
                return;

            {
                auto& owners = grid->owners[static_cast<std::size_t>(meta.container)];
                eraseTracked(owners, h, slot->gridOwnerBucketIndex, false);
            }

            if (auto* cell = tryGetCell(*grid, lcid))
            {
                auto& bucket = cell->byContainer[static_cast<std::size_t>(meta.container)];
                eraseTracked(bucket, h, slot->cellBucketIndex, true);
                ++cell->epoch;
            }
        }

        void reindexWithinGrid(ObjectHandle h, int gid, int oldLcid, int newLcid)
        {
            ObjectSlot* slot = nullptr;
            if (!m_pool.tryGet(h, slot))
                return;

            const auto& meta = slot->meta;
            auto* grid = tryGetGrid(gid);
            if (!grid)
                return;

            if (auto* oldCell = tryGetCell(*grid, oldLcid))
            {
                auto& bucket = oldCell->byContainer[static_cast<std::size_t>(meta.container)];
                eraseTracked(bucket, h, slot->cellBucketIndex, true);
                ++oldCell->epoch;
            }

            auto& newCell = getOrCreateCell(*grid, newLcid);
            {
                auto& bucket = newCell.byContainer[static_cast<std::size_t>(meta.container)];

                appendTracked(bucket, h, slot->cellBucketIndex);

                ++newCell.epoch;
            }
        }

        void transferCell(ObjectHandle h, int oldGid, int newGid, int oldLcid, int newLcid)
        {
            ObjectSlot* slot = nullptr;
            if (!m_pool.tryGet(h, slot))
                return;

            const auto& meta = slot->meta;

            if (auto* oldGrid = tryGetGrid(oldGid))
            {
                {
                    auto& owners = oldGrid->owners[static_cast<std::size_t>(meta.container)];
                    eraseTracked(owners, h, slot->gridOwnerBucketIndex, false);
                }

                if (auto* oldCell = tryGetCell(*oldGrid, oldLcid))
                {
                    auto& bucket = oldCell->byContainer[static_cast<std::size_t>(meta.container)];
                    eraseTracked(bucket, h, slot->cellBucketIndex, true);
                    ++oldCell->epoch;
                }
            }

            auto& newGrid = getOrCreateGrid(newGid);
            {
                auto& owners = newGrid.owners[static_cast<std::size_t>(meta.container)];

                appendTracked(owners, h, slot->gridOwnerBucketIndex);
            }

            auto& newCell = getOrCreateCell(newGrid, newLcid);
            {
                auto& bucket = newCell.byContainer[static_cast<std::size_t>(meta.container)];

                appendTracked(bucket, h, slot->cellBucketIndex);

                ++newCell.epoch;
            }
        }


        template<std::size_t N, typename Fn>
        void forEachObjectInContainers(const LocationVector& centerPos, float maxSearchRange, const std::array<std::size_t, N>& containers, Fn&& fn) const
        {
            ++m_performanceCounters.queries;
            const float minX = centerPos.x - maxSearchRange;
            const float maxX = centerPos.x + maxSearchRange;
            const float minY = centerPos.y - maxSearchRange;
            const float maxY = centerPos.y + maxSearchRange;

            auto clampGrid = [](int g) { return std::clamp(g, 0, Terrain::TilesCount - 1); };
            const int minGx = clampGrid(static_cast<int>(std::floor((minX - Terrain::MinX) / Terrain::TileSize)));
            const int maxGx = clampGrid(static_cast<int>(std::floor((maxX - Terrain::MinX) / Terrain::TileSize)));
            const int minGy = clampGrid(static_cast<int>(std::floor((minY - Terrain::MinY) / Terrain::TileSize)));
            const int maxGy = clampGrid(static_cast<int>(std::floor((maxY - Terrain::MinY) / Terrain::TileSize)));

            for (int gy = minGy; gy <= maxGy; ++gy)
            {
                for (int gx = minGx; gx <= maxGx; ++gx)
                {
                    const float gridOriginX = Terrain::MinX + gx * Terrain::TileSize;
                    const float gridOriginY = Terrain::MinY + gy * Terrain::TileSize;

                    auto clampCell = [](int c) { return std::clamp(c, 0, Cell::CellsPerTile - 1); };
                    const int minCx = clampCell(static_cast<int>(std::floor((minX - gridOriginX) / Cell::Size)));
                    const int maxCx = clampCell(static_cast<int>(std::floor((maxX - gridOriginX) / Cell::Size)));
                    const int minCy = clampCell(static_cast<int>(std::floor((minY - gridOriginY) / Cell::Size)));
                    const int maxCy = clampCell(static_cast<int>(std::floor((maxY - gridOriginY) / Cell::Size)));

                    const int gid = packGridId(gx, gy);
                    const GridChunk* grid = tryGetGrid(gid);
                    if (!grid)
                        continue;

                    for (int cy = minCy; cy <= maxCy; ++cy)
                    {
                        for (int cx = minCx; cx <= maxCx; ++cx)
                        {
                            const int lcid = packCellId(cx, cy);
                            const CellChunk* cell = tryGetCell(*grid, lcid);
                            if (!cell)
                                continue;
                            for (std::size_t idx : containers)
                            {
                                const auto& list = cell->byContainer[idx];
                                m_performanceCounters.candidates += list.size();
                                for (auto h : list)
                                {
                                    const ObjectSlot* slot = nullptr;
                                    if (!tryGet(h, slot) || !slot || !slot->native)
                                        continue;

                                    fn(h, *slot);
                                }
                            }
                        }
                    }
                }
            }
        }

        template<typename T, typename Fn>
        void forEachContainerObjectInRange(const LocationVector& centerPos, float maxSearchRange, Fn&& fn) const
        {
            constexpr std::array<std::size_t, 1> containers{ static_cast<std::size_t>(TypeMap<T>::container) };
            forEachObjectInContainers(centerPos, maxSearchRange, containers, std::forward<Fn>(fn));
        }

        template<typename Fn>
        void forEachAnyObjectInRange(const LocationVector& centerPos, float maxSearchRange, Fn&& fn) const
        {
            ++m_performanceCounters.queries;
            std::array<std::size_t, static_cast<std::size_t>(Container::Count)> containers{};
            for (std::size_t i = 0; i < containers.size(); ++i)
                containers[i] = i;

            const float minX = centerPos.x - maxSearchRange;
            const float maxX = centerPos.x + maxSearchRange;
            const float minY = centerPos.y - maxSearchRange;
            const float maxY = centerPos.y + maxSearchRange;

            auto clampGrid = [](int g) { return std::clamp(g, 0, Terrain::TilesCount - 1); };
            const int minGx = clampGrid(static_cast<int>(std::floor((minX - Terrain::MinX) / Terrain::TileSize)));
            const int maxGx = clampGrid(static_cast<int>(std::floor((maxX - Terrain::MinX) / Terrain::TileSize)));
            const int minGy = clampGrid(static_cast<int>(std::floor((minY - Terrain::MinY) / Terrain::TileSize)));
            const int maxGy = clampGrid(static_cast<int>(std::floor((maxY - Terrain::MinY) / Terrain::TileSize)));

            for (int gy = minGy; gy <= maxGy; ++gy)
            {
                for (int gx = minGx; gx <= maxGx; ++gx)
                {
                    const float gridOriginX = Terrain::MinX + gx * Terrain::TileSize;
                    const float gridOriginY = Terrain::MinY + gy * Terrain::TileSize;

                    auto clampCell = [](int c) { return std::clamp(c, 0, Cell::CellsPerTile - 1); };
                    const int minCx = clampCell(static_cast<int>(std::floor((minX - gridOriginX) / Cell::Size)));
                    const int maxCx = clampCell(static_cast<int>(std::floor((maxX - gridOriginX) / Cell::Size)));
                    const int minCy = clampCell(static_cast<int>(std::floor((minY - gridOriginY) / Cell::Size)));
                    const int maxCy = clampCell(static_cast<int>(std::floor((maxY - gridOriginY) / Cell::Size)));

                    const int gid = packGridId(gx, gy);
                    const GridChunk* grid = tryGetGrid(gid);
                    if (!grid)
                        continue;

                    for (int cy = minCy; cy <= maxCy; ++cy)
                    {
                        for (int cx = minCx; cx <= maxCx; ++cx)
                        {
                            const int lcid = packCellId(cx, cy);
                            const CellChunk* cell = tryGetCell(*grid, lcid);
                            if (!cell)
                                continue;
                            for (std::size_t idx = 0; idx < static_cast<std::size_t>(Container::Count); ++idx)
                            {
                                const auto& list = cell->byContainer[idx];
                                m_performanceCounters.candidates += list.size();
                                for (auto h : list)
                                {
                                    const ObjectSlot* slot = nullptr;
                                    if (!tryGet(h, slot) || !slot || !slot->native)
                                        continue;

                                    fn(h, *slot);
                                }
                            }
                        }
                    }
                }
            }
        }

        ObjectPool m_pool;
        mutable SpatialPerformanceCounters m_performanceCounters{};
        GridStorage m_grids{};
        std::size_t m_loadedGridCount{ 0 };
        /// Keep the main spatial data map-thread-owned; only this lookup is shared.
        mutable std::shared_mutex m_guidToHandleMutex;
        std::unordered_map<std::uint64_t, ObjectHandle> m_guidToHandle;
    };
}
