/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class SmsgShowBank : public ManagedPacket
    {
    public:
        uint64_t guid;

        SmsgShowBank() : SmsgShowBank(0)
        {
        }

        SmsgShowBank(uint64_t guid) :
            ManagedPacket(SMSG_SHOW_BANK, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            // data.writeBit(guid[2]);
            // data.writeBit(guid[4]);
            // data.writeBit(guid[3]);
            // data.writeBit(guid[6]);
            // data.writeBit(guid[5]);
            // data.writeBit(guid[1]);
            // data.writeBit(guid[7]);
            // data.writeBit(guid[0]);

            // data.WriteByteSeq(guid[7]);
            // data.WriteByteSeq(guid[0]);
            // data.WriteByteSeq(guid[5]);
            // data.WriteByteSeq(guid[3]);
            // data.WriteByteSeq(guid[6]);
            // data.WriteByteSeq(guid[1]);
            // data.WriteByteSeq(guid[4]);
            // data.WriteByteSeq(guid[2]);

            // SendPacket(&data);
            return false;
        }
    };
}
