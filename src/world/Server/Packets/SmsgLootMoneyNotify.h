/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgLootMoneyNotify : public ManagedPacket
    {
    public:
        uint32_t money;
        uint8_t playersNear; // 0 = "Your share of the loot is...", 1 = "You loot..."

        SmsgLootMoneyNotify() : SmsgLootMoneyNotify(0, 1)
        {
        }

        SmsgLootMoneyNotify(uint32_t money, uint8_t playersNear) :
            ManagedPacket(SMSG_LOOT_MONEY_NOTIFY, 0),
            money(money), playersNear(playersNear)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << money << playersNear;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
