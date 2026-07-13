/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgDestroyObject : public ManagedPacket
    {
    public:

        uint64_t guid;

        SmsgDestroyObject() : SmsgDestroyObject(0)
        {
        }

        SmsgDestroyObject(uint64_t guid) : 
        ManagedPacket(SMSG_DESTROY_OBJECT, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << guid;

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                    packet << uint8_t(0);
            }
            else // Mop
            {
                WoWGuid oGuid = guid;

                packet.writeBit(oGuid[3]);
                packet.writeBit(oGuid[2]);
                packet.writeBit(oGuid[4]);
                packet.writeBit(oGuid[1]);

                packet << uint8_t(0);

                packet.writeBit(oGuid[7]);
                packet.writeBit(oGuid[0]);
                packet.writeBit(oGuid[6]);
                packet.writeBit(oGuid[5]);

                packet.flushBits();

                packet.writeByteSeq(oGuid[0]);
                packet.writeByteSeq(oGuid[4]);
                packet.writeByteSeq(oGuid[7]);
                packet.writeByteSeq(oGuid[2]);
                packet.writeByteSeq(oGuid[6]);
                packet.writeByteSeq(oGuid[3]);
                packet.writeByteSeq(oGuid[1]);
                packet.writeByteSeq(oGuid[5]);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
