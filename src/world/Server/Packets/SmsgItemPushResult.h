/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgItemPushResult : public ManagedPacket
    {
    public:
        uint64_t guid;
        WoWGuid itemGuid;
        bool isReceived;
        bool isCreated;
        uint8_t bagSlot;
        uint32_t slot;

        uint32_t entry;
        uint32_t suffix;
        int32_t randomProp;
        uint32_t count;
        uint32_t stackCount;

        SmsgItemPushResult() : SmsgItemPushResult(0, WoWGuid(), false, false, 0, 0, 0, 0, 0, 0, 0)
        {
        }

        SmsgItemPushResult(uint64_t guid, WoWGuid itemGuid, bool isReceived, bool isCreated, uint8_t bagSlot, uint32_t slot,
            uint32_t entry, uint32_t suffix, int32_t randomProp, uint32_t count, uint32_t stackCount) :
            ManagedPacket(SMSG_ITEM_PUSH_RESULT, 0),
            guid(guid),
            itemGuid(itemGuid),
            isReceived(isReceived),
            isCreated(isCreated),
            bagSlot(bagSlot),
            slot(slot),
            entry(entry),
            suffix(suffix),
            randomProp(randomProp),
            count(count),
            stackCount(stackCount)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4 + 4 + 4 + 1 + 4 + 4 + 4 + 4 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const WoWGuid playerGuid(guid);
                const uint32_t itemSlot = slot ? slot : uint32_t(-1);

                packet.writeBit(itemGuid[2]);
                packet.writeBit(playerGuid[4]);
                packet.writeBit(itemGuid[5]);
                packet.writeBit(true);              // display in chat
                packet.writeBit(playerGuid[1]);
                packet.writeBit(isReceived);
                packet.writeBit(itemGuid[4]);
                packet.writeBit(playerGuid[6]);
                packet.writeBit(playerGuid[5]);
                packet.writeBit(playerGuid[7]);
                packet.writeBit(playerGuid[0]);
                packet.writeBit(itemGuid[0]);
                packet.writeBit(itemGuid[7]);
                packet.writeBit(playerGuid[2]);
                packet.writeBit(itemGuid[6]);
                packet.writeBit(false);             // bonus loot
                packet.writeBit(playerGuid[3]);
                packet.writeBit(itemGuid[1]);
                packet.writeBit(isCreated);
                packet.writeBit(itemGuid[3]);
                packet.flushBits();

                packet.writeByteSeq(playerGuid[1]);
                packet.writeByteSeq(itemGuid[1]);
                packet << uint32_t(0);              // battle pet species
                packet.writeByteSeq(itemGuid[0]);
                packet.writeByteSeq(playerGuid[5]);
                packet.writeByteSeq(playerGuid[2]);
                packet << uint32_t(suffix);
                packet.writeByteSeq(itemGuid[7]);
                packet << uint32_t(0);              // battle pet quality
                packet << uint32_t(entry);
                packet << int32_t(randomProp);
                packet.writeByteSeq(itemGuid[6]);
                packet << uint32_t(0);              // battle pet breed
                packet << uint32_t(stackCount);     // count of items in inventory
                packet.writeByteSeq(itemGuid[2]);
                packet.writeByteSeq(playerGuid[0]);
                packet << uint32_t(count);
                packet.writeByteSeq(playerGuid[7]);
                packet.writeByteSeq(itemGuid[5]);
                packet.writeByteSeq(playerGuid[4]);
                packet << uint32_t(itemSlot);
                packet << uint8_t(bagSlot);
                packet.writeByteSeq(playerGuid[3]);
                packet.writeByteSeq(playerGuid[6]);
                packet << uint32_t(0);              // battle pet level
                packet.writeByteSeq(itemGuid[3]);
                packet.writeByteSeq(itemGuid[4]);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << guid << uint32_t(isReceived) << uint32_t(isCreated) << uint32_t(1);
                packet << bagSlot << uint32_t(slot ? slot : -1);
                packet << entry << suffix << randomProp << count << stackCount;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
