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
    class CmsgAttackSwing : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgAttackSwing() : CmsgAttackSwing(0)
        {
        }

        CmsgAttackSwing(uint64_t guid) :
            ManagedPacket(CMSG_ATTACKSWING, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
            uint64_t unpacked_guid;
            packet >> unpacked_guid;
#else
            WoWGuid unpacked_guid;
            unpacked_guid[6] = packet.readBit();
            unpacked_guid[5] = packet.readBit();
            unpacked_guid[7] = packet.readBit();
            unpacked_guid[0] = packet.readBit();
            unpacked_guid[3] = packet.readBit();
            unpacked_guid[1] = packet.readBit();
            unpacked_guid[4] = packet.readBit();
            unpacked_guid[2] = packet.readBit();

            packet.ReadByteSeq(unpacked_guid[6]);
            packet.ReadByteSeq(unpacked_guid[7]);
            packet.ReadByteSeq(unpacked_guid[1]);
            packet.ReadByteSeq(unpacked_guid[3]);
            packet.ReadByteSeq(unpacked_guid[2]);
            packet.ReadByteSeq(unpacked_guid[0]);
            packet.ReadByteSeq(unpacked_guid[4]);
            packet.ReadByteSeq(unpacked_guid[5]);
#endif
            guid.init(unpacked_guid);
            return true;
        }
    };
}
