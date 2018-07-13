/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgGuildBankMoneyWithdrawn : public ManagedPacket
    {
#if VERSION_STRING == Cata
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
            packet << amount;
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            return false;
        }
#endif
    };
}}
