/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GuidAllocator.hpp"
#include <algorithm>

uint32_t GuidAllocator::allocateLow(uint32_t highTypeMasked, bool canReuse)
{
    const int idx = slotOf(highTypeMasked);
    if (idx < 0)
    {
        throw std::logic_error("unsupported HighGuid for allocation");
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    if (canReuse && !m_reuse[idx].empty())
    {
        uint32_t lowGuid = m_reuse[idx].front();
        m_reuse[idx].pop_front();
        return lowGuid;
    }

    uint32_t& next = m_nextLow[idx];

    if (next >= 0xFFFFFFu)
    {
        throw std::overflow_error("24-bit id space exhausted");
    }

    return ++next;
}

uint64_t GuidAllocator::packRaw(uint32_t highType, uint32_t entryId, uint32_t lowCounter)
{
    const uint64_t highPart =
        (static_cast<uint64_t>(highType) & static_cast<uint64_t>(HIGHGUID_TYPE_MASK)) << 32;

    if (entryId > 0x00FFFFFFu)
    {
        throw std::out_of_range("entryId > 24-bit");
    }

    if (lowCounter > 0x00FFFFFFu)
    {
        throw std::out_of_range("lowCounter > 24-bit");
    }

    const uint64_t entryPart = static_cast<uint64_t>(entryId & 0x00FFFFFFu) << 24;
    const uint64_t lowPart = static_cast<uint64_t>(lowCounter & 0x00FFFFFFu);

    return highPart | entryPart | lowPart;
}

uint64_t GuidAllocator::allocCreature(uint32_t entry, bool canReuse, bool isVehicle)
{
    const uint32_t highType = isVehicle ? HIGHGUID_TYPE_VEHICLE : HIGHGUID_TYPE_UNIT;
    const uint32_t low = allocateLow(highType, canReuse);
    return packRaw(highType, entry, low);
}

uint64_t GuidAllocator::allocGameObject(uint32_t entry, bool canReuse)
{
    const uint32_t highType = HIGHGUID_TYPE_GAMEOBJECT;
    const uint32_t low = allocateLow(highType, canReuse);
    return packRaw(highType, entry, low);
}

uint64_t GuidAllocator::allocTransporter(uint32_t entry, bool canReuse)
{
    const uint32_t highType = HIGHGUID_TYPE_TRANSPORTER;
    const uint32_t low = allocateLow(highType, canReuse);
    return packRaw(highType, entry, low);
}

uint64_t GuidAllocator::allocDynamicObject(uint32_t entry, bool canReuse)
{
    const uint32_t highType = HIGHGUID_TYPE_DYNAMICOBJECT;
    const uint32_t low = allocateLow(highType, canReuse);
    return packRaw(highType, entry, low);
}

uint64_t GuidAllocator::allocCorpse(bool canReuse)
{
    const uint32_t highType = HIGHGUID_TYPE_CORPSE;
    const uint32_t low = allocateLow(highType, canReuse);
    return packRaw(highType, 0, low);
}

void GuidAllocator::release(const uint64_t& guid)
{
    const uint32_t typeMasked = extractHighType(guid);
    const int idx = slotOf(typeMasked);
    if (idx < 0)
    {
        return;
    }

    const uint32_t low = extractLow24(guid);
    std::lock_guard<std::mutex> lock(m_mutex);
    m_reuse[idx].push_back(low);
}
