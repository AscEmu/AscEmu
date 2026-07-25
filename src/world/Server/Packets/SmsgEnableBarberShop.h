/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgEnableBarberShop : public ManagedPacket
    {
    public:

        SmsgEnableBarberShop() :
            ManagedPacket(SMSG_ENABLE_BARBER_SHOP, 0)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
