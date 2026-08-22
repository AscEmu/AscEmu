/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgMinimapPing : public ManagedPacket
    {
    public:
        WoWGuid guid;
        float posX;
        float posY;

        SmsgMinimapPing() : SmsgMinimapPing(0, 0, 0)
        {
        }

        SmsgMinimapPing(uint64_t guid, float posX, float posY) :
            ManagedPacket(SMSG_MINIMAP_PING, 17),
            guid(guid),
            posX(posX),
            posY(posY)
        {
        }

    protected:
        size_t expectedSize() const override { return 17; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet << posY << posX;

                packet.writeBit(guid[0]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[4]);
                packet.flushBits();

                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[4]);

                return true;
            }

            return false;
        }
    };
}
