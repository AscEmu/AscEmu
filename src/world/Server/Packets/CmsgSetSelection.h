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
    class CmsgSetSelection : public ManagedPacket
    {
    public:
        uint64_t guid;

        CmsgSetSelection() : CmsgSetSelection(0)
        {
        }

        CmsgSetSelection(uint64_t guid) :
            ManagedPacket(CMSG_SET_SELECTION, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
            packet >> guid;
#else
            WoWGuid rawGuid;
            rawGuid[7] = packet.readBit();
            rawGuid[6] = packet.readBit();
            rawGuid[5] = packet.readBit();
            rawGuid[4] = packet.readBit();
            rawGuid[3] = packet.readBit();
            rawGuid[2] = packet.readBit();
            rawGuid[1] = packet.readBit();
            rawGuid[0] = packet.readBit();
            packet.ReadByteSeq(rawGuid[0]);
            packet.ReadByteSeq(rawGuid[7]);
            packet.ReadByteSeq(rawGuid[3]);
            packet.ReadByteSeq(rawGuid[5]);
            packet.ReadByteSeq(rawGuid[1]);
            packet.ReadByteSeq(rawGuid[4]);
            packet.ReadByteSeq(rawGuid[6]);
            packet.ReadByteSeq(rawGuid[2]);

            guid = rawGuid.getRawGuid();
#endif
            return true;
        }
    };
}
