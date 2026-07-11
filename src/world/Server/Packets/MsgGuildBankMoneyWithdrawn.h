/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class MsgGuildBankMoneyWithdrawn : public ManagedPacket
    {
    public:
        int32_t amount;

        MsgGuildBankMoneyWithdrawn() : MsgGuildBankMoneyWithdrawn(0)
        {
        }

        MsgGuildBankMoneyWithdrawn(int32_t amount) :
            ManagedPacket(MSG_GUILD_BANK_MONEY_WITHDRAWN, 0),
            amount(amount)
        {
        }

    protected:
        size_t expectedSize() const override { return m_protocol.expansion < WoW::Expansion::_Cata ? 4 : 0; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
                return false;

            packet << amount;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
