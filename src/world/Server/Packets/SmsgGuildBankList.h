/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Guild/GuildBankTab.hpp"
#include <cstdint>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    struct GuildBankListTabInfo
    {
        std::string name;
        std::string icon;
    };

    struct GuildBankListItemSlot
    {
        uint8_t slotId = 0;
        uint32_t entry = 0;
        uint32_t stackCount = 0;
        uint32_t randomPropertiesId = 0;
        uint32_t propertySeed = 0;
        uint32_t spellCharges = 0; // Mop only
        std::vector<std::pair<uint32_t, uint32_t>> enchants; // {enchantId, enchantSlot}
    };

    class SmsgGuildBankList : public ManagedPacket
    {
    public:
        uint64_t bankMoney = 0;
        uint8_t tabId = 0;
        uint32_t remainingSlots = 0; // mutable
        bool sendAllSlots = false;
        bool withTabInfo = false;
        bool withContent = false; // Mop only
        uint8_t tabInfoBitWidth = 21; // Mop only

        const GuildBankTab* tab = nullptr; // pre-Cata only
        std::set<uint8_t> slots; // pre-Cata only

        std::vector<GuildBankListTabInfo> tabInfo; // used when withTabInfo is true
        std::vector<GuildBankListItemSlot> items; // Cata/Mop only

        SmsgGuildBankList() : SmsgGuildBankList(0, 0, 0, false, false, 21, nullptr, {}, {}, {})
        {
        }

        SmsgGuildBankList(uint64_t bankMoney, uint8_t tabId, uint32_t remainingSlots, bool sendAllSlots, bool withTabInfo,
            uint8_t tabInfoBitWidth, const GuildBankTab* tab, std::set<uint8_t> slots,
            std::vector<GuildBankListTabInfo> tabInfo, std::vector<GuildBankListItemSlot> items) :
            ManagedPacket(SMSG_GUILD_BANK_LIST, 1200),
            bankMoney(bankMoney), tabId(tabId), remainingSlots(remainingSlots), sendAllSlots(sendAllSlots), withTabInfo(withTabInfo),
            tabInfoBitWidth(tabInfoBitWidth), tab(tab), slots(std::move(slots)), tabInfo(std::move(tabInfo)), items(std::move(items))
        {
        }

    protected:
        size_t expectedSize() const override { return 1200 + items.size() * 44 + tabInfo.size() * 24; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << uint64_t(bankMoney);
                packet << uint8_t(tabId);
                packet << uint32_t(remainingSlots);
                packet << uint8_t(sendAllSlots);

                if (withTabInfo)
                {
                    packet << uint8_t(tabInfo.size());
                    for (const auto& t : tabInfo)
                    {
                        packet << t.name;
                        packet << t.icon;
                    }
                }

                if (!tab)
                {
                    packet << uint8_t(0);
                }
                else if (sendAllSlots)
                {
                    tab->writeInfoPacket(packet);
                }
                else if (!slots.empty())
                {
                    packet << uint8_t(slots.size());
                    for (auto slotId : slots)
                        tab->writeSlotPacket(packet, slotId, false);
                }
                else
                {
                    packet << uint8_t(0);
                }

                return true;
            }
            else if (m_protocol.isCata())
            {
                ByteBuffer tabData;

                packet.writeBit(0);
                packet.writeBits(items.size(), 20);
                packet.writeBits(withTabInfo ? tabInfo.size() : 0, 22);

                for (const auto& item : items)
                {
                    packet.writeBit(0);
                    packet.writeBits(item.enchants.size(), 23);

                    for (const auto& ench : item.enchants)
                    {
                        tabData << uint32_t(ench.first);
                        tabData << uint32_t(ench.second);
                    }

                    tabData << uint32_t(0);
                    tabData << uint32_t(0);
                    tabData << uint32_t(0);
                    tabData << uint32_t(item.stackCount);
                    tabData << uint32_t(item.slotId);
                    tabData << uint32_t(0);
                    tabData << uint32_t(item.entry);
                    tabData << uint32_t(item.randomPropertiesId);
                    tabData << uint32_t(0); // @todo add Spell Charges
                    tabData << uint32_t(item.propertySeed);
                }

                if (withTabInfo)
                {
                    for (const auto& t : tabInfo)
                    {
                        packet.writeBits(t.icon.length(), 9);
                        packet.writeBits(t.name.length(), 7);
                    }
                }

                packet.flushBits();

                if (withTabInfo)
                {
                    for (size_t i = 0; i < tabInfo.size(); ++i)
                    {
                        packet.writeString(tabInfo[i].icon);
                        packet << uint32_t(i);
                        packet.writeString(tabInfo[i].name);
                    }
                }

                packet << uint64_t(bankMoney);
                if (tabData.size())
                    packet.append(tabData);

                packet << uint32_t(tabId);
                packet << uint32_t(remainingSlots);

                return true;
            }
            else if (m_protocol.isMop())
            {
                ByteBuffer tabData;

                packet << uint32_t(tabId);
                packet << uint64_t(bankMoney);
                packet << uint32_t(remainingSlots);

                packet.writeBit(withContent && withTabInfo);
                packet.writeBits(withTabInfo ? tabInfo.size() : 0, tabInfoBitWidth);
                packet.writeBits(items.size(), 18);

                if (withTabInfo)
                {
                    for (const auto& t : tabInfo)
                    {
                        packet.writeBits(t.icon.length(), 9);
                        packet.writeBits(t.name.length(), 7);
                    }
                }

                for (const auto& item : items)
                {
                    tabData << uint32_t(0);
                    tabData << uint32_t(0);

                    for (const auto& ench : item.enchants)
                    {
                        tabData << uint32_t(ench.first);
                        tabData << uint32_t(ench.second);
                    }

                    packet.writeBit(0);
                    packet.writeBits(item.enchants.size(), 21);

                    tabData << uint32_t(0);
                    tabData << uint32_t(0);
                    tabData << uint32_t(item.entry);
                    tabData << uint32_t(item.spellCharges);
                    tabData << uint32_t(item.stackCount);
                    tabData << uint32_t(item.slotId);
                    tabData << uint32_t(item.randomPropertiesId);
                    tabData << uint32_t(item.propertySeed);
                }

                packet.flushBits();

                if (tabData.size())
                    packet.append(tabData);

                if (withTabInfo)
                {
                    for (size_t i = 0; i < tabInfo.size(); ++i)
                    {
                        packet << uint32_t(i);
                        packet.writeString(tabInfo[i].icon);
                        packet.writeString(tabInfo[i].name);
                    }
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
