/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <map>
#include <utility>

namespace AscEmu::Packets
{
    class SmsgSetActiveMover : public ManagedPacket
    {
    public:
        WoWGuid guid;

        SmsgSetActiveMover(WoWGuid guid) :
            ManagedPacket(SMSG_MOVE_SET_ACTIVE_MOVER, 9),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet.writeBit(guid[5]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[2]);

                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[4]);

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(guid[5]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[6]);

                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[1]);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
