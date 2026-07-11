/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

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
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
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
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[4]);

                // virtual and native realm addresses
                if (hasVirtualRealm)
                    packet >> virtualRealmId;

                if (hasNativeRealm)
                    packet >> nativeRealmId;

                return true;
            }
            else
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid;
                guid.init(unpacked_guid);
                return true;
            }
        }
    };
}
