/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/ByteBuffer.hpp"
#include "WoWGuid.hpp"

#include <atomic>
#include <cstdint>

namespace AscEmu::Packets::ForeverLootPacket
{
    inline WoWGuid makeOwnerGuid(uint64_t legacyGuid, uint32_t realmId, uint16_t mapId)
    {
        return WoWGuid::createModernFromLegacy(legacyGuid, realmId, mapId, 0);
    }

    inline uint64_t nextLootObjectCounter()
    {
        static std::atomic<uint64_t> counter{1};
        const uint64_t value = counter.fetch_add(1, std::memory_order_relaxed) & UINT64_C(0xFFFFFFFFFF);
        return value == 0 ? 1 : value;
    }

    inline WoWGuid makeLootObjectGuid(uint64_t legacyGuid, uint32_t realmId, uint16_t mapId, uint64_t counter)
    {
        const WoWGuid owner = makeOwnerGuid(legacyGuid, realmId, mapId);
        const uint64_t high = (uint64_t(ModernHighGuid::LootObject) << 58U) | (uint64_t(realmId & 0x1FFFU) << 42U);
        const uint64_t low = (uint64_t(owner.getModernServerId() & 0xFFFFFFU) << 40U) | (counter & UINT64_C(0xFFFFFFFFFF));
        return WoWGuid::createModern(high, low);
    }

    inline void writeGuid(ByteBuffer& data, WoWGuid const& guid)
    {
        const auto packed = guid.packModern();
        data.append(packed.data(), packed.size());
    }

    inline void writeItemInstance(ByteBuffer& data, uint32_t itemId)
    {
        data << static_cast<int32_t>(itemId);
        data.writeBits(0U, 7);
        data.flushBits();
        data.writeBit(false);
        data.flushBits();
    }
}
