/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "BattlePetPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // a pet could not be learned, the creature names it in the message
    class SmsgBattlePetError : public ManagedPacket
    {
    public:
        uint8_t result = 0;
        uint32_t creatureId = 0;

        SmsgBattlePetError(uint8_t result, uint32_t creatureId) : ManagedPacket(SMSG_BATTLE_PET_ERROR, 5), result(result), creatureId(creatureId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_WoD)
                return false;

            // 8.x sends the result in three bits
            packet.writeBits(result, m_protocol.isBfA() ? 3 : 4);
            packet.flushBits();
            packet << int32_t(creatureId);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
