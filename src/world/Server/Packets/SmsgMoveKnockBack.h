/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#if VERSION_STRING != Cata
            packet << guid << time << cos << sin << horizontal << vertical;
#else
            ObjectGuid objectGuid = guid;

            WorldPacket data(SMSG_MOVE_KNOCK_BACK, 50);
            data.WriteByteMask(objectGuid[0]);
            data.WriteByteMask(objectGuid[3]);
            data.WriteByteMask(objectGuid[6]);
            data.WriteByteMask(objectGuid[7]);
            data.WriteByteMask(objectGuid[2]);
            data.WriteByteMask(objectGuid[5]);
            data.WriteByteMask(objectGuid[1]);
            data.WriteByteMask(objectGuid[4]);

            data.WriteByteSeq(objectGuid[1]);
            data << float(sin);
            data << uint32_t(0);
            data.WriteByteSeq(objectGuid[6]);
            data.WriteByteSeq(objectGuid[7]);
            data << float(horizontal);
            data.WriteByteSeq(objectGuid[4]);
            data.WriteByteSeq(objectGuid[5]);
            data.WriteByteSeq(objectGuid[3]);
            data << float(-vertical);
            data << float(cos);
            data.WriteByteSeq(objectGuid[2]);
            data.WriteByteSeq(objectGuid[0]);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override { return false; }
    };
}}
