/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/ItemDefines.hpp"
#include "Macros/GuildMacros.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // 7.x and later move guild bank items with one request per action, the request names the action. Every request
    // starts with the banker, the bank tab and the bank slot.
    class CmsgGuildBankMoveItem : public ManagedPacket
    {
    public:
        uint64_t bankGuid = 0;
        bool bankToBank = false;
        bool toChar = false;
        uint8_t tabId = 0;
        uint8_t slotId = 0;
        uint8_t destTabId = 0;
        uint8_t destSlotId = 0;
        uint8_t playerBag = INVALID_BACKPACK_SLOT;
        uint8_t playerSlotId = UNDEFINED_TAB_SLOT;
        uint32_t splitedAmount = 0;

        explicit CmsgGuildBankMoveItem(uint16_t opcode) : ManagedPacket(opcode, 0)
        {
        }

    protected:
        // player slot, optional count, optional bag
        void readInventorySlot(WorldPacket& packet, bool withCount)
        {
            packet >> playerSlotId;
            if (withCount)
                packet >> splitedAmount;
            if (packet.readBit())
                packet >> playerBag;
        }

        void readBankSlot(WorldPacket& packet, bool withCount)
        {
            bankToBank = true;
            packet >> destTabId >> destSlotId;
            if (withCount)
                packet >> splitedAmount;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            WoWGuid128 banker;
            packet >> banker >> tabId >> slotId;
            bankGuid = WoWGuid::fromGuid128(banker).getRawGuid();

            switch (m_opcode)
            {
                // 7.x and 8.x
                case CMSG_GUILD_BANK_MOVE_ITEMS_PLAYER_BANK:        readInventorySlot(packet, false); toChar = false; return true;
                case CMSG_GUILD_BANK_MOVE_ITEMS_BANK_PLAYER:        readInventorySlot(packet, false); toChar = true; return true;
                case CMSG_GUILD_BANK_SWAP_ITEMS_BANK_PLAYER:        readInventorySlot(packet, false); toChar = true; return true;
                case CMSG_GUILD_BANK_MOVE_ITEMS_PLAYER_BANK_COUNT:  readInventorySlot(packet, true); toChar = false; return true;
                case CMSG_GUILD_BANK_MERGE_ITEMS_PLAYER_BANK:       readInventorySlot(packet, true); toChar = false; return true;
                case CMSG_GUILD_BANK_MOVE_ITEMS_BANK_PLAYER_COUNT:  readInventorySlot(packet, true); toChar = true; return true;
                case CMSG_GUILD_BANK_MERGE_ITEMS_BANK_PLAYER:       readInventorySlot(packet, true); toChar = true; return true;
                case CMSG_GUILD_BANK_MOVE_ITEMS_BANK_PLAYER_AUTO:   toChar = true; return true;
                case CMSG_GUILD_BANK_MOVE_ITEMS_BANK_BANK:          readBankSlot(packet, false); return true;
                case CMSG_GUILD_BANK_SWAP_ITEMS_BANK_BANK:          readBankSlot(packet, false); return true;
                case CMSG_GUILD_BANK_MOVE_ITEMS_BANK_BANK_COUNT:    readBankSlot(packet, true); return true;
                case CMSG_GUILD_BANK_MERGE_ITEMS_BANK_BANK:         readBankSlot(packet, true); return true;

                // 9.x and later
                case CMSG_AUTO_GUILD_BANK_ITEM:                     readInventorySlot(packet, false); toChar = false; return true;
                case CMSG_SWAP_ITEM_WITH_GUILD_BANK_ITEM:           readInventorySlot(packet, false); toChar = false; return true;
                case CMSG_STORE_GUILD_BANK_ITEM:                    readInventorySlot(packet, false); toChar = true; return true;
                case CMSG_MERGE_ITEM_WITH_GUILD_BANK_ITEM:          readInventorySlot(packet, true); toChar = false; return true;
                case CMSG_SPLIT_ITEM_TO_GUILD_BANK:                 readInventorySlot(packet, true); toChar = false; return true;
                case CMSG_MERGE_GUILD_BANK_ITEM_WITH_ITEM:          readInventorySlot(packet, true); toChar = true; return true;
                case CMSG_SPLIT_GUILD_BANK_ITEM_TO_INVENTORY:       readInventorySlot(packet, true); toChar = true; return true;
                case CMSG_AUTO_STORE_GUILD_BANK_ITEM:               toChar = true; return true;
                case CMSG_SWAP_GUILD_BANK_ITEM_WITH_GUILD_BANK_ITEM: readBankSlot(packet, false); return true;
                case CMSG_MOVE_GUILD_BANK_ITEM:                     readBankSlot(packet, false); return true;
                case CMSG_MERGE_GUILD_BANK_ITEM_WITH_GUILD_BANK_ITEM: readBankSlot(packet, true); return true;
                case CMSG_SPLIT_GUILD_BANK_ITEM:                    readBankSlot(packet, true); return true;
                default:
                    return false;
            }
        }
    };
}
