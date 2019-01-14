/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgItemPushResult : public ManagedPacket
    {
    public:
        uint64_t guid;
        bool isReceived;
        bool isCreated;
        uint8_t bagSlot;
        uint32_t slot;

        uint32_t entry;
        uint32_t suffix;
        int32_t randomProp;
        uint32_t count;
        uint32_t stackCount;

        SmsgItemPushResult() : SmsgItemPushResult(0, false, false, 0, 0, 0, 0, 0, 0, 0)
        {
        }

        SmsgItemPushResult(uint64_t guid, bool isReceived, bool isCreated, uint8_t bagSlot, uint32_t slot,
            uint32_t entry, uint32_t suffix, int32_t randomProp, uint32_t count, uint32_t stackCount) :
            ManagedPacket(SMSG_ITEM_PUSH_RESULT, 0),
            guid(guid),
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
            packet << guid << uint32_t(isReceived) << uint32_t(isCreated) << uint32_t(1);
            packet << bagSlot << uint32_t(slot ? slot : -1);
            packet << entry << suffix << randomProp << count << stackCount;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
