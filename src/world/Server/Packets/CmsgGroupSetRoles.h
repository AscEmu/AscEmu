/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGroupSetRoles : public ManagedPacket
    {
    public:
        WoWGuid targetGuid;
        uint32_t newRole;

        CmsgGroupSetRoles() : CmsgGroupSetRoles(0, 0)
        {
        }

        CmsgGroupSetRoles(WoWGuid targetGuid, uint32_t newRole) :
            ManagedPacket(CMSG_GROUP_SET_ROLES, 0),
            targetGuid(targetGuid),
            newRole(newRole)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet >> newRole;

                targetGuid[2] = packet.readBit();
                targetGuid[6] = packet.readBit();
                targetGuid[3] = packet.readBit();
                targetGuid[7] = packet.readBit();
                targetGuid[5] = packet.readBit();
                targetGuid[1] = packet.readBit();
                targetGuid[0] = packet.readBit();
                targetGuid[4] = packet.readBit();

                packet.readByteSeq(targetGuid[6]);
                packet.readByteSeq(targetGuid[4]);
                packet.readByteSeq(targetGuid[1]);
                packet.readByteSeq(targetGuid[3]);
                packet.readByteSeq(targetGuid[0]);
                packet.readByteSeq(targetGuid[5]);
                packet.readByteSeq(targetGuid[2]);
                packet.readByteSeq(targetGuid[7]);

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.readSkip<uint8_t>();

                packet >> newRole;

                targetGuid[2] = packet.readBit();
                targetGuid[0] = packet.readBit();
                targetGuid[7] = packet.readBit();
                targetGuid[4] = packet.readBit();
                targetGuid[1] = packet.readBit();
                targetGuid[3] = packet.readBit();
                targetGuid[6] = packet.readBit();
                targetGuid[5] = packet.readBit();

                packet.readByteSeq(targetGuid[1]);
                packet.readByteSeq(targetGuid[5]);
                packet.readByteSeq(targetGuid[2]);
                packet.readByteSeq(targetGuid[6]);
                packet.readByteSeq(targetGuid[7]);
                packet.readByteSeq(targetGuid[0]);
                packet.readByteSeq(targetGuid[4]);
                packet.readByteSeq(targetGuid[3]);

                return true;
            }

            return false;
        }
    };
}
