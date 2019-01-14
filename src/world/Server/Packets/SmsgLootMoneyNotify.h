/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgLootMoneyNotify : public ManagedPacket
    {
    public:
        uint32_t money;

        SmsgLootMoneyNotify() : SmsgLootMoneyNotify(0)
        {
        }

        SmsgLootMoneyNotify(uint32_t money) :
            ManagedPacket(SMSG_LOOT_MONEY_NOTIFY, 0),
            money(money)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << money;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
