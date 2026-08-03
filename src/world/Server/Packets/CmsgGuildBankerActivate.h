/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildBankerActivate : public ManagedPacket
    {
    public:
        WoWGuid guid;
        bool full = false;  // since Cata

        CmsgGuildBankerActivate() : CmsgGuildBankerActivate(0)
        {
        }

        CmsgGuildBankerActivate(uint64_t guid) :
            ManagedPacket(CMSG_GUILD_BANKER_ACTIVATE, 0),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                guid.init(unpackedGuid);

                if (m_protocol.expansion >= WoW::Expansion::_Cata)
                    packet >> full;
            }
            else // Mop
            {
                guid[3] = packet.readBit();

                full = packet.readBit();

                guid[0] = packet.readBit();
                guid[7] = packet.readBit();
                guid[1] = packet.readBit();
                guid[5] = packet.readBit();
                guid[2] = packet.readBit();
                guid[6] = packet.readBit();
                guid[4] = packet.readBit();

                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[3]);
            }

            return true;
        }
    };
}
