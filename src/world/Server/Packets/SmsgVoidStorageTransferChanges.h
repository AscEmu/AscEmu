/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/ItemInterface.h"
#include <cstdint>
#include <utility>

namespace AscEmu::Packets
{
    class SmsgVoidStorageTransferChanges : public ManagedPacket
    {
    public:
        const std::pair<VoidStorageItem, uint8_t>* depositItems;
        uint8_t depositCount;
        const VoidStorageItem* withdrawItems;
        uint8_t withdrawCount;

        SmsgVoidStorageTransferChanges() : SmsgVoidStorageTransferChanges(nullptr, 0, nullptr, 0)
        {
        }

        SmsgVoidStorageTransferChanges(const std::pair<VoidStorageItem, uint8_t>* depositItems, uint8_t depositCount,
            const VoidStorageItem* withdrawItems, uint8_t withdrawCount) :
            ManagedPacket(SMSG_VOID_STORAGE_TRANSFER_CHANGES, 0),
            depositItems(depositItems), depositCount(depositCount),
            withdrawItems(withdrawItems), withdrawCount(withdrawCount)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return ((5 + 5 + (7 + 7) * depositCount + 7 * withdrawCount) / 8)
                + 7 * withdrawCount + (7 + 7 + 4 * 4) * depositCount;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet.writeBits(depositCount, 5);
                packet.writeBits(withdrawCount, 5);

                for (uint8_t i = 0; i < depositCount; ++i)
                {
                    WoWGuid itemId = depositItems[i].first.itemId;
                    WoWGuid creatorGuid = depositItems[i].first.creatorGuid;
                    packet.writeBit(creatorGuid[7]);
                    packet.writeBit(itemId[7]);
                    packet.writeBit(itemId[4]);
                    packet.writeBit(creatorGuid[6]);
                    packet.writeBit(creatorGuid[5]);
                    packet.writeBit(itemId[3]);
                    packet.writeBit(itemId[5]);
                    packet.writeBit(creatorGuid[4]);
                    packet.writeBit(creatorGuid[2]);
                    packet.writeBit(creatorGuid[0]);
                    packet.writeBit(creatorGuid[3]);
                    packet.writeBit(creatorGuid[1]);
                    packet.writeBit(itemId[2]);
                    packet.writeBit(itemId[0]);
                    packet.writeBit(itemId[1]);
                    packet.writeBit(itemId[6]);
                }

                for (uint8_t i = 0; i < withdrawCount; ++i)
                {
                    WoWGuid itemId = withdrawItems[i].itemId;
                    packet.writeBit(itemId[1]);
                    packet.writeBit(itemId[7]);
                    packet.writeBit(itemId[3]);
                    packet.writeBit(itemId[5]);
                    packet.writeBit(itemId[6]);
                    packet.writeBit(itemId[2]);
                    packet.writeBit(itemId[4]);
                    packet.writeBit(itemId[0]);
                }

                packet.flushBits();

                for (uint8_t i = 0; i < withdrawCount; ++i)
                {
                    WoWGuid itemId = withdrawItems[i].itemId;
                    packet.writeByteSeq(itemId[3]);
                    packet.writeByteSeq(itemId[1]);
                    packet.writeByteSeq(itemId[0]);
                    packet.writeByteSeq(itemId[2]);
                    packet.writeByteSeq(itemId[7]);
                    packet.writeByteSeq(itemId[5]);
                    packet.writeByteSeq(itemId[6]);
                    packet.writeByteSeq(itemId[4]);
                }

                for (uint8_t i = 0; i < depositCount; ++i)
                {
                    WoWGuid itemId = depositItems[i].first.itemId;
                    WoWGuid creatorGuid = depositItems[i].first.creatorGuid;

                    packet << uint32_t(depositItems[i].first.itemSuffixFactor);

                    packet.writeByteSeq(itemId[6]);
                    packet.writeByteSeq(itemId[4]);
                    packet.writeByteSeq(creatorGuid[4]);
                    packet.writeByteSeq(itemId[2]);
                    packet.writeByteSeq(creatorGuid[1]);
                    packet.writeByteSeq(creatorGuid[3]);
                    packet.writeByteSeq(itemId[3]);
                    packet.writeByteSeq(creatorGuid[0]);
                    packet.writeByteSeq(itemId[0]);
                    packet.writeByteSeq(creatorGuid[6]);
                    packet.writeByteSeq(itemId[5]);
                    packet.writeByteSeq(creatorGuid[5]);
                    packet.writeByteSeq(creatorGuid[7]);

                    packet << uint32_t(depositItems[i].first.itemEntry);

                    packet.writeByteSeq(itemId[1]);

                    packet << uint32_t(depositItems[i].second); // slot

                    packet.writeByteSeq(creatorGuid[2]);
                    packet.writeByteSeq(itemId[7]);

                    packet << uint32_t(depositItems[i].first.itemRandomPropertyId);
                }

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBits(withdrawCount, 4);

                for (uint8_t i = 0; i < withdrawCount; ++i)
                {
                    WoWGuid itemId = withdrawItems[i].itemId;
                    packet.writeBit(itemId[1]);
                    packet.writeBit(itemId[6]);
                    packet.writeBit(itemId[7]);
                    packet.writeBit(itemId[3]);
                    packet.writeBit(itemId[2]);
                    packet.writeBit(itemId[0]);
                    packet.writeBit(itemId[4]);
                    packet.writeBit(itemId[5]);
                }

                packet.writeBits(depositCount, 4);

                for (uint8_t i = 0; i < depositCount; ++i)
                {
                    WoWGuid itemId = depositItems[i].first.itemId;
                    WoWGuid creatorGuid = depositItems[i].first.creatorGuid;
                    packet.writeBit(itemId[0]);
                    packet.writeBit(creatorGuid[6]);
                    packet.writeBit(creatorGuid[4]);
                    packet.writeBit(itemId[3]);
                    packet.writeBit(creatorGuid[3]);
                    packet.writeBit(itemId[5]);
                    packet.writeBit(itemId[7]);
                    packet.writeBit(creatorGuid[0]);
                    packet.writeBit(creatorGuid[5]);
                    packet.writeBit(creatorGuid[7]);
                    packet.writeBit(itemId[6]);
                    packet.writeBit(itemId[4]);
                    packet.writeBit(creatorGuid[1]);
                    packet.writeBit(itemId[1]);
                    packet.writeBit(creatorGuid[2]);
                }

                packet.flushBits();

                for (uint8_t i = 0; i < depositCount; ++i)
                {
                    WoWGuid itemId = depositItems[i].first.itemId;
                    WoWGuid creatorGuid = depositItems[i].first.creatorGuid;

                    packet << uint32_t(depositItems[i].second); // slot

                    packet.writeByteSeq(creatorGuid[5]);

                    packet << uint32_t(depositItems[i].first.itemEntry);

                    packet.writeByteSeq(creatorGuid[6]);
                    packet.writeByteSeq(creatorGuid[3]);

                    packet << uint32_t(depositItems[i].first.itemSuffixFactor);

                    packet.writeByteSeq(creatorGuid[2]);
                    packet.writeByteSeq(itemId[5]);

                    packet << uint32_t(depositItems[i].first.itemRandomPropertyId);

                    packet.writeByteSeq(itemId[3]);
                    packet.writeByteSeq(creatorGuid[7]);
                    packet.writeByteSeq(creatorGuid[4]);
                    packet.writeByteSeq(creatorGuid[1]);
                    packet.writeByteSeq(itemId[0]);
                    packet.writeByteSeq(itemId[4]);
                    packet.writeByteSeq(itemId[6]);

                    packet << uint32_t(0); // unk upgrade?
                    
                    packet.writeByteSeq(itemId[1]);
                    packet.writeByteSeq(itemId[2]);
                    packet.writeByteSeq(creatorGuid[0]);
                    packet.writeByteSeq(itemId[7]);
                }

                for (uint8_t i = 0; i < withdrawCount; ++i)
                {
                    WoWGuid itemId = withdrawItems[i].itemId;
                    packet.writeByteSeq(itemId[7]);
                    packet.writeByteSeq(itemId[3]);
                    packet.writeByteSeq(itemId[1]);
                    packet.writeByteSeq(itemId[5]);
                    packet.writeByteSeq(itemId[4]);
                    packet.writeByteSeq(itemId[0]);
                    packet.writeByteSeq(itemId[6]);
                    packet.writeByteSeq(itemId[2]);
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
