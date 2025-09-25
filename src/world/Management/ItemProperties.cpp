/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ItemProperties.hpp"
#include "Utilities/Narrow.hpp"

#include <cstdint>

class Spell;

bool ItemProperties::HasFlag(uint32_t flag) const
{
    if ((Flags & flag) != 0)
        return true;

    return false;
}

bool ItemProperties::HasFlag2(uint32_t flag) const
{
    if ((Flags2 & flag) != 0)
        return true;

    return false;
}

bool ItemProperties::isPotion() const { return Class == ITEM_CLASS_CONSUMABLE && SubClass == ITEM_SUBCLASS_POTION; }
bool ItemProperties::isVellum() const { return Class == ITEM_CLASS_TRADEGOODS && SubClass == ITEM_SUBCLASS_ARMOR_ENCHANTMENT; }
bool ItemProperties::isConjuredConsumable() const { return Class == ITEM_CLASS_CONSUMABLE && (Flags & ITEM_FLAG_CONJURED); }
bool ItemProperties::isCurrencyToken() const { return BagFamily & ITEM_TYPE_CURRENCY; }

bool ItemProperties::isRangedWeapon() const
{
    return Class == ITEM_CLASS_WEAPON ||
        SubClass == ITEM_SUBCLASS_WEAPON_BOW ||
        SubClass == ITEM_SUBCLASS_WEAPON_GUN ||
        SubClass == ITEM_SUBCLASS_WEAPON_CROSSBOW;
}

uint32_t ItemProperties::getBuyPriceForItem(uint32_t count, uint32_t factionStanding) const
{
    const float pricemod[9] =
    {
        1.0f,        // HATED
        1.0f,        // HOSTILE
        1.0f,        // UNFRIENDLY
        1.0f,        // NEUTRAL
        0.95f,       // FRIENDLY
        0.90f,       // HONORED
        0.85f,       // REVERED
        0.80f        // EXHALTED
    };

    int32_t cost = BuyPrice;

    if (factionStanding && factionStanding <= 8)
        cost = Util::float2int32(ceilf(BuyPrice * pricemod[factionStanding]));

    return cost * count;
}

void ItemProperties::addStat(uint32_t type, int32_t value)
{
    if (type <= ITEM_MOD_EXTRA_ARMOR && generalStatsMap.size() < MAX_ITEM_PROTO_STATS)
        generalStatsMap[type] = value;
    
    if (type >= ITEM_MOD_HOLY_RESISTANCE && type <= ITEM_MOD_ARCANE_RESISTANCE)
        resistanceStatsMap[type] = value;
}

int32_t ItemProperties::getStat(uint32_t type) const
{
    if (type <= ITEM_MOD_EXTRA_ARMOR)
    {
        auto itr = generalStatsMap.find(type);
        if (itr != generalStatsMap.end())
            return itr->second;
    }

    if (type >= ITEM_MOD_HOLY_RESISTANCE && type <= ITEM_MOD_ARCANE_RESISTANCE)
    {
        auto itr = resistanceStatsMap.find(type);
        if (itr != resistanceStatsMap.end())
            return itr->second;
    }

    return 0;
}

bool ItemProperties::hasStat(uint32_t type) const
{
    return generalStatsMap.find(type) != generalStatsMap.end();
}
