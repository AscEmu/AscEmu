/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
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
#if VERSION_STRING < Cata
            packet << guid << time << cos << sin << horizontal << vertical;
#else
            ObjectGuid objectGuid = guid.GetOldGuid();

            packet.WriteByteMask(objectGuid[0]);
            packet.WriteByteMask(objectGuid[3]);
            packet.WriteByteMask(objectGuid[6]);
            packet.WriteByteMask(objectGuid[7]);
            packet.WriteByteMask(objectGuid[2]);
            packet.WriteByteMask(objectGuid[5]);
            packet.WriteByteMask(objectGuid[1]);
            packet.WriteByteMask(objectGuid[4]);

            packet.WriteByteSeq(objectGuid[1]);
            packet << float(sin);
            packet << uint32_t(0);
            packet.WriteByteSeq(objectGuid[6]);
            packet.WriteByteSeq(objectGuid[7]);
            packet << float(horizontal);
            packet.WriteByteSeq(objectGuid[4]);
            packet.WriteByteSeq(objectGuid[5]);
            packet.WriteByteSeq(objectGuid[3]);
            packet << float(-vertical);
            packet << float(cos);
            packet.WriteByteSeq(objectGuid[2]);
            packet.WriteByteSeq(objectGuid[0]);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
