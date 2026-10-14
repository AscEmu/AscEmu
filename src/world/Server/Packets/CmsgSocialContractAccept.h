/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgSocialContractAccept : public ManagedPacket
    {
    public:
        CmsgSocialContractAccept() : ManagedPacket(CMSG_SOCIAL_CONTRACT_ACCEPT, 0) {}

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            return m_protocol.isForever() && packet.remaining() == 0;
        }
    };
}
