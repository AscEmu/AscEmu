/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <deque>
#include <limits>
#include <tuple>
#include <vector>

#include "VisibilityTypes.hpp"

namespace visibility
{
    //////////////////////////////////////////////////////////////////////////////////////////
    /// NearCache
    ///
    /// Small per-object cache for nearby spatial queries.
    //////////////////////////////////////////////////////////////////////////////////////////
    struct NearCache
    {
        int centerGid = -1;
        int lcx = -1;
        int lcy = -1;
        int radius = 2;

        std::vector<std::tuple<int, int, uint32_t>> stamps;
        std::vector<WoWGuid> guids;
        bool forceInvalidate = true;

        /// Mark the cache as invalid so it will be rebuilt on next use.
        void invalidate() { forceInvalidate = true; }

        /// Clear and release cached allocations. Use only on object release/unload,
        /// not on normal movement invalidation.
        void clearAndRelease()
        {
            centerGid = -1;
            lcx = -1;
            lcy = -1;
            radius = 2;

            stamps.clear();
            guids.clear();
            stamps.shrink_to_fit();
            guids.shrink_to_fit();

            forceInvalidate = true;
        }

        /// Trim excessive retained vector capacity after a cache rebuild.
        /// This is intentionally conservative and is never called on movement
        /// invalidation or cache hits.
        void trimExcessCapacity()
        {
            constexpr std::size_t GuidCapacityFloor = 4096;
            constexpr std::size_t StampCapacityFloor = 512;

            if (guids.capacity() > GuidCapacityFloor && guids.capacity() > guids.size() * 4)
                std::vector<WoWGuid>(guids).swap(guids);

            if (stamps.capacity() > StampCapacityFloor && stamps.capacity() > stamps.size() * 4)
                std::vector<std::tuple<int, int, uint32_t>>(stamps).swap(stamps);
        }
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    /// ObjectSlot
    ///
    /// A single slot inside the ObjectPool storing metadata, position and optional
    /// back-reference to a native engine object.
    //////////////////////////////////////////////////////////////////////////////////////////
    struct ObjectSlot
    {
        /// Handle Generation & Use-Flag
        std::uint32_t               gen{ 1 };
        bool                        inUse{ false };

        /// Meta / Position / Backref
        ObjectMeta                  meta{};
        LocationVector              pos{};
        void*                       native{ nullptr };  // optional Back-Ref (no Ownership)
        SubState                    sub{};              // subscription state
        PubState                    pub{};              // broadcast state
        InterestProfile             interest{};         // interest / publish profile

        /// Last position used for the more expensive visibility refresh.
        bool                        lastObjectInterestRefreshValid{ false };
        LocationVector              lastObjectInterestRefreshPos{};

        /// Scratch stamp used to dedupe visibility candidates.
        mutable std::uint64_t       visibilityQueryStamp{ 0 };

        /// Indices used for O(1) removal from spatial buckets.
        inline static constexpr std::uint32_t InvalidSpatialBucketIndex = std::numeric_limits<std::uint32_t>::max();
        std::uint32_t               cellBucketIndex{ InvalidSpatialBucketIndex };
        std::uint32_t               gridOwnerBucketIndex{ InvalidSpatialBucketIndex };

        mutable NearCache           nearCache;

        ObjectSlot() = default;

        /// no copy
        ObjectSlot(const ObjectSlot&) = delete;
        ObjectSlot& operator=(const ObjectSlot&) = delete;

        /// explicit move
        ObjectSlot(ObjectSlot&& other) noexcept
        {
            gen = other.gen;
            inUse = other.inUse;
            meta = other.meta;
            pos = other.pos;
            native = other.native;
            sub = other.sub;
            pub = other.pub;
            interest = other.interest;
            lastObjectInterestRefreshValid = other.lastObjectInterestRefreshValid;
            lastObjectInterestRefreshPos = other.lastObjectInterestRefreshPos;
            visibilityQueryStamp = 0;
            cellBucketIndex = other.cellBucketIndex;
            gridOwnerBucketIndex = other.gridOwnerBucketIndex;
            /// nearCache intentionally not moved to avoid stale references
            nearCache.invalidate();
        }

        ObjectSlot& operator=(ObjectSlot&& other) noexcept
        {
            if (this != &other)
            {
                gen = other.gen;
                inUse = other.inUse;
                meta = other.meta;
                pos = other.pos;
                native = other.native;
                sub = other.sub;
                pub = other.pub;
                interest = other.interest;
                lastObjectInterestRefreshValid = other.lastObjectInterestRefreshValid;
                lastObjectInterestRefreshPos = other.lastObjectInterestRefreshPos;
                visibilityQueryStamp = 0;
                cellBucketIndex = other.cellBucketIndex;
                gridOwnerBucketIndex = other.gridOwnerBucketIndex;
                /// nearCache intentionally not moved to avoid stale references
                nearCache.invalidate();
            }
            return *this;
        }
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    /// ObjectPool
    ///
    /// Growing slot pool used by the spatial index.
    /// Slot 0 stays invalid and generations protect against stale handles.
    //////////////////////////////////////////////////////////////////////////////////////////
    class ObjectPool
    {
    public:
        /// Create an empty object pool.
        ///
        /// Slot 0 is created immediately and kept reserved as invalid handle value.
        explicit ObjectPool(std::size_t initialCapacity = 0)
        {
            /// Slot 0 is never allocated. It represents an invalid ObjectHandle.
            m_slots.resize(1);
            m_next.resize(1, 0);

            if (initialCapacity > 0)
                grow(initialCapacity);
        }

        ObjectPool(const ObjectPool&) = delete;
        ObjectPool& operator=(const ObjectPool&) = delete;
        ObjectPool(ObjectPool&&) = delete;
        ObjectPool& operator=(ObjectPool&&) = delete;

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Allocate / Release
        //////////////////////////////////////////////////////////////////////////////////////////
        /// Allocate a slot and return its generation checked handle.
        ///
        /// The pool grows automatically if no free slot is available.
        ObjectHandle allocate()
        {
            if (m_freeHead == 0)
            {
                const std::size_t currentCapacity = capacityUnlocked();

                /// Grow gradually for small pools and exponentially for larger pools.
                const std::size_t growBy = currentCapacity < 64 ? 16 : currentCapacity / 2;
                grow(growBy);
            }

            const std::uint32_t id = m_freeHead;
            m_freeHead = m_next[id];
            m_next[id] = 0;

            auto& slot = m_slots[id];
            slot.inUse = true;

            ++m_liveCount;

            return { id, slot.gen };
        }

        /// Release a previously allocated handle back to the pool.
        ///
        /// Invalid, stale or already released handles are ignored.
        bool release(ObjectHandle h)
        {
            if (h.id == 0 || h.id >= m_slots.size())
                return false;

            auto& slot = m_slots[h.id];

            if (slot.gen != h.gen)
                return false;

            if (!slot.inUse)
                return false;

            slot.inUse = false;

            resetSlot(slot);

            /// Bump generation after the slot was released so old handles become stale.
            ++slot.gen;

            m_next[h.id] = m_freeHead;
            m_freeHead = h.id;

            --m_liveCount;

            return true;
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Access
        //////////////////////////////////////////////////////////////////////////////////////////
        /// Try to obtain a mutable pointer to the slot for a valid handle.
        bool tryGet(ObjectHandle h, ObjectSlot*& out)
        {
            if (h.id == 0 || h.id >= m_slots.size())
                return false;

            auto& slot = m_slots[h.id];

            if (!slot.inUse)
                return false;

            if (slot.gen != h.gen)
                return false;

            out = &slot;
            return true;
        }

        /// Try to obtain a const pointer to the slot for a valid handle.
        bool tryGet(ObjectHandle h, const ObjectSlot*& out) const
        {
            if (h.id == 0 || h.id >= m_slots.size())
                return false;

            const auto& slot = m_slots[h.id];

            if (!slot.inUse)
                return false;

            if (slot.gen != h.gen)
                return false;

            out = &slot;
            return true;
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Init / Meta / Pos
        //////////////////////////////////////////////////////////////////////////////////////////
        /// Initialize an allocated slot.
        ///
        /// Returns false if the handle is invalid or stale.
        bool init(ObjectHandle h, const ObjectMeta& meta, const LocationVector& pos, void* native = nullptr)
        {
            ObjectSlot* slot = nullptr;
            if (!tryGet(h, slot))
                return false;

            slot->meta = meta;
            slot->pos = pos;
            slot->native = native;
            slot->sub = {};
            slot->pub = {};
            slot->interest = {};
            slot->lastObjectInterestRefreshValid = false;
            slot->lastObjectInterestRefreshPos = {};
            slot->visibilityQueryStamp = 0;
            slot->cellBucketIndex = ObjectSlot::InvalidSpatialBucketIndex;
            slot->gridOwnerBucketIndex = ObjectSlot::InvalidSpatialBucketIndex;
            slot->nearCache = {};
            slot->nearCache.forceInvalidate = true;

            return true;
        }

        /// Returns mutable metadata without validating the handle.
        ///
        /// Caller must guarantee that the handle is valid.
        ObjectMeta& meta(ObjectHandle h)
        {
            return m_slots[h.id].meta;
        }

        /// Returns immutable metadata without validating the handle.
        ///
        /// Caller must guarantee that the handle is valid.
        const ObjectMeta& meta(ObjectHandle h) const
        {
            return m_slots[h.id].meta;
        }

        /// Returns mutable position without validating the handle.
        ///
        /// Caller must guarantee that the handle is valid.
        LocationVector& pos(ObjectHandle h)
        {
            return m_slots[h.id].pos;
        }

        /// Returns immutable position without validating the handle.
        ///
        /// Caller must guarantee that the handle is valid.
        const LocationVector& pos(ObjectHandle h) const
        {
            return m_slots[h.id].pos;
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Pool Stats / Maintenance
        //////////////////////////////////////////////////////////////////////////////////////////
        /// Number of currently allocated slots.
        std::uint32_t liveCount() const
        {
            return m_liveCount;
        }

        /// Number of usable slots, excluding reserved slot 0.
        std::uint32_t capacity() const
        {
            return static_cast<std::uint32_t>(capacityUnlocked());
        }

        /// Try to release unused memory from the end of the pool.
        ///
        /// Only free slots at the end can be removed. Live slots are never moved.
        ///
        /// \param minCapacity Minimum number of usable slots to keep.
        /// \param maxFreeRatio Shrink only if free slots exceed live slots by this ratio.
        void shrinkToFit(std::size_t minCapacity = 64, std::size_t maxFreeRatio = 4)
        {
            const std::size_t usableCapacity = capacityUnlocked();
            if (usableCapacity <= minCapacity)
                return;

            const std::size_t freeCount = usableCapacity - m_liveCount;
            if (m_liveCount > 0 && freeCount < m_liveCount * maxFreeRatio)
                return;

            rebuildFreeListWithoutTrailingFreeSlots(minCapacity);
        }

    private:
        /// Number of usable slots, excluding reserved slot 0.
        std::size_t capacityUnlocked() const noexcept
        {
            return m_slots.empty() ? 0 : m_slots.size() - 1;
        }

        /// Add new slots and link them into the free-list.
        void grow(std::size_t count)
        {
            if (count == 0)
                return;

            /// ObjectSlot addresses are intentionally stable: m_slots is a deque and
            /// slots are appended individually. Do not replace this with vector-backed
            /// storage or code that relocates existing slots; visibility callbacks can
            /// allocate additional objects while callers still hold ObjectSlot pointers.
            for (std::size_t i = 0; i < count; ++i)
            {
                const auto id = static_cast<std::uint32_t>(m_slots.size());

                m_slots.emplace_back();
                m_next.push_back(m_freeHead);
                m_freeHead = id;
            }
        }

        /// Reset all non-handle state of a released slot.
        void resetSlot(ObjectSlot& slot)
        {
            slot.native = nullptr;
            slot.sub = {};
            slot.pub = {};
            slot.interest = {};
            slot.lastObjectInterestRefreshValid = false;
            slot.lastObjectInterestRefreshPos = {};
            slot.visibilityQueryStamp = 0;
            slot.cellBucketIndex = ObjectSlot::InvalidSpatialBucketIndex;
            slot.gridOwnerBucketIndex = ObjectSlot::InvalidSpatialBucketIndex;
            slot.nearCache.clearAndRelease();
        }

        /// Rebuild the free-list and remove unused trailing slots.
        ///
        /// This does not move live slots. It only removes free slots from the end of the
        /// storage and then rebuilds the free-list for all remaining free slots.
        void rebuildFreeListWithoutTrailingFreeSlots(std::size_t minCapacity)
        {
            std::size_t newSize = m_slots.size();

            while (newSize > minCapacity + 1)
            {
                const ObjectSlot& slot = m_slots[newSize - 1];

                if (slot.inUse)
                    break;

                --newSize;
            }

            if (newSize == m_slots.size())
                return;

            while (m_slots.size() > newSize)
                m_slots.pop_back();

            m_next.resize(newSize, 0);

            m_freeHead = 0;

            for (std::size_t i = 1; i < m_slots.size(); ++i)
            {
                if (m_slots[i].inUse)
                    continue;

                const auto id = static_cast<std::uint32_t>(i);
                m_next[i] = m_freeHead;
                m_freeHead = id;
            }
        }

    private:
        std::deque<ObjectSlot>      m_slots{};
        std::vector<std::uint32_t>  m_next{};
        std::uint32_t               m_freeHead{ 0 };
        std::uint32_t               m_liveCount{ 0 };
    };


} /// namespace visibility
