/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgBuyBankSlotResult : public ManagedPacket
    {
    public:
        uint32_t error;

        SmsgBuyBankSlotResult() : SmsgBuyBankSlotResult(0)
        {
        }

        SmsgBuyBankSlotResult(uint32_t error) :
            ManagedPacket(SMSG_BUY_BANK_SLOT_RESULT, 4),
            error(error)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // the 6.x and later clients take the bought slot from the player values
                return false;
            }

            packet << error;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
