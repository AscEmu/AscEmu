/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <vector>
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
        std::vector<WoWGuid> guids;     // 9.x: every requested player

        CmsgNameQuery() : ManagedPacket(CMSG_NAME_QUERY, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isShadowlands() || m_protocol.isDragonflight())
            {
                // 9.x asks for several players at once, the first one takes the single guid
                const uint32_t count = packet.read<uint32_t>();
                if (count == 0 || count > 50)
                    return false;

                for (uint32_t i = 0; i < count; ++i)
                {
                    WoWGuid128 clientGuid;
                    packet >> clientGuid;
                    guids.push_back(WoWGuid::fromGuid128(clientGuid));
                }

                guid = guids.front();
                return !packet.hadReadFailure();
            }

            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA())
            {
                WoWGuid128 clientGuid;
                packet >> clientGuid;
                guid = WoWGuid::fromGuid128(clientGuid);
                return !packet.hadReadFailure();
            }

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
