/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class SmsgGroupSetRoles : public ManagedPacket
    {
    public:
        WoWGuid targetGuid;
        WoWGuid guid;
        uint32_t newRole;

        SmsgGroupSetRoles(WoWGuid targetGuid, WoWGuid guid, uint32_t newRole) :
            ManagedPacket(SMSG_GROUP_SET_ROLE, 8 + 8 + 4),
            targetGuid(targetGuid),
            guid(guid),
            newRole(newRole)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet.writeBit(guid[1]);

                packet.writeBit(targetGuid[0]);
                packet.writeBit(targetGuid[2]);
                packet.writeBit(targetGuid[4]);
                packet.writeBit(targetGuid[7]);
                packet.writeBit(targetGuid[3]);

                packet.writeBit(guid[7]);

                packet.writeBit(targetGuid[5]);

                packet.writeBit(guid[5]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[3]);

                packet.writeBit(targetGuid[6]);

                packet.writeBit(guid[2]);
                packet.writeBit(guid[6]);

                packet.writeBit(targetGuid[1]);

                packet.writeBit(guid[0]);

                packet.writeByteSeq(guid[7]);

                packet.writeByteSeq(targetGuid[3]);

                packet.writeByteSeq(guid[6]);

                packet.writeByteSeq(targetGuid[4]);
                packet.writeByteSeq(targetGuid[0]);

                packet << uint32_t(newRole);

                packet.writeByteSeq(targetGuid[5]);
                packet.writeByteSeq(targetGuid[2]);

                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[4]);

                packet.writeByteSeq(targetGuid[1]);

                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[2]);

                packet.writeByteSeq(targetGuid[6]);
                packet.writeByteSeq(targetGuid[7]);

                packet.writeByteSeq(guid[1]);

                packet << uint32_t(0);              // OldRole (not tracked per-member in AscEmu's Group)

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(guid[1]);

                packet.writeBit(targetGuid[7]);
                packet.writeBit(targetGuid[6]);
                packet.writeBit(targetGuid[4]);
                packet.writeBit(targetGuid[1]);
                packet.writeBit(targetGuid[0]);

                packet.writeBit(guid[0]);
                packet.writeBit(guid[7]);

                packet.writeBit(targetGuid[3]);

                packet.writeBit(guid[6]);

                packet.writeBit(targetGuid[2]);

                packet.writeBit(guid[4]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[2]);

                packet.writeBit(targetGuid[5]);

                packet.writeBit(guid[3]);

                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[2]);

                packet.writeByteSeq(targetGuid[3]);

                packet << uint32_t(0);              // old role?

                packet.writeByteSeq(guid[7]);

                packet.writeByteSeq(targetGuid[5]);

                packet.writeByteSeq(guid[3]);

                packet.writeByteSeq(targetGuid[4]);
                packet.writeByteSeq(targetGuid[7]);

                packet.writeByteSeq(guid[5]);

                packet.writeByteSeq(targetGuid[6]);
                packet.writeByteSeq(targetGuid[2]);
                packet.writeByteSeq(targetGuid[1]);
                packet.writeByteSeq(targetGuid[0]);

                packet.writeByteSeq(guid[4]);

                packet << uint8_t(0);               // unk

                packet.writeByteSeq(guid[0]);

                packet << uint32_t(newRole);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
