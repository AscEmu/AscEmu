/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class MsgTabardvendorActivate : public ManagedPacket
    {
    public:
        WoWGuid guid;

        MsgTabardvendorActivate() : MsgTabardvendorActivate(0)
        {
        }

        MsgTabardvendorActivate(uint64_t guid) :
            ManagedPacket(MSG_TABARDVENDOR_ACTIVATE, 0),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << guid.getRawGuid();
            }
            else // Mop
            {
                packet.writeBit(guid[1]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[2]);

                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[7]);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                guid.init(unpackedGuid);
                return true;
            }

            return false;
        }
    };
}
