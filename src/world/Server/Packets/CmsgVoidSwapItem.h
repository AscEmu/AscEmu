/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgVoidSwapItem : public ManagedPacket
    {
    public:
        uint32_t newSlot = 0;
        WoWGuid npcGuid;
        WoWGuid itemId;

        CmsgVoidSwapItem() :
            ManagedPacket(CMSG_VOID_SWAP_ITEM, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> newSlot;

            if (m_protocol.isMop())
            {
                npcGuid[6] = packet.readBit();
                itemId[4] = packet.readBit();
                itemId[7] = packet.readBit();
                itemId[3] = packet.readBit();
                itemId[2] = packet.readBit();
                npcGuid[4] = packet.readBit();
                npcGuid[2] = packet.readBit();
                itemId[0] = packet.readBit();
                itemId[1] = packet.readBit();
                npcGuid[7] = packet.readBit();
                npcGuid[1] = packet.readBit();
                itemId[6] = packet.readBit();
                npcGuid[3] = packet.readBit();
                npcGuid[5] = packet.readBit();
                itemId[5] = packet.readBit();
                npcGuid[0] = packet.readBit();

                packet.readByteSeq(npcGuid[3]);
                packet.readByteSeq(npcGuid[5]);
                packet.readByteSeq(itemId[6]);
                packet.readByteSeq(npcGuid[4]);
                packet.readByteSeq(itemId[4]);
                packet.readByteSeq(npcGuid[0]);
                packet.readByteSeq(itemId[5]);
                packet.readByteSeq(itemId[7]);
                packet.readByteSeq(npcGuid[7]);
                packet.readByteSeq(npcGuid[2]);
                packet.readByteSeq(npcGuid[1]);
                packet.readByteSeq(itemId[1]);
                packet.readByteSeq(itemId[3]);
                packet.readByteSeq(npcGuid[6]);
                packet.readByteSeq(itemId[0]);
                packet.readByteSeq(itemId[2]);

                return true;
            }
            else if (m_protocol.isCata())
            {
                npcGuid[2] = packet.readBit();
                npcGuid[4] = packet.readBit();
                npcGuid[0] = packet.readBit();
                itemId[2] = packet.readBit();
                itemId[6] = packet.readBit();
                itemId[5] = packet.readBit();
                npcGuid[1] = packet.readBit();
                npcGuid[7] = packet.readBit();
                itemId[3] = packet.readBit();
                itemId[7] = packet.readBit();
                itemId[0] = packet.readBit();
                npcGuid[6] = packet.readBit();
                npcGuid[5] = packet.readBit();
                npcGuid[3] = packet.readBit();
                itemId[1] = packet.readBit();
                itemId[4] = packet.readBit();

                packet.readByteSeq(npcGuid[1]);
                packet.readByteSeq(itemId[3]);
                packet.readByteSeq(itemId[2]);
                packet.readByteSeq(itemId[4]);
                packet.readByteSeq(npcGuid[3]);
                packet.readByteSeq(npcGuid[0]);
                packet.readByteSeq(itemId[6]);
                packet.readByteSeq(itemId[1]);
                packet.readByteSeq(npcGuid[5]);
                packet.readByteSeq(itemId[5]);
                packet.readByteSeq(npcGuid[6]);
                packet.readByteSeq(itemId[0]);
                packet.readByteSeq(npcGuid[2]);
                packet.readByteSeq(npcGuid[7]);
                packet.readByteSeq(npcGuid[4]);
                packet.readByteSeq(itemId[7]);

                return true;
            }

            return false;
        }
    };
}
