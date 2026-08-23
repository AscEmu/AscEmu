/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgPetNameQuery : public ManagedPacket
    {
    public:
        uint32_t petNumber;
        WoWGuid guid;

        CmsgPetNameQuery() : CmsgPetNameQuery(0, 0)
        {
        }

        CmsgPetNameQuery(uint64_t guid, uint32_t petNumber) :
            ManagedPacket(CMSG_PET_NAME_QUERY, 8),
            petNumber(petNumber),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpacked_guid;
                packet >> petNumber >> unpacked_guid;
                guid.init(unpacked_guid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                WoWGuid petNumberGuid;

                petNumberGuid[0] = packet.readBit();
                petNumberGuid[5] = packet.readBit();
                guid[1] = packet.readBit();
                guid[7] = packet.readBit();
                petNumberGuid[7] = packet.readBit();
                guid[6] = packet.readBit();
                guid[4] = packet.readBit();
                guid[5] = packet.readBit();
                guid[0] = packet.readBit();
                petNumberGuid[3] = packet.readBit();
                petNumberGuid[6] = packet.readBit();
                petNumberGuid[2] = packet.readBit();
                guid[3] = packet.readBit();
                guid[2] = packet.readBit();
                petNumberGuid[1] = packet.readBit();
                petNumberGuid[4] = packet.readBit();

                packet.readByteSeq(petNumberGuid[2]);
                packet.readByteSeq(petNumberGuid[1]);
                packet.readByteSeq(petNumberGuid[0]);
                packet.readByteSeq(petNumberGuid[7]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(petNumberGuid[6]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(petNumberGuid[5]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(petNumberGuid[3]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(petNumberGuid[4]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[7]);

                petNumber = petNumberGuid.getGuidLowPart();
                return true;
            }

            return false;
        }
    };
}
