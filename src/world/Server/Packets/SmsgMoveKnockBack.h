/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgMoveKnockBack : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t time;
        float cos;
        float sin;
        float horizontal;
        float vertical;

        SmsgMoveKnockBack() : SmsgMoveKnockBack(WoWGuid(), 0, 0, 0, 0, 0)
        {
        }

        SmsgMoveKnockBack(WoWGuid guid, uint32_t time, float cos, float sin, float horizontal, float vertical) :
            ManagedPacket(SMSG_MOVE_KNOCK_BACK, 0),
            guid(guid),
            time(time),
            cos(cos),
            sin(sin),
            horizontal(horizontal),
            vertical(vertical)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4 + 4 + 4 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << guid << time << cos << sin << horizontal << vertical;
            }
            else
            {
                WoWGuid objectGuid = guid.getRawGuid();

                packet.writeByteMask(objectGuid[0]);
                packet.writeByteMask(objectGuid[3]);
                packet.writeByteMask(objectGuid[6]);
                packet.writeByteMask(objectGuid[7]);
                packet.writeByteMask(objectGuid[2]);
                packet.writeByteMask(objectGuid[5]);
                packet.writeByteMask(objectGuid[1]);
                packet.writeByteMask(objectGuid[4]);

                packet.writeByteSeq(objectGuid[1]);
                packet << float(sin);
                packet << uint32_t(0);
                packet.writeByteSeq(objectGuid[6]);
                packet.writeByteSeq(objectGuid[7]);
                packet << float(horizontal);
                packet.writeByteSeq(objectGuid[4]);
                packet.writeByteSeq(objectGuid[5]);
                packet.writeByteSeq(objectGuid[3]);
                packet << float(-vertical);
                packet << float(cos);
                packet.writeByteSeq(objectGuid[2]);
                packet.writeByteSeq(objectGuid[0]);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
