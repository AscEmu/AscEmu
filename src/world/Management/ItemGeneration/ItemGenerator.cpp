/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ItemGenerator.hpp"

#if defined(AE_FOREVER)
#include "Objects/ItemDefines.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

#include <algorithm>
#include <cmath>
#endif

namespace AscEmu::Items
{
#if defined(AE_FOREVER)
    namespace
    {
        std::optional<uint32_t> getRandomPropertyIndex(uint32_t inventoryType, uint32_t subClass)
        {
            switch (inventoryType)
            {
                case INVTYPE_HEAD:
                case INVTYPE_BODY:
                case INVTYPE_CHEST:
                case INVTYPE_LEGS:
                case INVTYPE_RANGED:
                case INVTYPE_2HWEAPON:
                case INVTYPE_ROBE:
                case INVTYPE_THROWN:
                    return 0;
                case INVTYPE_RANGEDRIGHT:
                    return subClass == ITEM_SUBCLASS_WEAPON_WAND ? 3U : 0U;
                case INVTYPE_WEAPON:
                case INVTYPE_WEAPONMAINHAND:
                case INVTYPE_WEAPONOFFHAND:
                    return 3;
                case INVTYPE_SHOULDERS:
                case INVTYPE_WAIST:
                case INVTYPE_FEET:
                case INVTYPE_HANDS:
                case INVTYPE_TRINKET:
                    return 1;
                case INVTYPE_NECK:
                case INVTYPE_WRISTS:
                case INVTYPE_FINGER:
                case INVTYPE_SHIELD:
                case INVTYPE_CLOAK:
                case INVTYPE_HOLDABLE:
                    return 2;
                case INVTYPE_RELIC:
                    return 4;
                default:
                    return std::nullopt;
            }
        }

        float getRandomPropertyPoints(uint32_t itemLevel, uint32_t quality, uint32_t inventoryType, uint32_t subClass)
        {
            auto const propIndex = getRandomPropertyIndex(inventoryType, subClass);
            if (!propIndex)
                return 0.0f;

            auto const* points = sRandPropPointsStore.lookupEntry(itemLevel);
            if (!points)
                return 0.0f;

            switch (quality)
            {
                case ITEM_QUALITY_UNCOMMON:
                    return points->GoodF[*propIndex];
                case ITEM_QUALITY_RARE:
                case ITEM_QUALITY_HEIRLOOM:
                    return points->SuperiorF[*propIndex];
                case ITEM_QUALITY_EPIC:
                case ITEM_QUALITY_LEGENDARY:
                case ITEM_QUALITY_ARTIFACT:
                    return points->EpicF[*propIndex];
                default:
                    return 0.0f;
            }
        }

        StatOverride const* findOverride(GenerationOverrides const& overrides, int32_t type)
        {
            auto const itr = std::find_if(overrides.stats.begin(), overrides.stats.end(),
                [type](StatOverride const& overrideData) { return overrideData.type == type; });
            return itr != overrides.stats.end() ? &*itr : nullptr;
        }

        WDB::Structures::ItemArmorQualityEntry const* getArmorQuality(uint32_t itemLevel)
        {
            if (auto const* entry = sItemArmorQualityForeverStore.lookupEntry(itemLevel))
                return entry;
            for (auto const& [id, entry] : sItemArmorQualityForeverStore)
                if (id == itemLevel) return &entry;
            return nullptr;
        }

        WDB::Structures::ItemArmorTotalEntry const* getArmorTotal(uint32_t itemLevel)
        {
            if (auto const* entry = sItemArmorTotalForeverStore.lookupEntry(itemLevel); entry && entry->ItemLevel == itemLevel)
                return entry;
            for (auto const& [id, entry] : sItemArmorTotalForeverStore)
                if (entry.ItemLevel == itemLevel) return &entry;
            return nullptr;
        }

        WDB::Structures::ItemArmorShieldEntry const* getArmorShield(uint32_t itemLevel)
        {
            if (auto const* entry = sItemArmorShieldForeverStore.lookupEntry(itemLevel); entry && entry->ItemLevel == itemLevel)
                return entry;
            for (auto const& [id, entry] : sItemArmorShieldForeverStore)
                if (entry.ItemLevel == itemLevel) return &entry;
            return nullptr;
        }

        uint32_t generateArmor(uint32_t itemLevel, uint32_t itemClass, uint32_t subClass, uint32_t quality, uint32_t inventoryType)
        {
            if (itemClass != ITEM_CLASS_ARMOR || quality >= 7) return 0;
            if (subClass == ITEM_SUBCLASS_ARMOR_SHIELD)
            {
                auto const* shield = getArmorShield(itemLevel);
                return shield ? static_cast<uint32_t>(std::lround(std::max(0.0f, shield->Quality[quality]))) : 0;
            }
            if (subClass < ITEM_SUBCLASS_ARMOR_CLOTH || subClass > ITEM_SUBCLASS_ARMOR_PLATE_MAIL) return 0;
            uint32_t const locationId = inventoryType == INVTYPE_ROBE ? INVTYPE_CHEST : inventoryType;
            auto const* location = sArmorLocationForeverStore.lookupEntry(locationId);
            auto const* qualityEntry = getArmorQuality(itemLevel);
            auto const* total = getArmorTotal(itemLevel);
            if (!location || !qualityEntry || !total) return 0;
            uint32_t const armorIndex = subClass - ITEM_SUBCLASS_ARMOR_CLOTH;
            float const armor = qualityEntry->Quality[quality] * total->Armor[armorIndex] * location->ArmorModifier[armorIndex];
            return static_cast<uint32_t>(std::lround(std::max(0.0f, armor)));
        }

        WDB::Structures::ItemDamageEntry const* getThrownDamage(uint32_t itemLevel)
        {
            if (auto const* entry = sItemDamageThrownForeverStore.lookupEntry(itemLevel); entry && entry->ItemLevel == itemLevel)
                return entry;
            for (auto const& [id, entry] : sItemDamageThrownForeverStore)
                if (entry.ItemLevel == itemLevel) return &entry;
            return nullptr;
        }

        void generateWeaponDamage(uint32_t itemLevel, uint32_t itemClass, uint32_t quality, uint32_t inventoryType, uint32_t delay, float variance, float& minDamage, float& maxDamage)
        {
            minDamage = 0.0f;
            maxDamage = 0.0f;
            if (itemClass != ITEM_CLASS_WEAPON || inventoryType != INVTYPE_THROWN || quality >= 7 || delay == 0)
                return;

            auto const* damage = getThrownDamage(itemLevel);
            if (!damage)
                return;

            float const dps = damage->Quality[quality];
            if (dps <= 0.0f)
                return;

            float const average = dps * static_cast<float>(delay) * 0.001f;
            minDamage = std::max(0.0f, (1.0f - variance * 0.5f) * average);
            maxDamage = std::max(minDamage, std::floor(average * (1.0f + variance * 0.5f) + 0.5f));
        }

        float getDurabilityQualityModifier(uint32_t quality)
        {
            switch (quality)
            {
                case ITEM_QUALITY_POOR:
                case ITEM_QUALITY_NORMAL:
                case ITEM_QUALITY_UNCOMMON: return 1.0f;
                case ITEM_QUALITY_RARE: return 1.17f;
                case ITEM_QUALITY_EPIC: return 1.37f;
                case ITEM_QUALITY_LEGENDARY: return 1.68f;
                default: return 0.0f;
            }
        }

        float getArmorDurabilitySlotModifier(uint32_t inventoryType)
        {
            switch (inventoryType)
            {
                case INVTYPE_HANDS:
                case INVTYPE_WAIST:
                case INVTYPE_WRISTS: return 0.35f;
                case INVTYPE_FEET: return 0.49f;
                case INVTYPE_HEAD:
                case INVTYPE_SHOULDERS: return 0.59f;
                case INVTYPE_LEGS: return 0.75f;
                case INVTYPE_CHEST:
                case INVTYPE_ROBE: return 1.0f;
                default: return 0.0f;
            }
        }

        float getArmorDurabilitySubclassModifier(uint32_t subClass)
        {
            switch (subClass)
            {
                case ITEM_SUBCLASS_ARMOR_CLOTH: return 0.63f;
                case ITEM_SUBCLASS_ARMOR_LEATHER: return 0.76f;
                case ITEM_SUBCLASS_ARMOR_MAIL: return 0.89f;
                case ITEM_SUBCLASS_ARMOR_PLATE_MAIL: return 1.0f;
                default: return 0.0f;
            }
        }

        float getWeaponDurabilityModifier(uint32_t subClass)
        {
            switch (subClass)
            {
                case ITEM_SUBCLASS_WEAPON_WAND:
                case ITEM_SUBCLASS_WEAPON_DAGGER:
                case ITEM_SUBCLASS_WEAPON_FIST_WEAPON: return 0.64f;
                case ITEM_SUBCLASS_WEAPON_BOW:
                case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                case ITEM_SUBCLASS_WEAPON_GUN: return 0.77f;
                case ITEM_SUBCLASS_WEAPON_AXE:
                case ITEM_SUBCLASS_WEAPON_SWORD:
                case ITEM_SUBCLASS_WEAPON_MACE: return 0.89f;
                case ITEM_SUBCLASS_WEAPON_TWOHAND_AXE:
                case ITEM_SUBCLASS_WEAPON_TWOHAND_MACE:
                case ITEM_SUBCLASS_WEAPON_POLEARM:
                case ITEM_SUBCLASS_WEAPON_TWOHAND_SWORD:
                case ITEM_SUBCLASS_WEAPON_STAFF: return 1.0f;
                default: return 0.0f;
            }
        }

        uint32_t generateMaxDurability(uint32_t itemLevel, uint32_t itemClass, uint32_t subClass, uint32_t quality, uint32_t inventoryType)
        {
            float const qualityModifier = getDurabilityQualityModifier(quality);
            if (qualityModifier <= 0.0f) return 0;
            float base = 0.0f;
            if (itemClass == ITEM_CLASS_ARMOR && subClass == ITEM_SUBCLASS_ARMOR_SHIELD) base = 5.0f * std::round(17.0f * qualityModifier);
            else if (itemClass == ITEM_CLASS_ARMOR)
            {
                float const slotModifier = getArmorDurabilitySlotModifier(inventoryType);
                float const armorModifier = getArmorDurabilitySubclassModifier(subClass);
                if (slotModifier <= 0.0f || armorModifier <= 0.0f) return 0;
                base = 5.0f * std::round(23.0f * qualityModifier * slotModifier * armorModifier);
            }
            else if (itemClass == ITEM_CLASS_WEAPON)
            {
                float const weaponModifier = getWeaponDurabilityModifier(subClass);
                if (weaponModifier <= 0.0f) return 0;
                base = 5.0f * std::round(17.0f * qualityModifier * weaponModifier);
            }
            else return 0;
            if (itemLevel <= 28) base *= 0.966f - static_cast<float>(28 - itemLevel) / 54.0f;
            return static_cast<uint32_t>(std::max<long>(0, std::lround(base)));
        }

        std::vector<ResolvedItemEffect> resolveEffects(uint32_t itemId)
        {
            struct EffectRef { uint32_t id; WDB::Structures::ItemEffectEntry const* effect; };
            std::vector<EffectRef> refs;
            for (auto const& [id, relation] : sItemXItemEffectForeverStore)
            {
                if (relation.ItemID != itemId) continue;
                if (auto const* effect = sItemEffectForeverStore.lookupEntry(relation.ItemEffectID)) refs.push_back({relation.ItemEffectID, effect});
            }
            std::sort(refs.begin(), refs.end(), [](EffectRef const& left, EffectRef const& right) { if (left.effect->LegacySlotIndex != right.effect->LegacySlotIndex) return left.effect->LegacySlotIndex < right.effect->LegacySlotIndex; return left.id < right.id; });
            std::vector<ResolvedItemEffect> result;
            result.reserve(refs.size());
            for (auto const& ref : refs) result.push_back({ref.effect->LegacySlotIndex, ref.effect->SpellID, static_cast<uint32_t>(std::max(0, ref.effect->TriggerType)), ref.effect->Charges, ref.effect->Cooldown, ref.effect->Category, ref.effect->CategoryCooldown});
            return result;
        }
    }
#endif

    std::optional<GeneratedItemData> generateItemData(uint32_t entry, GenerationOverrides const& overrides)
    {
#if defined(AE_FOREVER)
        auto const* sparse = sItemSparseStore.lookupEntry(entry);
        auto const* item = sItemStore.lookupEntry(entry);
        if (!sparse || !item)
            return std::nullopt;

        GeneratedItemData result;
        result.entry = entry;
        result.baseItemLevel = sparse->ItemLevel;
        result.itemLevel = overrides.itemLevel.value_or(sparse->ItemLevel);
        result.quality = overrides.quality.value_or(static_cast<uint32_t>(static_cast<uint8_t>(sparse->OverallQualityID)));
        result.inventoryType = static_cast<uint32_t>(static_cast<uint8_t>(sparse->InventoryType));
        result.randomPropertyPoints = getRandomPropertyPoints(result.itemLevel, result.quality, result.inventoryType, item->SubClass);
        result.armor = generateArmor(result.itemLevel, item->Class, item->SubClass, result.quality, result.inventoryType);
        generateWeaponDamage(result.itemLevel, item->Class, result.quality, result.inventoryType, sparse->ItemDelay, sparse->DmgVariance, result.damageMin, result.damageMax);
        result.damageType = sparse->DamageType;
        result.maxDurability = generateMaxDurability(result.itemLevel, item->Class, item->SubClass, result.quality, result.inventoryType);
        result.effects = resolveEffects(entry);
        result.stats.reserve(10 + overrides.stats.size());

        for (uint32_t index = 0; index < 10; ++index)
        {
            int32_t const type = sparse->StatModifierBonusStat[index];
            if (type < 0)
                continue;

            StatOverride const* overrideData = findOverride(overrides, type);
            if (overrideData && overrideData->remove)
                continue;

            ResolvedStat stat;
            stat.type = type;
            stat.percentageOfSocket = sparse->StatPercentageOfSocket[index];
            stat.percentEditor = overrideData && overrideData->percentEditor
                ? *overrideData->percentEditor
                : sparse->StatPercentEditor[index];

            if (overrideData && overrideData->fixedValue)
            {
                stat.value = *overrideData->fixedValue;
                stat.fixed = true;
            }
            else if (result.randomPropertyPoints > 0.0f && stat.percentEditor != 0)
            {
                // Blizzard stores allocation in 1/10000ths of the slot/quality
                // budget. Socket-cost handling is intentionally kept separate;
                // Forever items observed so far with zero socket percentages map
                // directly through this formula.
                stat.value = static_cast<int32_t>(std::lround(
                    result.randomPropertyPoints * static_cast<float>(stat.percentEditor) / 10000.0f));
            }

            result.stats.push_back(stat);
        }

        // Allow a custom item/override to add a completely new fixed or scaled
        // stat without forcing a full replacement of the Blizzard definition.
        for (StatOverride const& overrideData : overrides.stats)
        {
            if (overrideData.type < 0 || overrideData.remove)
                continue;

            bool const alreadyPresent = std::any_of(result.stats.begin(), result.stats.end(),
                [&overrideData](ResolvedStat const& stat) { return stat.type == overrideData.type; });
            if (alreadyPresent)
                continue;

            ResolvedStat stat;
            stat.type = overrideData.type;
            stat.percentEditor = overrideData.percentEditor.value_or(0);
            if (overrideData.fixedValue)
            {
                stat.value = *overrideData.fixedValue;
                stat.fixed = true;
            }
            else if (result.randomPropertyPoints > 0.0f && stat.percentEditor != 0)
            {
                stat.value = static_cast<int32_t>(std::lround(
                    result.randomPropertyPoints * static_cast<float>(stat.percentEditor) / 10000.0f));
            }
            result.stats.push_back(stat);
        }

        return result;
#else
        (void)entry;
        (void)overrides;
        return std::nullopt;
#endif
    }
}
