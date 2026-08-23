/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgVoidStorageQuery : public ManagedPacket
    {
    public:
        WoWGuid npcGuid;

        CmsgVoidStorageQuery() :
            ManagedPacket(CMSG_VOID_STORAGE_QUERY, 8)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                npcGuid[1] = packet.readBit();
                npcGuid[5] = packet.readBit();
                npcGuid[6] = packet.readBit();
                npcGuid[0] = packet.readBit();
                npcGuid[7] = packet.readBit();
                npcGuid[2] = packet.readBit();
                npcGuid[3] = packet.readBit();
                npcGuid[4] = packet.readBit();

                packet.readByteSeq(npcGuid[1]);
                packet.readByteSeq(npcGuid[6]);
                packet.readByteSeq(npcGuid[4]);
                packet.readByteSeq(npcGuid[3]);
                packet.readByteSeq(npcGuid[7]);
                packet.readByteSeq(npcGuid[0]);
                packet.readByteSeq(npcGuid[2]);
                packet.readByteSeq(npcGuid[5]);

                return true;
            }
            else if (m_protocol.isCata())
            {
                npcGuid[4] = packet.readBit();
                npcGuid[0] = packet.readBit();
                npcGuid[5] = packet.readBit();
                npcGuid[7] = packet.readBit();
                npcGuid[6] = packet.readBit();
                npcGuid[3] = packet.readBit();
                npcGuid[1] = packet.readBit();
                npcGuid[2] = packet.readBit();

                packet.readByteSeq(npcGuid[5]);
                packet.readByteSeq(npcGuid[6]);
                packet.readByteSeq(npcGuid[3]);
                packet.readByteSeq(npcGuid[7]);
                packet.readByteSeq(npcGuid[1]);
                packet.readByteSeq(npcGuid[0]);
                packet.readByteSeq(npcGuid[4]);
                packet.readByteSeq(npcGuid[2]);

                return true;
            }

            return false;
        }
    };
}
