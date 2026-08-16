/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgVoidItemSwapResponse : public ManagedPacket
    {
    public:
        bool usedSrcSlot;
        bool usedDestSlot;
        WoWGuid itemId;
        WoWGuid itemIdDest;
        uint8_t oldSlot;
        uint32_t newSlot;

        SmsgVoidItemSwapResponse() : SmsgVoidItemSwapResponse(false, false, WoWGuid(), WoWGuid(), 0, 0)
        {
        }

        SmsgVoidItemSwapResponse(bool usedSrcSlot, bool usedDestSlot, WoWGuid itemId, WoWGuid itemIdDest, uint8_t oldSlot, uint32_t newSlot) :
            ManagedPacket(SMSG_VOID_ITEM_SWAP_RESPONSE, 0),
            usedSrcSlot(usedSrcSlot), usedDestSlot(usedDestSlot),
            itemId(itemId), itemIdDest(itemIdDest),
            oldSlot(oldSlot), newSlot(newSlot)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 1 + (usedSrcSlot ? sizeof(itemId) : 0) + (usedDestSlot ? sizeof(itemIdDest) : 0)
                + sizeof(oldSlot) + sizeof(newSlot);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet.writeBit(!usedDestSlot);
                packet.writeBit(!usedSrcSlot);

                if (usedSrcSlot)
                {
                    packet.writeBit(itemId[5]);
                    packet.writeBit(itemId[2]);
                    packet.writeBit(itemId[1]);
                    packet.writeBit(itemId[4]);
                    packet.writeBit(itemId[0]);
                    packet.writeBit(itemId[6]);
                    packet.writeBit(itemId[7]);
                    packet.writeBit(itemId[3]);
                }

                packet.writeBit(!usedDestSlot); // unk

                if (usedDestSlot)
                {
                    packet.writeBit(itemIdDest[7]);
                    packet.writeBit(itemIdDest[3]);
                    packet.writeBit(itemIdDest[4]);
                    packet.writeBit(itemIdDest[0]);
                    packet.writeBit(itemIdDest[5]);
                    packet.writeBit(itemIdDest[1]);
                    packet.writeBit(itemIdDest[2]);
                    packet.writeBit(itemIdDest[6]);
                }

                packet.writeBit(!usedSrcSlot); // unk

                packet.flushBits();

                if (usedDestSlot)
                {
                    packet.writeByteSeq(itemIdDest[4]);
                    packet.writeByteSeq(itemIdDest[6]);
                    packet.writeByteSeq(itemIdDest[5]);
                    packet.writeByteSeq(itemIdDest[2]);
                    packet.writeByteSeq(itemIdDest[3]);
                    packet.writeByteSeq(itemIdDest[1]);
                    packet.writeByteSeq(itemIdDest[7]);
                    packet.writeByteSeq(itemIdDest[0]);
                }

                if (usedSrcSlot)
                {
                    packet.writeByteSeq(itemId[6]);
                    packet.writeByteSeq(itemId[3]);
                    packet.writeByteSeq(itemId[5]);
                    packet.writeByteSeq(itemId[0]);
                    packet.writeByteSeq(itemId[1]);
                    packet.writeByteSeq(itemId[2]);
                    packet.writeByteSeq(itemId[4]);
                    packet.writeByteSeq(itemId[7]);
                }

                if (usedDestSlot)
                    packet << uint32_t(oldSlot);

                if (usedSrcSlot)
                    packet << uint32_t(newSlot);

                return true;
            }
            else if (m_protocol.isMop())
            {
                // writeByteSeq() already skips emitting
                // an actual byte when the value is 0, so this is safe (and correct) even
                // when one of the guids is a default/empty WoWGuid.
                packet.writeBit(!usedSrcSlot);

                packet.writeBit(itemId[4]);
                packet.writeBit(itemId[1]);
                packet.writeBit(itemId[6]);
                packet.writeBit(itemId[0]);
                packet.writeBit(itemId[3]);
                packet.writeBit(itemId[7]);
                packet.writeBit(itemId[2]);
                packet.writeBit(itemId[5]);

                packet.writeBit(!usedDestSlot);

                packet.writeBit(itemIdDest[6]);
                packet.writeBit(itemIdDest[0]);
                packet.writeBit(itemIdDest[3]);
                packet.writeBit(itemIdDest[2]);
                packet.writeBit(itemIdDest[1]);
                packet.writeBit(itemIdDest[5]);
                packet.writeBit(itemIdDest[7]);
                packet.writeBit(itemIdDest[4]);

                packet.writeBit(!false); // unk 
                packet.writeBit(!usedDestSlot); // unk

                packet.flushBits();

                packet.writeByteSeq(itemIdDest[3]);
                packet.writeByteSeq(itemIdDest[7]);
                packet.writeByteSeq(itemIdDest[2]);
                packet.writeByteSeq(itemIdDest[5]);
                packet.writeByteSeq(itemIdDest[0]);
                packet.writeByteSeq(itemIdDest[1]);
                packet.writeByteSeq(itemIdDest[4]);
                packet.writeByteSeq(itemIdDest[6]);

                packet.writeByteSeq(itemId[0]);
                packet.writeByteSeq(itemId[2]);
                packet.writeByteSeq(itemId[7]);
                packet.writeByteSeq(itemId[5]);
                packet.writeByteSeq(itemId[6]);
                packet.writeByteSeq(itemId[4]);
                packet.writeByteSeq(itemId[3]);
                packet.writeByteSeq(itemId[1]);

                if (usedDestSlot)
                    packet << uint32_t(newSlot);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
