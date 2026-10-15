/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildBankWithdrawMoney : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t money;
        uint64_t money64;   // Cata

        CmsgGuildBankWithdrawMoney() : CmsgGuildBankWithdrawMoney(0, 0)
        {
        }

        CmsgGuildBankWithdrawMoney(uint64_t guid, uint32_t money) :
            ManagedPacket(CMSG_GUILD_BANK_WITHDRAW_MONEY, 12),
            guid(guid),
            money(money)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet >> money64;
                money = static_cast<uint32_t>(money64);

                WoWGuid banker;
                banker[1] = packet.readBit();
                banker[3] = packet.readBit();
                banker[7] = packet.readBit();
                banker[6] = packet.readBit();
                banker[5] = packet.readBit();
                banker[0] = packet.readBit();
                banker[4] = packet.readBit();
                banker[2] = packet.readBit();
                packet.readByteSeq(banker[0]);
                packet.readByteSeq(banker[7]);
                packet.readByteSeq(banker[4]);
                packet.readByteSeq(banker[2]);
                packet.readByteSeq(banker[1]);
                packet.readByteSeq(banker[6]);
                packet.readByteSeq(banker[3]);
                packet.readByteSeq(banker[5]);
                guid = banker.getRawGuid();
                return true;
            }

            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                WoWGuid128 banker;
                packet >> banker;
                guid = WoWGuid::fromGuid128(banker).getRawGuid();
                packet >> money64;
                money = static_cast<uint32_t>(money64);
                return true;
            }

            packet >> guid;

            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> money;
            }
            else // Cata and greater
            {
                packet >> money64;
                money = static_cast<uint32_t>(money);
            }
            return true;
        }
    };
}
