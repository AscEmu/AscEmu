/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgBarberShopResult : public ManagedPacket
    {
#if VERSION_STRING > TBC
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
            packet << result;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}}
