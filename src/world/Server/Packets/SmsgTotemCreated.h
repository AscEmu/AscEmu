/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgTotemCreated : public ManagedPacket
    {
    public:
        uint8_t slot;
        uint64_t guid;
        uint32_t duration;
        uint32_t spellId;

        SmsgTotemCreated() : SmsgTotemCreated(0, 0, 0, 0)
        {
        }

        SmsgTotemCreated(uint8_t slot, uint64_t guid, uint32_t duration, uint32_t spellId) :
            ManagedPacket(SMSG_TOTEM_CREATED, 0),
            slot(slot),
            guid(guid),
            duration(duration),
            spellId(spellId)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 1 + 8 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion())
            {
                // slot, totem, duration, spell, 7.x: time modifier, cannot dismiss
                packet << int8_t(slot);
                packet << WoWGuid(guid).toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << int32_t(duration);
                packet << int32_t(spellId);
                if (m_protocol.isLegion())
                {
                    packet << float(1.0f);
                    packet.writeBit(false);
                    packet.flushBits();
                }

                return true;
            }

            packet << slot << guid << duration << spellId;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
