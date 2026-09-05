/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    // Values are already display-precision-adjusted by the caller.
    struct CurrencyRecord
    {
        uint32_t id = 0;
        uint32_t quantity = 0;
        uint32_t weeklyQuantity = 0;
        uint32_t weekCap = 0;
        uint32_t trackedQuantity = 0;
        uint8_t flags = 0;
    };

    class SmsgSetupCurrency : public ManagedPacket
    {
    public:
        std::vector<CurrencyRecord> records;

        SmsgSetupCurrency() : SmsgSetupCurrency(std::vector<CurrencyRecord>{})
        {
        }

        explicit SmsgSetupCurrency(std::vector<CurrencyRecord> currencyRecords) :
            ManagedPacket(SMSG_SETUP_CURRENCY, 4 + static_cast<size_t>(currencyRecords.size()) * 20),
            records(std::move(currencyRecords))
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + records.size() * 20; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet.writeBits(static_cast<uint32_t>(records.size()), 23);

                for (auto const& record : records)
                {
                    packet.writeBit(record.weeklyQuantity != 0);
                    packet.writeBits(record.flags, 4);
                    packet.writeBit(record.weekCap != 0);
                    packet.writeBit(record.trackedQuantity != 0);
                }

                packet.flushBits();

                for (auto const& record : records)
                {
                    packet << uint32_t(record.quantity);
                    if (record.weekCap != 0)
                        packet << uint32_t(record.weekCap);
                    if (record.trackedQuantity != 0)
                        packet << uint32_t(record.trackedQuantity);
                    packet << uint32_t(record.id);
                    if (record.weeklyQuantity != 0)
                        packet << uint32_t(record.weeklyQuantity);
                }

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBits(static_cast<uint32_t>(records.size()), 21);

                for (auto const& record : records)
                {
                    packet.writeBit(record.trackedQuantity != 0);
                    packet.writeBits(record.flags, 5);
                    packet.writeBit(record.weekCap != 0);
                    packet.writeBit(record.weeklyQuantity != 0);
                }

                packet.flushBits();

                for (auto const& record : records)
                {
                    if (record.weeklyQuantity != 0)
                        packet << uint32_t(record.weeklyQuantity);
                    packet << uint32_t(record.id);
                    if (record.trackedQuantity != 0)
                        packet << uint32_t(record.trackedQuantity);
                    packet << uint32_t(record.quantity);
                    if (record.weekCap != 0)
                        packet << uint32_t(record.weekCap);
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
