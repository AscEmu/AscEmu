/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgPetitionShowSignatures : public ManagedPacket
    {
    public:
        WoWGuid itemGuid;

        CmsgPetitionShowSignatures() : CmsgPetitionShowSignatures(0)
        {
        }

        CmsgPetitionShowSignatures(uint64_t itemGuid) :
            ManagedPacket(CMSG_PETITION_SHOW_SIGNATURES, 8),
            itemGuid(itemGuid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                itemGuid.init(unpackedGuid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                itemGuid[3] = packet.readBit();
                itemGuid[7] = packet.readBit();
                itemGuid[2] = packet.readBit();
                itemGuid[4] = packet.readBit();
                itemGuid[5] = packet.readBit();
                itemGuid[6] = packet.readBit();
                itemGuid[0] = packet.readBit();
                itemGuid[1] = packet.readBit();

                packet.readByteSeq(itemGuid[2]);
                packet.readByteSeq(itemGuid[4]);
                packet.readByteSeq(itemGuid[5]);
                packet.readByteSeq(itemGuid[7]);
                packet.readByteSeq(itemGuid[1]);
                packet.readByteSeq(itemGuid[0]);
                packet.readByteSeq(itemGuid[3]);
                packet.readByteSeq(itemGuid[6]);
                return true;
            }

            return false;
        }
    };
}
