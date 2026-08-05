/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgGroupInviteResponse : public ManagedPacket
    {
    public:
        bool isAccepted;

        CmsgGroupInviteResponse() : CmsgGroupInviteResponse(false)
        {
        }

        CmsgGroupInviteResponse(bool isAccepted) :
            ManagedPacket(CMSG_GROUP_INVITE_RESPONSE, 1),
            isAccepted(isAccepted)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet.readBit();   // unk
                isAccepted = packet.readBit();

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.readSkip<uint8_t>();
                packet.readBit();   // unk
                isAccepted = packet.readBit();

                return true;
            }

            return false;
        }
    };
}
