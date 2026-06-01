/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

namespace AscEmu
{
    class BipBuffer
    {
    public:
        BipBuffer() = default;

        explicit BipBuffer(size_t capacity)
        {
            Allocate(capacity);
        }

        BipBuffer(const BipBuffer&) = delete;
        BipBuffer& operator=(const BipBuffer&) = delete;

        BipBuffer(BipBuffer&&) noexcept = default;
        BipBuffer& operator=(BipBuffer&&) noexcept = default;

        void Allocate(size_t capacity)
        {
            m_buffer.assign(capacity, std::byte{});
            resetRegions();
        }

        [[nodiscard]] size_t GetSize() const noexcept
        {
            return m_size;
        }

        [[nodiscard]] size_t Capacity() const noexcept
        {
            return m_buffer.size();
        }

        [[nodiscard]] bool Empty() const noexcept
        {
            return GetSize() == 0;
        }

        [[nodiscard]] bool Full() const noexcept
        {
            return GetSize() == Capacity();
        }

        // Compatibility with the old typo in CircularBuffer.
        [[nodiscard]] size_t GetContiguiousBytes() const noexcept
        {
            return GetContiguousBytes();
        }

        [[nodiscard]] size_t GetContiguousBytes() const noexcept
        {
            if (m_regionASize > 0)
                return m_regionASize;

            return m_regionBSize;
        }

        [[nodiscard]] void* GetBufferStart() noexcept
        {
            if (m_regionASize > 0)
                return m_buffer.data() + m_regionAStart;

            if (m_regionBSize > 0)
                return m_buffer.data() + m_regionBStart;

            return nullptr;
        }

        [[nodiscard]] const void* GetBufferStart() const noexcept
        {
            if (m_regionASize > 0)
                return m_buffer.data() + m_regionAStart;

            if (m_regionBSize > 0)
                return m_buffer.data() + m_regionBStart;

            return nullptr;
        }

        bool Write(const void* data, size_t bytes)
        {
            if (bytes == 0)
                return true;

            if (data == nullptr || m_buffer.empty())
                return false;

            ensureWriteRegion();

            if (m_writeRegion == WriteRegion::B)
            {
                const size_t writeOffset = m_regionBStart + m_regionBSize;
                const size_t available = m_regionAStart - writeOffset;

                if (bytes > available)
                    return false;

                std::memcpy(m_buffer.data() + writeOffset, data, bytes);
                m_regionBSize += bytes;
                m_size += bytes;
                return true;
            }

            const size_t writeOffset = m_regionAStart + m_regionASize;
            const size_t available = Capacity() - writeOffset;

            if (bytes > available)
                return false;

            std::memcpy(m_buffer.data() + writeOffset, data, bytes);
            m_regionASize += bytes;
            m_size += bytes;
            return true;
        }

        bool Read(void* destination, size_t bytes)
        {
            if (bytes == 0)
                return true;

            if (destination == nullptr || m_size < bytes)
                return false;

            auto* target = static_cast<std::byte*>(destination);
            size_t remaining = bytes;

            if (m_regionASize > 0)
            {
                const size_t amount = std::min(remaining, m_regionASize);

                std::memcpy(target, m_buffer.data() + m_regionAStart, amount);

                m_regionAStart += amount;
                m_regionASize -= amount;
                remaining -= amount;
                target += amount;
            }

            if (remaining > 0)
            {
                std::memcpy(target, m_buffer.data() + m_regionBStart, remaining);

                m_regionBStart += remaining;
                m_regionBSize -= remaining;
            }

            m_size -= bytes;
            normalizeAfterRead();
            return true;
        }

        void Remove(size_t bytes) noexcept
        {
            const size_t bytesToRemove = std::min(bytes, m_size);
            size_t remaining = bytesToRemove;

            if (m_regionASize > 0)
            {
                const size_t amount = std::min(remaining, m_regionASize);
                m_regionAStart += amount;
                m_regionASize -= amount;
                remaining -= amount;
            }

            if (remaining > 0 && m_regionBSize > 0)
            {
                const size_t amount = std::min(remaining, m_regionBSize);
                m_regionBStart += amount;
                m_regionBSize -= amount;
            }

            m_size -= bytesToRemove;
            normalizeAfterRead();
        }

        // Returns contiguous writable space, not total free space.
        // This intentionally mirrors the old CircularBuffer behavior.
        [[nodiscard]] size_t GetSpace() noexcept
        {
            if (m_buffer.empty())
                return 0;

            ensureWriteRegion();

            return m_writeRegion == WriteRegion::B ? getBFreeSpace() : getAFreeSpace();
        }

        [[nodiscard]] void* GetBuffer() noexcept
        {
            if (m_buffer.empty())
                return nullptr;

            ensureWriteRegion();

            if (m_writeRegion == WriteRegion::B)
            {
                if (getBFreeSpace() == 0)
                    return nullptr;

                return m_buffer.data() + m_regionBStart + m_regionBSize;
            }

            if (getAFreeSpace() == 0)
                return nullptr;

            return m_buffer.data() + m_regionAStart + m_regionASize;
        }

        [[nodiscard]] std::span<std::byte> WritableSpan() noexcept
        {
            return {
                static_cast<std::byte*>(GetBuffer()),
                GetSpace()
            };
        }

        [[nodiscard]] std::span<std::byte const> ReadableSpan() const noexcept
        {
            auto const* start = static_cast<std::byte const*>(GetBufferStart());
            return { start, GetContiguousBytes() };
        }

        bool Commit(size_t bytes) noexcept
        {
            ensureWriteRegion();

            if (bytes > currentWriteSpace())
                return false;

            if (m_writeRegion == WriteRegion::B)
                m_regionBSize += bytes;
            else
                m_regionASize += bytes;

            m_size += bytes;
            return true;
        }

        // Old CircularBuffer compatibility
        void IncrementWritten(size_t bytes) noexcept
        {
            const bool committed = Commit(bytes);
            assert(committed && "BipBuffer::IncrementWritten exceeds contiguous writable space");
        }

        [[nodiscard]] size_t FreeSpace() const noexcept
        {
            return Capacity() - m_size;
        }

        void Clear() noexcept
        {
            resetRegions();
        }

    private:
        enum class WriteRegion {A, B};

        void resetRegions() noexcept
        {
            m_regionAStart = 0;
            m_regionASize = 0;

            m_regionBStart = 0;
            m_regionBSize = 0;
            m_regionBActive = false;

            m_size = 0;

            m_writeRegion = WriteRegion::A;
        }

        [[nodiscard]] size_t getSpaceBeforeA() const noexcept
        {
            return m_regionAStart;
        }

        [[nodiscard]] size_t getAFreeSpace() const noexcept
        {
            if (m_buffer.empty())
                return 0;

            return Capacity() - m_regionAStart - m_regionASize;
        }

        [[nodiscard]] size_t getBFreeSpace() const noexcept
        {
            if (!m_regionBActive || m_buffer.empty())
                return 0;

            return m_regionAStart - m_regionBStart - m_regionBSize;
        }

        [[nodiscard]] size_t currentWriteSpace() const noexcept
        {
            if (m_writeRegion == WriteRegion::B)
                return getBFreeSpace();

            return getAFreeSpace();
        }

        void ensureWriteRegion() noexcept
        {
            if (m_buffer.empty())
                return;

            if (m_regionBActive)
            {
                m_writeRegion = WriteRegion::B;
                return;
            }

            // If the space before A is bigger than the tail after A,
            // start writing to B at the beginning of the backing buffer.
            if (getAFreeSpace() < getSpaceBeforeA())
            {
                m_regionBActive = true;
                m_regionBStart = 0;
                m_regionBSize = 0;
                m_writeRegion = WriteRegion::B;
                return;
            }

            m_writeRegion = WriteRegion::A;
        }

        void normalizeAfterRead() noexcept
        {
            if (m_regionASize != 0)
                return;

            if (m_regionBActive && m_regionBSize > 0)
            {
                // When A becomes empty, move B back to the beginning.
                if (m_regionBStart != 0)
                {
                    std::memmove(
                        m_buffer.data(),
                        m_buffer.data() + m_regionBStart,
                        m_regionBSize
                    );
                }

                m_regionAStart = 0;
                m_regionASize = m_regionBSize;

                m_regionBStart = 0;
                m_regionBSize = 0;
                m_regionBActive = false;
                m_writeRegion = WriteRegion::A;
                return;
            }

            resetRegions();
        }

    private:
        std::vector<std::byte> m_buffer;

        size_t m_regionAStart = 0;
        size_t m_regionASize = 0;

        size_t m_regionBStart = 0;
        size_t m_regionBSize = 0;
        bool m_regionBActive = false;

        size_t m_size = 0;

        WriteRegion m_writeRegion = WriteRegion::A;
    };
}
