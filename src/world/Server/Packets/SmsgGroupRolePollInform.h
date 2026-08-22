/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgGroupRolePollInform : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t partyIndex;

        SmsgGroupRolePollInform() : SmsgGroupRolePollInform(0, 0)
        {
        }

        SmsgGroupRolePollInform(uint64_t guid, uint8_t partyIndex) :
            ManagedPacket(SMSG_GROUP_ROLE_POLL_INFORM, 9),
            guid(guid),
            partyIndex(partyIndex)
        {
        }

    protected:
        size_t expectedSize() const override { return 9; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet.writeBit(guid[5]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[6]);
            packet.flushBits();

            packet.writeByteSeq(guid[7]);
            packet << partyIndex;
            packet.writeByteSeq(guid[6]);
            packet.writeByteSeq(guid[5]);
            packet.writeByteSeq(guid[0]);
            packet.writeByteSeq(guid[1]);
            packet.writeByteSeq(guid[4]);
            packet.writeByteSeq(guid[2]);
            packet.writeByteSeq(guid[3]);

            return true;
        }
    };
}
