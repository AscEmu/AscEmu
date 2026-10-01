/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../ObjectUpdate.hpp"

#include "Network/ByteBuffer.hpp"

#include <limits>

namespace AscEmu::Version::Forever::ObjectUpdate
{
    std::vector<uint8_t> buildUpdateObjectPacket(uint16_t mapId, uint32_t updateCount, std::span<const uint8_t> updateBlocks, uint32_t destroyCount, std::span<const uint8_t> destroyGuids, uint32_t outOfRangeCount, std::span<const uint8_t> outOfRangeGuids)
    {
        const uint64_t totalRemovalCount64 = static_cast<uint64_t>(destroyCount) + outOfRangeCount;
        if (totalRemovalCount64 > std::numeric_limits<uint32_t>::max())
            return {};

        const uint32_t totalRemovalCount = static_cast<uint32_t>(totalRemovalCount64);
        if (updateCount == 0 && totalRemovalCount == 0)
            return {};
        if (updateCount != 0 && updateBlocks.empty())
            return {};
        if (destroyCount != 0 && destroyGuids.empty())
            return {};
        if (outOfRangeCount != 0 && outOfRangeGuids.empty())
            return {};
        if (destroyCount > std::numeric_limits<uint16_t>::max())
            return {};

        ByteBuffer packet;
        packet << mapId << updateCount;
        packet.writeBit(1); // UpdateData header flag
        packet.writeBit(totalRemovalCount != 0);
        packet.flushBits();

        if (totalRemovalCount != 0)
        {
            // Modern retail uses one removal list. The first DestroyCount GUIDs
            // are hard destroys; the remaining GUIDs are normal out-of-range
            // removals. Keep the two queues separate internally and concatenate
            // them only on the wire.
            packet << static_cast<uint16_t>(destroyCount);
            packet << totalRemovalCount;
            if (!destroyGuids.empty())
                packet.append(destroyGuids.data(), destroyGuids.size());
            if (!outOfRangeGuids.empty())
                packet.append(outOfRangeGuids.data(), outOfRangeGuids.size());
        }

        packet << uint32_t(updateBlocks.size());
        if (!updateBlocks.empty())
            packet.append(updateBlocks.data(), updateBlocks.size());

        return std::vector<uint8_t>(packet.contents(), packet.contents() + packet.size());
    }

    std::vector<uint8_t> buildSelfCreatePacket(uint16_t mapId, std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, std::span<const uint8_t> fieldPayload)
    {
        const std::vector<uint8_t> block = buildSelfCreateBlock(packedGuid, x, y, z, orientation, fieldPayload);
        if (block.empty())
            return {};
        return buildUpdateObjectPacket(mapId, 1, std::span<const uint8_t>(block.data(), block.size()), 0, {}, 0, {});
    }
}
