/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSplineMoveRoot : public ManagedPacket
    {
    public:
        WoWGuid guid;

        SmsgSplineMoveRoot() : SmsgSplineMoveRoot(WoWGuid())
        {
        }

        SmsgSplineMoveRoot(WoWGuid guid) :
            ManagedPacket(SMSG_SPLINE_MOVE_ROOT, 4),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << int32_t(0);

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(guid[3]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[1]);

                packet.flushBits();

                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[6]);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
