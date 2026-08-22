/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgSetPartyAssignment : public ManagedPacket
    {
    public:
        uint8_t assignment = 0;
        uint8_t partyIndex = 0;
        bool apply = false;
        WoWGuid guid;

        CmsgSetPartyAssignment() : ManagedPacket(CMSG_SET_PARTY_ASSIGNMENT, 2)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet >> assignment;
            packet >> partyIndex;

            guid[5] = packet.readBit();
            guid[6] = packet.readBit();
            guid[2] = packet.readBit();
            guid[3] = packet.readBit();
            guid[1] = packet.readBit();
            guid[0] = packet.readBit();
            guid[4] = packet.readBit();
            guid[7] = packet.readBit();

            apply = packet.readBit();

            packet.readByteSeq(guid[2]);
            packet.readByteSeq(guid[5]);
            packet.readByteSeq(guid[1]);
            packet.readByteSeq(guid[0]);
            packet.readByteSeq(guid[6]);
            packet.readByteSeq(guid[3]);
            packet.readByteSeq(guid[4]);
            packet.readByteSeq(guid[7]);

            return true;
        }
    };
}
