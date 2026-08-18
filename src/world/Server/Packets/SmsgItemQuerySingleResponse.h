/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/ItemProperties.hpp"
#include "Storage/MySQLDataStore.hpp"
#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class SmsgItemQuerySingleResponse : public ManagedPacket
    {
    public:
        ItemProperties const* itemProperties = nullptr;
        std::string name;
        std::string description;

        SmsgItemQuerySingleResponse() : SmsgItemQuerySingleResponse(nullptr, "", "")
        {
        }

        SmsgItemQuerySingleResponse(ItemProperties const* itemProperties, std::string name, std::string description) :
            ManagedPacket(SMSG_ITEM_QUERY_SINGLE_RESPONSE, 800),
            itemProperties(itemProperties),
            name(std::move(name)),
            description(std::move(description))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            size_t size = 250 + (5 * 6 * 4) + name.size() + description.size();
            if (itemProperties != nullptr)
                size += itemProperties->generalStatsMap.size() * 8;

            return size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isTbc())
            {
                packet << itemProperties->ItemId;
                packet << itemProperties->Class;
                packet << uint32_t(itemProperties->SubClass);
                packet << itemProperties->unknown_bc;  // soundOverride
                packet << name;
                packet << uint8_t(0);           // name 2?
                packet << uint8_t(0);           // name 3?
                packet << uint8_t(0);           // name 4?
                packet << itemProperties->DisplayInfoID;
                packet << itemProperties->Quality;
                packet << itemProperties->Flags;
                //packet << itemProperties->Flags2;
                packet << itemProperties->BuyPrice;
                packet << itemProperties->SellPrice;
                packet << itemProperties->InventoryType;
                packet << itemProperties->AllowableClass;
                packet << itemProperties->AllowableRace;
                packet << itemProperties->ItemLevel;
                packet << itemProperties->RequiredLevel;
                packet << uint32_t(itemProperties->RequiredSkill);
                packet << itemProperties->RequiredSkillRank;
                packet << itemProperties->RequiredSpell;
                packet << itemProperties->RequiredPlayerRank1;
                packet << itemProperties->RequiredPlayerRank2;
                packet << itemProperties->RequiredFaction;
                packet << itemProperties->RequiredFactionStanding;
                packet << itemProperties->Unique;
                packet << itemProperties->MaxCount;
                packet << itemProperties->ContainerSlots;

                // we have 10 * 8 bytes of stat data
                {
                    auto it = itemProperties->generalStatsMap.begin();
                    for (uint8_t i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
                    {
                        if (it != itemProperties->generalStatsMap.end())
                        {
                            packet << it->first;
                            packet << it->second;
                            ++it;
                        }
                        else
                        {
                            packet << uint32_t(0);
                            packet << int32_t(0);
                        }
                    }
                }

                for (uint8_t i = 0; i < 2; i++)
                {
                    packet << itemProperties->Damage[i].Min;
                    packet << itemProperties->Damage[i].Max;
                    packet << itemProperties->Damage[i].Type;
                }

                for (uint8_t i = 0; i < 3; i++)
                {
                    packet << float(0.0f);
                    packet << float(0.0f);
                    packet << uint32_t(0);
                }

                packet << itemProperties->Armor;

                packet << uint32_t(itemProperties->getStat(ITEM_MOD_HOLY_RESISTANCE));
                packet << uint32_t(itemProperties->getStat(ITEM_MOD_FIRE_RESISTANCE));
                packet << uint32_t(itemProperties->getStat(ITEM_MOD_NATURE_RESISTANCE));
                packet << uint32_t(itemProperties->getStat(ITEM_MOD_FROST_RESISTANCE));
                packet << uint32_t(itemProperties->getStat(ITEM_MOD_SHADOW_RESISTANCE));
                packet << uint32_t(itemProperties->getStat(ITEM_MOD_ARCANE_RESISTANCE));

                packet << itemProperties->Delay;
                packet << itemProperties->AmmoType;
                packet << itemProperties->Range;
                for (uint8_t i = 0; i < 5; i++)
                {
                    packet << itemProperties->Spells[i].Id;
                    packet << itemProperties->Spells[i].Trigger;
                    packet << itemProperties->Spells[i].Charges;
                    packet << itemProperties->Spells[i].Cooldown;
                    packet << itemProperties->Spells[i].Category;
                    packet << itemProperties->Spells[i].CategoryCooldown;
                }
                packet << itemProperties->Bonding;

                packet << description;

                packet << itemProperties->PageId;
                packet << itemProperties->PageLanguage;
                packet << itemProperties->PageMaterial;
                packet << itemProperties->QuestId;
                packet << itemProperties->LockId;
                packet << itemProperties->LockMaterial;
                packet << itemProperties->SheathID;
                packet << itemProperties->RandomPropId;
                packet << itemProperties->RandomSuffixId;
                packet << itemProperties->Block;

                {
                    const auto setBonus = sMySQLStore.getItemSetLinkedBonus(itemProperties->ItemSet);
                    if (setBonus == 0)
                        packet << itemProperties->ItemSet;
                    else
                        packet << setBonus;
                }

                packet << itemProperties->MaxDurability;
                packet << itemProperties->ZoneNameID;
                packet << itemProperties->MapID;
                packet << itemProperties->BagFamily;
                packet << itemProperties->TotemCategory;
                packet << itemProperties->Sockets[0].SocketColor;
                packet << itemProperties->Sockets[0].Unk;
                packet << itemProperties->Sockets[1].SocketColor;
                packet << itemProperties->Sockets[1].Unk;
                packet << itemProperties->Sockets[2].SocketColor;
                packet << itemProperties->Sockets[2].Unk;
                packet << itemProperties->SocketBonus;
                packet << itemProperties->GemProperties;
                packet << itemProperties->DisenchantReqSkill;
                packet << itemProperties->ArmorDamageModifier;
                packet << itemProperties->ExistingDuration;                    // 2.4.2 Item duration in seconds
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                // SMSG_ITEM_QUERY_SINGLE_RESPONSE / CMSG_ITEM_QUERY_SINGLE do not exist in Mop 5.4.8 -
                // Mop clients resolve item data via the DB2/hotfix cache instead, so Mop must fall
                // through to "return false" below, not reuse this legacy WotLK/Cata/TBC-style response.
                packet << itemProperties->ItemId;
                packet << itemProperties->Class;
                packet << uint32_t(itemProperties->SubClass);
                packet << itemProperties->unknown_bc;  // soundOverride
                packet << name;
                packet << uint8_t(0);           // name 2?
                packet << uint8_t(0);           // name 3?
                packet << uint8_t(0);           // name 4?
                packet << itemProperties->DisplayInfoID;
                packet << itemProperties->Quality;
                packet << itemProperties->Flags;
                packet << itemProperties->Flags2;
                packet << itemProperties->BuyPrice;
                packet << itemProperties->SellPrice;
                packet << itemProperties->InventoryType;
                packet << itemProperties->AllowableClass;
                packet << itemProperties->AllowableRace;
                packet << itemProperties->ItemLevel;
                packet << itemProperties->RequiredLevel;
                packet << uint32_t(itemProperties->RequiredSkill);
                packet << itemProperties->RequiredSkillRank;
                packet << itemProperties->RequiredSpell;
                packet << itemProperties->RequiredPlayerRank1;
                packet << itemProperties->RequiredPlayerRank2;
                packet << itemProperties->RequiredFaction;
                packet << itemProperties->RequiredFactionStanding;
                packet << itemProperties->Unique;
                packet << itemProperties->MaxCount;
                packet << itemProperties->ContainerSlots;

                packet << uint32_t(itemProperties->generalStatsMap.size());
                for (auto const& stat : itemProperties->generalStatsMap)
                {
                    packet << stat.first;
                    packet << stat.second;
                }

                packet << itemProperties->ScalingStatsEntry;
                packet << itemProperties->ScalingStatsFlag;

                // originally this went up to 5, now only to 2
                for (uint8_t i = 0; i < 2; i++)
                {
                    packet << itemProperties->Damage[i].Min;
                    packet << itemProperties->Damage[i].Max;
                    packet << itemProperties->Damage[i].Type;
                }
                packet << itemProperties->Armor;

                packet << uint32_t(itemProperties->getStat(ITEM_MOD_HOLY_RESISTANCE));
                packet << uint32_t(itemProperties->getStat(ITEM_MOD_FIRE_RESISTANCE));
                packet << uint32_t(itemProperties->getStat(ITEM_MOD_NATURE_RESISTANCE));
                packet << uint32_t(itemProperties->getStat(ITEM_MOD_FROST_RESISTANCE));
                packet << uint32_t(itemProperties->getStat(ITEM_MOD_SHADOW_RESISTANCE));
                packet << uint32_t(itemProperties->getStat(ITEM_MOD_ARCANE_RESISTANCE));

                packet << itemProperties->Delay;
                packet << itemProperties->AmmoType;
                packet << itemProperties->Range;
                for (uint8_t i = 0; i < 5; i++)
                {
                    packet << itemProperties->Spells[i].Id;
                    packet << itemProperties->Spells[i].Trigger;
                    packet << itemProperties->Spells[i].Charges;
                    packet << itemProperties->Spells[i].Cooldown;
                    packet << itemProperties->Spells[i].Category;
                    packet << itemProperties->Spells[i].CategoryCooldown;
                }
                packet << itemProperties->Bonding;

                packet << description;

                packet << itemProperties->PageId;
                packet << itemProperties->PageLanguage;
                packet << itemProperties->PageMaterial;
                packet << itemProperties->QuestId;
                packet << itemProperties->LockId;
                packet << itemProperties->LockMaterial;
                packet << itemProperties->SheathID;
                packet << itemProperties->RandomPropId;
                packet << itemProperties->RandomSuffixId;
                packet << itemProperties->Block;

                {
                    const auto setBonus = sMySQLStore.getItemSetLinkedBonus(itemProperties->ItemSet);
                    if (setBonus == 0)
                        packet << itemProperties->ItemSet;
                    else
                        packet << setBonus;
                }

                packet << itemProperties->MaxDurability;
                packet << itemProperties->ZoneNameID;
                packet << itemProperties->MapID;
                packet << itemProperties->BagFamily;
                packet << itemProperties->TotemCategory;
                packet << itemProperties->Sockets[0].SocketColor;
                packet << itemProperties->Sockets[0].Unk;
                packet << itemProperties->Sockets[1].SocketColor;
                packet << itemProperties->Sockets[1].Unk;
                packet << itemProperties->Sockets[2].SocketColor;
                packet << itemProperties->Sockets[2].Unk;
                packet << itemProperties->SocketBonus;
                packet << itemProperties->GemProperties;
                packet << itemProperties->DisenchantReqSkill;
                packet << itemProperties->ArmorDamageModifier;
                packet << itemProperties->ExistingDuration;                    // 2.4.2 Item duration in seconds
                packet << itemProperties->ItemLimitCategory;
                packet << itemProperties->HolidayId;                           // HolidayNames.dbc
            }
            else
            {
                return false;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
