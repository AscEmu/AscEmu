/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgGuildBankMoneyWithdrawn : public ManagedPacket
    {
    public:
        int64_t amount;

        SmsgGuildBankMoneyWithdrawn() : SmsgGuildBankMoneyWithdrawn(0)
        {
        }

        SmsgGuildBankMoneyWithdrawn(int64_t amount) :
            ManagedPacket(SMSG_GUILD_BANK_MONEY_WITHDRAWN, 0),
            amount(amount)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
                return false;

            packet << amount;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
