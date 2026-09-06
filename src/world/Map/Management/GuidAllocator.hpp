/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <deque>
#include <mutex>
#include "WoWGuid.hpp"

class GuidAllocator
{
public:
    uint64_t allocCreature(uint32_t entry, bool canReuse = true, bool isVehicle = false);

    uint64_t allocGameObject(uint32_t entry, bool canReuse = true);

    uint64_t allocTransporter(uint32_t entry, bool canReuse = true);

    uint64_t allocDynamicObject(uint32_t entry = 0, bool canReuse = true);

    uint64_t allocCorpse(bool canReuse = true);

    void release(const uint64_t& guid);

    uint32_t allocateLow(uint32_t highTypeMasked, bool canReuse);
    uint64_t packRaw(uint32_t highType, uint32_t entryId, uint32_t lowCounter);

private:
    static inline uint32_t extractHighType(uint64_t guid)
    {
        return static_cast<uint32_t>(WoWGuid::getHighTypeFromRaw(guid));
    }

    static inline uint32_t extractLow24(uint64_t guid)
    {
        return WoWGuid::getLowGuidFromRaw(guid) & LOWGUID_ENTRY_MASK;
    }
    static constexpr int slotOf(uint32_t highTypeMasked)
    {
        switch (highTypeMasked)
        {
            case HIGHGUID_TYPE_UNIT:         return 0;
            case HIGHGUID_TYPE_VEHICLE:      return 1;
            case HIGHGUID_TYPE_GAMEOBJECT:   return 2;
            case HIGHGUID_TYPE_DYNAMICOBJECT:return 3;
            case HIGHGUID_TYPE_PLAYER:       return 4;
            case HIGHGUID_TYPE_PET:          return 5;
            case HIGHGUID_TYPE_CORPSE:       return 6;
            case HIGHGUID_TYPE_TRANSPORTER:  return 7;
            case HIGHGUID_TYPE_TRANSPORT:    return 8;
            default:                         return -1; // fallback bucket
        }
    }

private:
    std::mutex m_mutex;
    uint32_t   m_nextLow[16]{};
    std::deque<uint32_t> m_reuse[16];
};
