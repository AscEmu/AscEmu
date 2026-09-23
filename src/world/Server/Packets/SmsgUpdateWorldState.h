/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgUpdateWorldState : public ManagedPacket
    {
    public:
        uint32_t worldState1;
        uint32_t value1;
        uint32_t worldState2;
        uint32_t value2;

        SmsgUpdateWorldState() : SmsgUpdateWorldState(0, 0, 0, 0)
        {
        }

        SmsgUpdateWorldState(uint32_t worldState1, uint32_t value1, uint32_t worldState2 = 0, uint32_t value2 = 0) :
            ManagedPacket(SMSG_UPDATE_WORLD_STATE, 8),
            worldState1(worldState1),
            value1(value1),
            worldState2(worldState2),
            value2(value2)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion == WoW::Expansion::_Classic)
            {
                // Arena states (0xC77 / 0xF3D) do not exist in Classic and must not be sent
                if (worldState1 == 0xC77)
                    return false;

                // Classic SMSG_UPDATE_WORLD_STATE only supports exactly 8 bytes (1 state per packet)
                packet << worldState1 << value1;
                return true;
            }

            if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                packet.writeBit(0);
            }

            packet << worldState1 << value1;

            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                if (worldState2 != 0)
                    packet << worldState2 << value2;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
