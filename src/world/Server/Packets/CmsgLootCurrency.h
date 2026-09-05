/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // Payload is the currency slot within the loot session's Currencies list, not the item slot
    // space. No Mop opcode value exists yet, so no Mop branch.
    class CmsgLootCurrency : public ManagedPacket
    {
    public:
        uint8_t slot = 0;

        CmsgLootCurrency() : CmsgLootCurrency(0)
        {
        }

        explicit CmsgLootCurrency(uint8_t currencySlot) :
            ManagedPacket(CMSG_LOOT_CURRENCY, 1),
            slot(currencySlot)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet >> slot;
                return true;
            }

            return false;
        }
    };
}
