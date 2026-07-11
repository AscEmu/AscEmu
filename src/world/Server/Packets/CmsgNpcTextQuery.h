/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgNpcTextQuery : public ManagedPacket
    {
    public:
        uint32_t text_id;
        WoWGuid guid;

        CmsgNpcTextQuery() : CmsgNpcTextQuery(0, 0)
        {
        }

        CmsgNpcTextQuery(uint32_t text_id, uint64_t guid) :
            ManagedPacket(CMSG_NPC_TEXT_QUERY, 0),
            text_id(text_id),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t rawGuid;
                packet >> text_id >> rawGuid;
                guid.init(rawGuid);
            }
            else // Mop
            {
                packet >> text_id;
                guid[4] = packet.readBit();
                guid[5] = packet.readBit();
                guid[1] = packet.readBit();
                guid[7] = packet.readBit();
                guid[0] = packet.readBit();
                guid[2] = packet.readBit();
                guid[6] = packet.readBit();
                guid[3] = packet.readBit();

                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[6]);
            }
            return true;
        }
    };
}
