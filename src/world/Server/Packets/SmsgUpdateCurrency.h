/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgUpdateCurrency : public ManagedPacket
    {
    public:
        uint32_t currencyId = 0;
        int32_t quantity = 0;
        uint32_t weeklyQuantity = 0;
        uint32_t trackedQuantity = 0;
        uint8_t flags = 0;
        bool suppressChatLog = false;

        SmsgUpdateCurrency() : ManagedPacket(SMSG_UPDATE_CURRENCY, 24)
        {
        }

        SmsgUpdateCurrency(uint32_t id, int32_t currencyQuantity, uint32_t currencyWeeklyQuantity, uint32_t currencyTrackedQuantity, uint8_t currencyFlags, bool suppress) :
            ManagedPacket(SMSG_UPDATE_CURRENCY, 24),
            currencyId(id), quantity(currencyQuantity), weeklyQuantity(currencyWeeklyQuantity),
            trackedQuantity(currencyTrackedQuantity), flags(currencyFlags), suppressChatLog(suppress)
        {
        }

    protected:
        size_t expectedSize() const override { return 24; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet.writeBit(weeklyQuantity != 0);
                packet.writeBit(trackedQuantity != 0);
                packet.writeBit(suppressChatLog);
                packet.flushBits();

                if (trackedQuantity != 0)
                    packet << uint32_t(trackedQuantity);

                packet << uint32_t(quantity);
                packet << uint32_t(currencyId);

                if (weeklyQuantity != 0)
                    packet << uint32_t(weeklyQuantity);

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet << uint32_t(currencyId);
                packet << int32_t(quantity);
                packet << uint32_t(flags);

                packet.writeBit(trackedQuantity != 0);
                packet.writeBit(suppressChatLog);
                packet.writeBit(weeklyQuantity != 0);
                packet.flushBits();

                if (trackedQuantity != 0)
                    packet << uint32_t(trackedQuantity);
                if (weeklyQuantity != 0)
                    packet << uint32_t(weeklyQuantity);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
