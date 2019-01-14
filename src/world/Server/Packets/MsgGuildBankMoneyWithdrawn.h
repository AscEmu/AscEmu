/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class MsgGuildBankMoneyWithdrawn : public ManagedPacket
    {
#if VERSION_STRING < Cata
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
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << amount;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
#endif
    };
}}
