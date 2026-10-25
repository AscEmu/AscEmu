/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgTaxinodeStatusQuery : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgTaxinodeStatusQuery() : CmsgTaxinodeStatusQuery(0)
        {
        }

        CmsgTaxinodeStatusQuery(uint64_t guid) :
            ManagedPacket(CMSG_TAXINODE_STATUS_QUERY, 8),
            guid(guid)
        {
        }

        bool deserialise(WorldPacket& packet) override
        {
            if (packet.remaining() < expectedSize())
                return false;

            return internalDeserialise(packet);
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
                return m_minimum_size;
            else if (m_protocol.isMop())
                return 1; // packed guid: mask byte + only the non-zero guid bytes (1..9 bytes)
            return 0;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // bit and byte order verified against a captured 5.4.8 packet (creature guid 0xF1300099AC000830)
                guid[0] = packet.readBit();
                guid[1] = packet.readBit();
                guid[6] = packet.readBit();
                guid[4] = packet.readBit();
                guid[3] = packet.readBit();
                guid[2] = packet.readBit();
                guid[5] = packet.readBit();
                guid[7] = packet.readBit();

                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[3]);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                guid.init(unpackedGuid);

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // 6.x and later: the 128 bit guid of the flight master
                WoWGuid128 unit;
                packet >> unit;
                guid.init(WoWGuid::fromGuid128(unit));

                return true;
            }

            return false;
        }
    };
}
