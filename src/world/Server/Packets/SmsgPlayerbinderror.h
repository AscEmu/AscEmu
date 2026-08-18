/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPlayerbinderror : public ManagedPacket
    {
    public:
        uint32_t error;

        SmsgPlayerbinderror() : SmsgPlayerbinderror(0)
        {
        }

        SmsgPlayerbinderror(uint32_t error) :
            ManagedPacket(SMSG_PLAYERBINDERROR, sizeof(error)),
            error(error)
        {
        }

    protected:
        size_t expectedSize() const override { return sizeof(error); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
            {
                packet << error;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
