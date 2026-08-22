/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgRequestPartyMemberStats : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgRequestPartyMemberStats() : CmsgRequestPartyMemberStats(0)
        {
        }

        CmsgRequestPartyMemberStats(uint64_t guid) :
            ManagedPacket(CMSG_REQUEST_PARTY_MEMBER_STATS, 0),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.readSkip<uint8_t>();

                guid[7] = packet.readBit();
                guid[4] = packet.readBit();
                guid[0] = packet.readBit();
                guid[1] = packet.readBit();
                guid[3] = packet.readBit();
                guid[6] = packet.readBit();
                guid[2] = packet.readBit();
                guid[5] = packet.readBit();

                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[7]);
                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid;
                guid.init(unpacked_guid);
                return true;
            }

            return false;
        }
    };
}
