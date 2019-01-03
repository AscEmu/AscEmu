/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
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

        //banktobank specific
        uint8_t destTabId = 0;
        uint8_t destSlotId = 0;

        //banktoplayer specific
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> bankGuid >> bankToBank;
            if (bankToBank)
            {
                packet >> destTabId >> destSlotId;
                packet.read_skip<uint32_t>();

                packet >> tabId >> slotId >> itemEntry;
                packet.read_skip<uint8_t>();

                packet >> splitedAmount;
            }
            else
            {
                packet >> tabId >> slotId >> itemEntry >> autoStore;
                if (autoStore)
                {
                    packet.read_skip<uint32_t>();
                    packet.read_skip<uint8_t>();
                    packet.read_skip<uint32_t>();
                }
                else
                {
                    packet >> playerBag >> playerSlotId >> toCharNum >> splitedAmount;
                }

                toChar = toCharNum > 0;
            }
            return true;
        }
    };
}}
