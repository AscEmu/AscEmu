/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
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
            packet << error;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
