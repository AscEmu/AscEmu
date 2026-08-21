/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WoWGuid.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLootStartRoll : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t mapId = 0;
        uint32_t itemSlot = 0;
        uint32_t itemId = 0;
        uint32_t randomSuffixFactor = 0;
        uint32_t randomPropertyOrSuffixId = 0;
        uint32_t itemCount = 0;
        uint32_t countDown = 0;
        uint8_t rollVoteMask = 0;
        uint32_t itemDisplayId = 0;

        SmsgLootStartRoll() : ManagedPacket(SMSG_LOOT_START_ROLL, 0)
        {
        }

        SmsgLootStartRoll(uint64_t guid, uint32_t mapId, uint32_t itemSlot, uint32_t itemId, uint32_t randomSuffixFactor,
            uint32_t randomPropertyOrSuffixId, uint32_t itemCount, uint32_t countDown, uint8_t rollVoteMask, uint32_t itemDisplayId) :
            ManagedPacket(SMSG_LOOT_START_ROLL, 0),
            guid(guid),
            mapId(mapId),
            itemSlot(itemSlot),
            itemId(itemId),
            randomSuffixFactor(randomSuffixFactor),
            randomPropertyOrSuffixId(randomPropertyOrSuffixId),
            itemCount(itemCount),
            countDown(countDown),
            rollVoteMask(rollVoteMask),
            itemDisplayId(itemDisplayId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_protocol.isMop() ? 45 : 32; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBits(7, 3); // unknown, always 7 in reference
                packet.writeBit(false); // loot slot always present
                packet.writeBit(0);

                packet.writeBit(guid[3]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[0]);

                packet.writeBit(1);
                packet.writeBits(3, 2);
                packet.flushBits();

                packet.writeByteSeq(guid[7]);
                packet << randomPropertyOrSuffixId;
                packet.writeByteSeq(guid[5]);
                packet << mapId;
                packet << randomSuffixFactor;
                packet << static_cast<uint8_t>(itemSlot);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[2]);
                packet << static_cast<uint32_t>(0);
                packet << itemId;
                packet << rollVoteMask;
                packet << itemCount;
                packet << static_cast<uint8_t>(0);
                packet << countDown;
                packet.writeByteSeq(guid[6]);
                packet << itemDisplayId;
                packet.writeByteSeq(guid[1]);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << guid.getRawGuid();
                packet << mapId;
                packet << itemSlot;
                packet << itemId;
                packet << randomSuffixFactor;
                packet << randomPropertyOrSuffixId;
                packet << itemCount;
                packet << countDown;
                packet << rollVoteMask;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
