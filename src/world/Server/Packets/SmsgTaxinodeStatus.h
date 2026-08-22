/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgTaxinodeStatus : public ManagedPacket
    {
    public:
        WoWGuid guid;
        bool isNodeKnown;

        SmsgTaxinodeStatus() : SmsgTaxinodeStatus(0, 0)
        {
        }

        SmsgTaxinodeStatus(uint64_t guid, bool isNodeKnown) :
            ManagedPacket(SMSG_TAXINODE_STATUS, 0),
            guid(guid),
            isNodeKnown(isNodeKnown)
        {
        }

    protected:

        size_t expectedSize() const override { return 9; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBit(guid[6]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[1]);
                packet.writeBits(isNodeKnown ? 1 : 3, 2);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[0]);
                packet.flushBits();

                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[3]);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << guid.getRawGuid();
                if (m_protocol.expansion < WoW::Expansion::_Cata)
                {
                    packet << static_cast<uint8_t>(isNodeKnown ? 1 : 0);
                }
                else
                {
                    packet << static_cast<uint8_t>(isNodeKnown ? 1 : 2);
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
