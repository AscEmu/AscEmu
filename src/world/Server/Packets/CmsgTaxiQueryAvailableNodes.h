/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgTaxiQueryAvailableNodes : public ManagedPacket
    {
    public:
        WoWGuid creatureGuid;

        CmsgTaxiQueryAvailableNodes() : CmsgTaxiQueryAvailableNodes(0)
        {
        }

        CmsgTaxiQueryAvailableNodes(uint64_t creatureGuid) :
            ManagedPacket(CMSG_TAXIQUERYAVAILABLENODES, 8),
            creatureGuid(creatureGuid)
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
                creatureGuid[7] = packet.readBit();
                creatureGuid[1] = packet.readBit();
                creatureGuid[0] = packet.readBit();
                creatureGuid[4] = packet.readBit();
                creatureGuid[2] = packet.readBit();
                creatureGuid[5] = packet.readBit();
                creatureGuid[6] = packet.readBit();
                creatureGuid[3] = packet.readBit();

                packet.readByteSeq(creatureGuid[0]);
                packet.readByteSeq(creatureGuid[3]);
                packet.readByteSeq(creatureGuid[7]);
                packet.readByteSeq(creatureGuid[5]);
                packet.readByteSeq(creatureGuid[2]);
                packet.readByteSeq(creatureGuid[6]);
                packet.readByteSeq(creatureGuid[4]);
                packet.readByteSeq(creatureGuid[1]);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                creatureGuid.init(unpackedGuid);

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // 6.x and later: the 128 bit guid of the flight master
                WoWGuid128 unit;
                packet >> unit;
                creatureGuid.init(WoWGuid::fromGuid128(unit));

                return true;
            }

            return false;
        }
    };
}
