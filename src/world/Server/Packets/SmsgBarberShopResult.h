/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgBarberShopResult : public ManagedPacket
    {
    public:
        uint32_t result;

        SmsgBarberShopResult() : SmsgBarberShopResult(0)
        {
        }

        SmsgBarberShopResult(uint32_t result) :
            ManagedPacket(SMSG_BARBER_SHOP_RESULT, 0),
            result(result)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            packet << result;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
