/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGameobjectQuery : public ManagedPacket
    {
    public:
        uint32_t entry;
        WoWGuid guid;

        CmsgGameobjectQuery() : CmsgGameobjectQuery(0, 0)
        {
        }

        CmsgGameobjectQuery(uint32_t entry, uint64_t guid) :
            ManagedPacket(CMSG_GAMEOBJECT_QUERY, 0),
            entry(entry),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t unpacked_guid;
                packet >> entry >> unpacked_guid;
                guid.init(unpacked_guid);
                return true;
            }
            else // Mop
            {
                packet >> entry;

                guid[5] = packet.readBit();
                guid[3] = packet.readBit();
                guid[6] = packet.readBit();
                guid[2] = packet.readBit();
                guid[7] = packet.readBit();
                guid[1] = packet.readBit();
                guid[0] = packet.readBit();
                guid[4] = packet.readBit();

                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[0]);

                return true;
            }
        }
    };
}
