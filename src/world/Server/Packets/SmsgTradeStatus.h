/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
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
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << staus;
                if (guid != 0)
                    packet << guid;
            }
            else // > Cata
            {
                packet.writeBit(false);
                packet.writeBits(staus, 5);

                switch (staus)
                {
                    case TRADE_STATUS_PROPOSED:
                    {
                        WoWGuid wGuid = guid;

                        packet.writeBit(wGuid[2]);
                        packet.writeBit(wGuid[4]);
                        packet.writeBit(wGuid[6]);
                        packet.writeBit(wGuid[0]);
                        packet.writeBit(wGuid[1]);
                        packet.writeBit(wGuid[3]);
                        packet.writeBit(wGuid[7]);
                        packet.writeBit(wGuid[5]);

                        packet.flushBits();

                        packet.writeByteSeq(wGuid[4]);
                        packet.writeByteSeq(wGuid[1]);
                        packet.writeByteSeq(wGuid[2]);
                        packet.writeByteSeq(wGuid[3]);
                        packet.writeByteSeq(wGuid[0]);
                        packet.writeByteSeq(wGuid[7]);
                        packet.writeByteSeq(wGuid[6]);
                        packet.writeByteSeq(wGuid[5]);
                        break;
                    }
                    case TRADE_STATUS_INITIATED:
                    {
                        packet.flushBits();
                        packet << uint32_t(0);
                        break;
                    }
                    case TRADE_STATUS_FAILED:
                    {
                        packet.writeBit(false);
                        packet.flushBits();
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        break;
                    }
                    case TRADE_STATUS_LOOT_ITEM:
                    case TRADE_STATUS_ONLY_CONJURED:
                    {
                        packet.flushBits();
                        packet << uint8_t(0);
                        break;
                    }
                    case TRADE_STATUS_CURRENCY_NOT_TRADEABLE:
                    case TRADE_STATUS_CURRENCY:
                    {
                        packet.flushBits();
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                    }
                    default:
                        packet.flushBits();
                        break;
                }
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
