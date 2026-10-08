/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSetCurrency : public ManagedPacket
    {
    public:
        uint32_t currencyId = 0;
        int32_t quantity = 0;
        uint32_t weeklyQuantity = 0;
        uint32_t trackedQuantity = 0;
        uint32_t maxQuantity = 0;
        uint32_t flags = 0;
        bool suppressChatLog = false;

        SmsgSetCurrency() : ManagedPacket(SMSG_SET_CURRENCY, 32) { }

        SmsgSetCurrency(uint32_t id, int32_t currencyQuantity, uint32_t currencyWeeklyQuantity,
            uint32_t currencyTrackedQuantity, uint32_t currencyMaxQuantity, uint32_t currencyFlags, bool suppress) :
            ManagedPacket(SMSG_SET_CURRENCY, 32), currencyId(id), quantity(currencyQuantity),
            weeklyQuantity(currencyWeeklyQuantity), trackedQuantity(currencyTrackedQuantity), maxQuantity(currencyMaxQuantity),
            flags(currencyFlags), suppressChatLog(suppress) { }

    protected:
        size_t expectedSize() const override { return 32; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            // FOREVER-VERIFIED STRUCTURE: modern SetCurrency base fields and optional-value bits.
            packet << int32_t(currencyId);
            packet << int32_t(quantity);
            packet << uint32_t(flags);
            packet << uint32_t(0); // UiEventToast count; not used by AscEmu yet.

            packet.writeBit(weeklyQuantity != 0);
            packet.writeBit(trackedQuantity != 0);
            packet.writeBit(maxQuantity != 0);
            packet.writeBit(false); // TotalEarned - not tracked by AscEmu yet.
            packet.writeBit(suppressChatLog);
            packet.writeBit(false); // QuantityChange - UNKNOWN/unused.
            packet.writeBit(false); // QuantityGainSource - UNKNOWN/unused.
            packet.writeBit(false); // QuantityLostSource - UNKNOWN/unused.
            packet.writeBit(false); // FirstCraftOperationID - UNKNOWN/unused.
            packet.writeBit(false); // NextRechargeTime - UNKNOWN/unused.
            packet.writeBit(false); // RechargeCycleStartTime - UNKNOWN/unused.
            packet.writeBit(false); // OverflownCurrencyID - UNKNOWN/unused.
            packet.flushBits();

            if (weeklyQuantity != 0)
                packet << int32_t(weeklyQuantity);
            if (trackedQuantity != 0)
                packet << int32_t(trackedQuantity);
            if (maxQuantity != 0)
                packet << int32_t(maxQuantity);

            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }
    };
}
