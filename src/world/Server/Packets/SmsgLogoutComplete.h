/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgLogoutComplete : public ManagedPacket
    {
    public:

        SmsgLogoutComplete() :
            ManagedPacket(SMSG_LOGOUT_COMPLETE, 0)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            // 11.x: the optional game mode switch
            if (m_protocol.isTheWarWithin())
            {
                packet.writeBit(false);
                packet.flushBits();
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
