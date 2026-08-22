/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>


namespace AscEmu::Packets
{
    class CmsgGuildBankSwapItems : public ManagedPacket
    {
    public:
        uint64_t bankGuid;
        uint8_t bankToBank;
        uint8_t tabId;
        uint8_t slotId;
        uint32_t itemEntry;
        uint32_t splitedAmount = 0;

        // banktobank specific
        uint8_t destTabId = 0;
        uint8_t destSlotId = 0;

        // banktoplayer specific
        uint8_t playerBag = 0;
        uint8_t playerSlotId = 255;
        uint8_t toCharNum = 1;
        uint8_t autoStore = 0;

        bool toChar = false;

        CmsgGuildBankSwapItems() : CmsgGuildBankSwapItems(0, 0, 0, 0, 0, 0)
        {
        }

        CmsgGuildBankSwapItems(uint64_t bankGuid, uint8_t bankToBank, uint8_t tabId, uint8_t slotId, uint32_t itemEntry, uint32_t splitedAmount) :
            ManagedPacket(CMSG_GUILD_BANK_SWAP_ITEMS, 21),
            bankGuid(bankGuid),
            bankToBank(bankToBank),
            tabId(tabId),
            slotId(slotId),
            itemEntry(itemEntry),
            splitedAmount(splitedAmount)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> bankGuid >> bankToBank;
                if (bankToBank)
                {
                    packet >> destTabId >> destSlotId;
                    packet.readSkip<uint32_t>();

                    packet >> tabId >> slotId >> itemEntry;
                    packet.readSkip<uint8_t>();

                    packet >> splitedAmount;
                }
                else
                {
                    packet >> tabId >> slotId >> itemEntry >> autoStore;
                    if (autoStore)
                    {
                        packet.readSkip<uint32_t>();
                        packet.readSkip<uint8_t>();
                        packet.readSkip<uint32_t>();
                    }
                    else
                    {
                        packet >> playerBag >> playerSlotId >> toCharNum >> splitedAmount;
                    }

                    toChar = toCharNum > 0;
                }
                return true;
            }

            return false;
        }
    };
}
