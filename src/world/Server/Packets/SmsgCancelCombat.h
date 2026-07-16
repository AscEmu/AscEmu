/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgCancelCombat : public ManagedPacket
    {
    public:

        SmsgCancelCombat() :
            ManagedPacket(SMSG_CANCEL_COMBAT, 0)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion == WoW::Expansion::_Mop)
                packet << uint64_t(0);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
