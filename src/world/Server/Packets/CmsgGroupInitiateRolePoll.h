/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgGroupInitiateRolePoll : public ManagedPacket
    {
    public:
        uint8_t partyIndex = 0;

        CmsgGroupInitiateRolePoll() : ManagedPacket(CMSG_GROUP_INITIATE_ROLE_POLL, 1)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet >> partyIndex;

            return true;
        }
    };
}
