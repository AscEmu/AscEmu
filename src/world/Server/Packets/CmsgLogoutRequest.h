/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgLogoutRequest : public ManagedPacket
    {
    public:
        bool idleLogout{false};

        CmsgLogoutRequest() : ManagedPacket(CMSG_LOGOUT_REQUEST, 0)
        {
        }

    protected:
        size_t expectedSize() const override { return m_protocol.isForever() ? size_t(1) : size_t(0); }

        bool internalSerialise(WorldPacket& /*packet*/) override { return false; }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return true;

            if (packet.remaining() != 1)
                return false;

            idleLogout = packet.readBit();
            return true;
        }
    };
}
