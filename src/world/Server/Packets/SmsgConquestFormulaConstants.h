/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgConquestFormulaConstants : public ManagedPacket
    {
    public:
        SmsgConquestFormulaConstants() : ManagedPacket(SMSG_CONQUEST_FORMULA_CONSTANTS, 4 + 4 + 4 + 4 + 4)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 4 + 4 + 4 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet << uint32_t(2000);      // PvpMinCPPerWeek
            packet << float(1639.28f);     // PvpCPExpCoefficient
            packet << float(0.00412f);     // PvpCPNumerator
            packet << uint32_t(3500);      // PvpMaxCPPerWeek
            packet << float(1511.26f);     // PvpCPBaseCoefficient

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
