/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    struct SmsgSpellHistoryEntry
    {
        uint32_t spellId = 0;
        uint32_t itemId = 0;
        uint32_t categoryId = 0;
        int32_t recoveryTime = 0;
        int32_t categoryRecoveryTime = 0;
        float modRate = 1.0f;
        bool onHold = false;
    };

    class SmsgSendSpellHistory : public ManagedPacket
    {
    public:
        std::vector<SmsgSpellHistoryEntry> entries;

        SmsgSendSpellHistory() : SmsgSendSpellHistory(std::vector<SmsgSpellHistoryEntry>{})
        {
        }

        explicit SmsgSendSpellHistory(std::vector<SmsgSpellHistoryEntry> entries) :
            ManagedPacket(SMSG_SEND_SPELL_HISTORY, 4 + entries.size() * 25),
            entries(std::move(entries))
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet << uint32_t(entries.size());

            for (auto const& entry : entries)
            {
                packet << uint32_t(entry.spellId);
                packet << uint32_t(entry.itemId);
                packet << uint32_t(entry.categoryId);
                packet << int32_t(entry.recoveryTime);
                packet << int32_t(entry.categoryRecoveryTime);
                packet << float(entry.modRate);

                packet.writeBit(false); // RecoveryTimeStartOffset
                packet.writeBit(false); // CategoryRecoveryTimeStartOffset
                packet.writeBit(entry.onHold);
                packet.flushBits();
            }

            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }
    };
}
