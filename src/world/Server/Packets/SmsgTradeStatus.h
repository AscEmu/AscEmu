/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgTradeStatus : public ManagedPacket
    {
    public:
        uint32_t staus;
        uint64_t guid;

        SmsgTradeStatus() : SmsgTradeStatus(0, 0)
        {
        }

        SmsgTradeStatus(uint32_t staus, uint64_t guid) :
            ManagedPacket(SMSG_TRADE_STATUS, 12),
            staus(staus),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << staus;
            if (guid != 0)
                packet << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
