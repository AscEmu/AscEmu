/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/ItemInterface.h"
#include "Objects/Units/Players/Player.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgVoidStorageContents : public ManagedPacket
    {
    public:
        Player* player;

        SmsgVoidStorageContents() : SmsgVoidStorageContents(nullptr)
        {
        }

        SmsgVoidStorageContents(Player* player) :
            ManagedPacket(SMSG_VOID_STORAGE_CONTENTS, 0),
            player(player)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (player == nullptr)
                return 0;

            uint8_t count = 0;
            for (uint8_t i = 0; i < VOID_STORAGE_MAX_SLOT; ++i)
                if (player->getVoidStorageItem(i))
                    ++count;

            return 2 * count + (14 + 4 + 4 + 4 + 4) * count;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (player == nullptr)
                return false;

            if (m_protocol.isCata())
            {
                uint8_t count = 0;
                for (uint8_t i = 0; i < VOID_STORAGE_MAX_SLOT; ++i)
                    if (player->getVoidStorageItem(i))
                        ++count;

                packet.writeBits(count, 8);

                ByteBuffer itemData((14 + 4 + 4 + 4 + 4) * count);

                for (uint8_t i = 0; i < VOID_STORAGE_MAX_SLOT; ++i)
                {
                    VoidStorageItem* item = player->getVoidStorageItem(i);
                    if (!item)
                        continue;

                    WoWGuid itemId = item->itemId;
                    WoWGuid creatorGuid = item->creatorGuid;

                    packet.writeBit(creatorGuid[3]);
                    packet.writeBit(itemId[5]);
                    packet.writeBit(creatorGuid[6]);
                    packet.writeBit(creatorGuid[1]);
                    packet.writeBit(itemId[1]);
                    packet.writeBit(itemId[3]);
                    packet.writeBit(itemId[6]);
                    packet.writeBit(creatorGuid[5]);
                    packet.writeBit(creatorGuid[2]);
                    packet.writeBit(itemId[2]);
                    packet.writeBit(creatorGuid[4]);
                    packet.writeBit(itemId[0]);
                    packet.writeBit(itemId[4]);
                    packet.writeBit(itemId[7]);
                    packet.writeBit(creatorGuid[0]);
                    packet.writeBit(creatorGuid[7]);

                    itemData.writeByteSeq(creatorGuid[3]);

                    itemData << uint32_t(item->itemSuffixFactor);

                    itemData.writeByteSeq(creatorGuid[4]);

                    itemData << uint32_t(i);

                    itemData.writeByteSeq(itemId[0]);
                    itemData.writeByteSeq(itemId[6]);
                    itemData.writeByteSeq(creatorGuid[0]);

                    itemData << uint32_t(item->itemRandomPropertyId);

                    itemData.writeByteSeq(itemId[4]);
                    itemData.writeByteSeq(itemId[5]);
                    itemData.writeByteSeq(itemId[2]);
                    itemData.writeByteSeq(creatorGuid[2]);
                    itemData.writeByteSeq(creatorGuid[6]);
                    itemData.writeByteSeq(itemId[1]);
                    itemData.writeByteSeq(itemId[3]);
                    itemData.writeByteSeq(creatorGuid[5]);
                    itemData.writeByteSeq(creatorGuid[7]);

                    itemData << uint32_t(item->itemEntry);

                    itemData.writeByteSeq(itemId[7]);
                }

                packet.flushBits();
                packet.append(itemData);

                return true;
            }
            else if (m_protocol.isMop())
            {
                uint8_t count = 0;
                for (uint8_t i = 0; i < VOID_STORAGE_MAX_SLOT; ++i)
                    if (player->getVoidStorageItem(i))
                        ++count;

                packet.writeBits(count, 7);

                ByteBuffer itemData((14 + 4 + 4 + 4 + 4) * count);

                for (uint8_t i = 0; i < VOID_STORAGE_MAX_SLOT; ++i)
                {
                    VoidStorageItem* item = player->getVoidStorageItem(i);
                    if (!item)
                        continue;

                    WoWGuid itemId = item->itemId;
                    WoWGuid creatorGuid = item->creatorGuid;

                    packet.writeBit(creatorGuid[1]);
                    packet.writeBit(creatorGuid[3]);
                    packet.writeBit(itemId[1]);
                    packet.writeBit(creatorGuid[2]);
                    packet.writeBit(itemId[2]);
                    packet.writeBit(creatorGuid[2]);
                    packet.writeBit(creatorGuid[0]);
                    packet.writeBit(itemId[6]);
                    packet.writeBit(itemId[5]);
                    packet.writeBit(creatorGuid[2]);
                    packet.writeBit(itemId[7]);
                    packet.writeBit(itemId[3]);
                    packet.writeBit(itemId[4]);
                    packet.writeBit(itemId[0]);
                    packet.writeBit(creatorGuid[6]);
                    packet.writeBit(creatorGuid[7]);

                    itemData.writeByteSeq(creatorGuid[4]);
                    itemData.writeByteSeq(creatorGuid[7]);
                    itemData.writeByteSeq(itemId[6]);
                    itemData.writeByteSeq(creatorGuid[6]);
                    itemData.writeByteSeq(itemId[2]);

                    itemData << uint32_t(item->itemSuffixFactor);

                    itemData.writeByteSeq(itemId[7]);
                    itemData.writeByteSeq(itemId[3]);
                    itemData.writeByteSeq(creatorGuid[0]);

                    itemData << uint32_t(0);    // unk upgrade?

                    itemData.writeByteSeq(itemId[0]);

                    itemData << uint32_t(item->itemRandomPropertyId);

                    itemData.writeByteSeq(creatorGuid[2]);
                    itemData.writeByteSeq(creatorGuid[5]);
                    itemData.writeByteSeq(creatorGuid[3]);

                    itemData << uint32_t(item->itemEntry);

                    itemData.writeByteSeq(itemId[5]);
                    itemData.writeByteSeq(itemId[1]);

                    itemData << uint32_t(i);

                    itemData.writeByteSeq(itemId[4]);
                    itemData.writeByteSeq(creatorGuid[1]);
                }

                packet.flushBits();
                packet.append(itemData);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
