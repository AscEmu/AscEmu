/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "ForeverSpellPacketUtils.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgCastFailed : public ManagedPacket
    {
    public:
        uint8_t multiCast;
        uint32_t spellId;
        uint8_t errorMsg;

        uint32_t extra1;
        uint32_t extra2;
        WoWGuid castId = WoWGuid::createModernEmpty();
        uint32_t spellXSpellVisualId = 0;
        uint32_t scriptVisualId = 0;
        WoWGuid failedBy = WoWGuid::createModernEmpty();
        uint16_t mapId = 0;

        SmsgCastFailed() : SmsgCastFailed(0, 0, 0, 0, 0)
        {
        }

        SmsgCastFailed(uint8_t multiCast, uint32_t spellId, uint8_t errorMsg, uint32_t extra1, uint32_t extra2) :
            ManagedPacket(SMSG_CAST_FAILED, 0),
            multiCast(multiCast),
            spellId(spellId),
            errorMsg(errorMsg),
            extra1(extra1),
            extra2(extra2)
        {
        }

    protected:
        size_t expectedSize() const override { return 1 + 4 + 1 + 4 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                ForeverSpellPacket::writePackedGuid(packet, castId);
                packet << static_cast<int32_t>(spellId);
                packet << static_cast<int32_t>(spellXSpellVisualId);
                packet << static_cast<int32_t>(scriptVisualId);
                packet << static_cast<int32_t>(errorMsg);
                packet << static_cast<int32_t>(extra1);
                packet << static_cast<int32_t>(extra2);
                ForeverSpellPacket::writePackedGuid(packet, ForeverSpellPacket::toModernGuid(failedBy, m_protocol.realmId, mapId));
                return true;
            }

            if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                packet << spellId << errorMsg << multiCast;
                packet.writeBit(1);
                packet.writeBit(1);
                packet.flushBits();

                packet.writeBits(0, extra2 ? 2 : 1);
                if (extra1 || extra2)
                    packet << extra1;

                if (extra2)
                    packet << extra2;
            }
            else // < Mop
            {
                packet << multiCast << spellId << errorMsg;

                if (extra1 || extra2)
                    packet << extra1;

                if (extra2)
                    packet << extra2;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
