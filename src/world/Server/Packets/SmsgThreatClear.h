/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgThreatClear : public ManagedPacket
    {
    public:
        WoWGuid guid;

        SmsgThreatClear(WoWGuid guid) :
            ManagedPacket(SMSG_THREAT_CLEAR, 8),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet.appendPackGuid(guid.getRawGuid());
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(guid[6]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[3]);

                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[5]);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
