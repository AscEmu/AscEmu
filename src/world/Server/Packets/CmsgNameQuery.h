/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgNameQuery : public ManagedPacket
    {
    public:
        WoWGuid guid;
        bool hasVirtualRealm = false;  // bit14 
        bool hasNativeRealm = false;  // bit1C
        uint32_t virtualRealmId = 0;
        uint32_t nativeRealmId = 0;

        CmsgNameQuery() : ManagedPacket(CMSG_NAME_QUERY, 0)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Mop
            // CMSGis only received by the server, not sent. Therefore, no serialization is necessary for MoP here.
            return false;
#else
            packet << guid.getRawGuid();
            return true;
#endif
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Mop
            // Reading the GUID bitmask
            guid[4] = packet.readBit();
            hasVirtualRealm = packet.readBit();
            guid[6] = packet.readBit();
            guid[0] = packet.readBit();
            guid[7] = packet.readBit();
            guid[1] = packet.readBit();
            hasNativeRealm = packet.readBit();
            guid[5] = packet.readBit();
            guid[2] = packet.readBit();
            guid[3] = packet.readBit();

            // Reading the GUID bytes
            packet.ReadByteSeq(guid[7]);
            packet.ReadByteSeq(guid[5]);
            packet.ReadByteSeq(guid[1]);
            packet.ReadByteSeq(guid[2]);
            packet.ReadByteSeq(guid[6]);
            packet.ReadByteSeq(guid[3]);
            packet.ReadByteSeq(guid[0]);
            packet.ReadByteSeq(guid[4]);

            // virtual and native realm addresses
            if (hasVirtualRealm)
                packet >> virtualRealmId;

            if (hasNativeRealm)
                packet >> nativeRealmId;

            return true;
#else
            uint64_t unpacked_guid;
            packet >> unpacked_guid;
            guid.init(unpacked_guid);
            return true;
#endif
        }
    };
}
