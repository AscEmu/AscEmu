/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2011 <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "DBCGlobals.hpp"

typedef std::map<WMOAreaTableTripple, DBC::Structures::WMOAreaTableEntry const*> WMOAreaInfoByTripple;

#ifdef ENABLE_ACHIEVEMENTS
SERVER_DECL DBC::DBCStorage<DBC::Structures::AchievementEntry> sAchievementStore(DBC::Structures::achievement_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::AchievementCriteriaEntry> sAchievementCriteriaStore(DBC::Structures::achievement_criteria_format);
#endif

SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaGroupEntry> sAreaGroupStore(DBC::Structures::area_group_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTableEntry> sAreaStore(DBC::Structures::area_table_entry_format);
static WMOAreaInfoByTripple sWMOAreaInfoByTripple;
SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTriggerEntry> sAreaTriggerStore(DBC::Structures::area_trigger_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::AuctionHouseEntry> sAuctionHouseStore(DBC::Structures::auction_house_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::BankBagSlotPrices> sBankBagSlotPricesStore(DBC::Structures::bank_bag_slot_prices_format);
//SERVER_DECL DBC::DBCStorage<DBC::Structures::StableSlotPrices> sStableSlotPricesStore(DBC::Structures::stable_slot_prices_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::BarberShopStyleEntry> sBarberShopStyleStore(DBC::Structures::barber_shop_style_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrClassesEntry> sChrClassesStore(DBC::Structures::chr_classes_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrRacesEntry> sChrRacesStore(DBC::Structures::chr_races_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::CharTitlesEntry> sCharTitlesStore(DBC::Structures::char_titles_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ChatChannelsEntry> sChatChannelsStore(DBC::Structures::chat_channels_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtCombatRatingsEntry> sGtCombatRatingsStore(DBC::Structures::gt_combat_ratings_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureDisplayInfoEntry> sCreatureDisplayInfoStore(DBC::Structures::creature_display_info_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore(DBC::Structures::creature_display_info_extra_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureSpellDataEntry> sCreatureSpellDataStore(DBC::Structures::creature_spell_data_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureFamilyEntry> sCreatureFamilyStore(DBC::Structures::creature_family_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::CurrencyTypesEntry> sCurrencyTypesStore(DBC::Structures::currency_types_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiNodesEntry> sTaxiNodesStore(DBC::Structures::taxi_nodes_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathEntry> sTaxiPathStore(DBC::Structures::taxi_path_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathNodeEntry> sTaxiPathNodeStore(DBC::Structures::taxi_path_node_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityCostsEntry> sDurabilityCostsStore(DBC::Structures::durability_costs_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityQualityEntry> sDurabilityQualityStore(DBC::Structures::durability_quality_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionTemplateEntry> sFactionTemplateStore(DBC::Structures::faction_template_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionEntry> sFactionStore(DBC::Structures::faction_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore(DBC::Structures::game_object_display_info_format);

SERVER_DECL DBC::DBCStorage<DBC::Structures::GuildPerkSpellsEntry> sGuildPerkSpellsStore(DBC::Structures::guild_perk_spells_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::EmotesEntry> sEmotesStore(DBC::Structures::emotes_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::EmotesTextEntry> sEmotesTextStore(DBC::Structures::emotes_text_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellItemEnchantmentEntry> sSpellItemEnchantmentStore(DBC::Structures::spell_item_enchantment_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GemPropertiesEntry> sGemPropertiesStore(DBC::Structures::gem_properties_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GlyphPropertiesEntry> sGlyphPropertiesStore(DBC::Structures::glyph_properties_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GlyphSlotEntry> sGlyphSlotStore(DBC::Structures::glyph_slot_format);
// db2 SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemExtendedCostEntry> sItemExtendedCostStore(DBC::Structures::item_extended_cost_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemLimitCategoryEntry> sItemLimitCategoryStore(DBC::Structures::item_limit_category_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemRandomSuffixEntry> sItemRandomSuffixStore(DBC::Structures::item_random_suffix_format);
// db2 SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemEntry> sItemStore(DBC::Structures::item_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemSetEntry> sItemSetStore(DBC::Structures::item_set_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::LockEntry> sLockStore(DBC::Structures::lock_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::MapEntry> sMapStore(DBC::Structures::map_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::HolidaysEntry> sHolidaysStore(DBC::Structures::holidays_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemRandomPropertiesEntry> sItemRandomPropertiesStore(DBC::Structures::item_random_properties_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ScalingStatDistributionEntry> sScalingStatDistributionStore(DBC::Structures::scaling_stat_distribution_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ScalingStatValuesEntry> sScalingStatValuesStore(DBC::Structures::scaling_stat_values_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SkillLineAbilityEntry> sSkillLineAbilityStore(DBC::Structures::skill_line_ability_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SkillLineEntry> sSkillLineStore(DBC::Structures::skill_line_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCastTimesEntry> sSpellCastTimesStore(DBC::Structures::spell_cast_times_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellDifficultyEntry> sSpellDifficultyStore(DBC::Structures::spell_difficulty_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellDurationEntry> sSpellDurationStore(DBC::Structures::spell_duration_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellAuraOptionsEntry> sSpellAuraOptionsStore(DBC::Structures::spell_aura_options_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellAuraRestrictionsEntry> sSpellAuraRestrictionsStore(DBC::Structures::spell_aura_restrictions_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCastingRequirementsEntry> sSpellCastingRequirementsStore(DBC::Structures::spell_casting_requirements_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCategoriesEntry> sSpellCategoriesStore(DBC::Structures::spell_categories_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellClassOptionsEntry> sSpellClassOptionsStore(DBC::Structures::spell_class_options_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCooldownsEntry> sSpellCooldownsStore(DBC::Structures::spell_cooldowns_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellEquippedItemsEntry> sSpellEquippedItemsStore(DBC::Structures::spell_equipped_items_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellInterruptsEntry> sSpellInterruptsStore(DBC::Structures::spell_interrupts_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellLevelsEntry> sSpellLevelsStore(DBC::Structures::spell_levels_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellPowerEntry> sSpellPowerStore(DBC::Structures::spell_power_format);

SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellScalingEntry> sSpellScalingStore(DBC::Structures::spell_scaling_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellReagentsEntry> sSpellReagentsStore(DBC::Structures::spell_reagents_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellShapeshiftEntry> sSpellShapeshiftStore(DBC::Structures::spell_shapeshift_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellTargetRestrictionsEntry> sSpellTargetRestrictionsStore(DBC::Structures::spell_target_restrictions_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellTotemsEntry> sSpellTotemsStore(DBC::Structures::spell_totems_format);

DBC::Structures::SpellEffectMap sSpellEffectMap;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellEffectEntry> sSpellEffectStore(DBC::Structures::spell_effect_format);
DBC::Structures::SpellCategoryStore sSpellCategoryStore;

SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellEntry> sSpellStore(DBC::Structures::spell_entry_format);

///\todo remove the old spell loader
SERVER_DECL DBCStorage<OLD_SpellEntry> dbcSpell;

SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRadiusEntry> sSpellRadiusStore(DBC::Structures::spell_radius_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRangeEntry> sSpellRangeStore(DBC::Structures::spell_range_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRuneCostEntry> sSpellRuneCostStore(DBC::Structures::spell_rune_cost_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentEntry> sTalentStore(DBC::Structures::talent_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentTabEntry> sTalentTabStore(DBC::Structures::talent_tab_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentTreePrimarySpells> sTalentTreePrimarySpellsStore(DBC::Structures::talent_tree_primary_spells_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::WorldMapOverlayEntry> sWorldMapOverlayStore(DBC::Structures::world_map_overlay_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtBarberShopCostBaseEntry> sBarberShopCostBaseStore(DBC::Structures::gt_barber_shop_cost_format);
//SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenHPEntry> sGtOCTRegenHPStore(DBC::Structures::gt_oct_regen_hp_format);
//SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenHPPerSptEntry> sGtRegenHPPerSptStore(DBC::Structures::gt_regen_hp_per_spt_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenMPEntry> sGtOCTRegenMPStore(DBC::Structures::gt_oct_regen_mp_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenMPPerSptEntry> sGtRegenMPPerSptStore(DBC::Structures::gt_regen_mp_per_spt_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritEntry> sGtChanceToMeleeCritStore(DBC::Structures::gt_chance_to_melee_crit_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore(DBC::Structures::gt_chance_to_melee_crit_base_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritEntry> sGtChanceToSpellCritStore(DBC::Structures::gt_chance_to_spell_crit_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore(DBC::Structures::gt_chance_to_spell_crit_base_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellShapeshiftFormEntry> sSpellShapeshiftFormStore(DBC::Structures::spell_shapeshift_form_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::QuestSortEntry> sQuestSortStore(DBC::Structures::quest_sort_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::QuestXP> sQuestXPStore(DBC::Structures::quest_xp_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::MailTemplateEntry> sMailTemplateStore(DBC::Structures::mail_template_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::WMOAreaTableEntry> sWMOAreaTableStore(DBC::Structures::wmo_area_table_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SummonPropertiesEntry> sSummonPropertiesStore(DBC::Structures::summon_properties_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::NameGenEntry> sNameGenStore(DBC::Structures::name_gen_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::PhaseEntry> sPhaseStore(DBC::Structures::phase_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::LFGDungeonEntry> sLFGDungeonStore(DBC::Structures::lfg_dungeon_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::LiquidTypeEntry> sLiquidTypeStore(DBC::Structures::liquid_type_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleEntry> sVehicleStore(DBC::Structures::vehicle_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleSeatEntry> sVehicleSeatStore(DBC::Structures::vehicle_seat_format);


template<class T>
bool loader_stub(const char* filename, const char* format, bool ind, T & l, bool loadstrs)
{
    Log.Notice("DBC", "Loading %s.", filename);
    return l.Load(filename, format, ind, loadstrs);
}

#define LOAD_DBC(filename, format, ind, stor, strings) if (!loader_stub(filename, format, ind, stor, strings)) { return false; }

bool LoadDBCs()
{
    uint32 available_dbc_locales = 0xFFFFFFFF;
    DBC::StoreProblemList bad_dbc_files;
    std::string dbc_path = "./DBC/";


    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWorldMapOverlayStore, dbc_path, "WorldMapOverlay.dbc");
#ifdef ENABLE_ACHIEVEMENTS
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAchievementCriteriaStore, dbc_path, "Achievement_Criteria.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAchievementStore, dbc_path, "Achievement.dbc");
#endif
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCharTitlesStore, dbc_path, "CharTitles.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCurrencyTypesStore, dbc_path, "CurrencyTypes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sBarberShopStyleStore, dbc_path, "BarberShopStyle.dbc");
    //DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemStore, dbc_path, "Item.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemSetStore, dbc_path, "ItemSet.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLockStore, dbc_path, "Lock.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sEmotesStore, dbc_path, "Emotes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sEmotesTextStore, dbc_path, "EmotesText.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSkillLineAbilityStore, dbc_path, "SkillLineAbility.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellItemEnchantmentStore, dbc_path, "SpellItemEnchantment.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGemPropertiesStore, dbc_path, "GemProperties.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGlyphPropertiesStore, dbc_path, "GlyphProperties.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGlyphSlotStore, dbc_path, "GlyphSlot.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSkillLineStore, dbc_path, "SkillLine.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellStore, dbc_path, "Spell.dbc");


    //DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemExtendedCostStore, dbc_path, "ItemExtendedCost.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentStore, dbc_path, "Talent.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentTabStore, dbc_path, "TalentTab.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentTreePrimarySpellsStore, dbc_path, "TalentTreePrimarySpells.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellCastTimesStore, dbc_path, "SpellCastTimes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellDifficultyStore, dbc_path, "SpellDifficulty.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRadiusStore, dbc_path, "SpellRadius.dbc");     ///\todo handle max and level radius
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRangeStore, dbc_path, "SpellRange.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRuneCostStore, dbc_path, "SpellRuneCost.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellDurationStore, dbc_path, "SpellDuration.dbc");

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellAuraOptionsStore, dbc_path, "SpellAuraOptions.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellAuraRestrictionsStore, dbc_path, "SpellAuraRestrictions.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellCastingRequirementsStore, dbc_path, "SpellCastingRequirements.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellCategoriesStore, dbc_path, "SpellCategories.dbc");
    //todo danko DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellClassOptionsStore, dbc_path, "SpellClassOptions.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellCooldownsStore, dbc_path, "SpellCooldowns.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellEquippedItemsStore, dbc_path, "SpellEquippedItems.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellInterruptsStore, dbc_path, "SpellInterrupts.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellLevelsStore, dbc_path, "SpellLevels.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellPowerStore, dbc_path, "SpellPower.dbc");

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellScalingStore, dbc_path, "SpellScaling.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellReagentsStore, dbc_path, "SpellReagents.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellShapeshiftStore, dbc_path, "SpellShapeshift.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellTargetRestrictionsStore, dbc_path, "SpellTargetRestrictions.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellTotemsStore, dbc_path, "SpellTotems.dbc");

    //todo danko DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellEffectStore, dbc_path, "SpellEffect.dbc");

    for (uint32 i = 1; i < sSpellStore.GetNumRows(); ++i)
    {
        if (DBC::Structures::SpellEntry const* spell = sSpellStore.LookupEntry(i))
        {
            if (DBC::Structures::SpellCategoriesEntry const* category = spell->GetSpellCategories())
                if (uint32 cat = category->Category)
                    sSpellCategoryStore[cat].insert(i);
        }
    }

    for (uint32 i = 1; i < sSpellEffectStore.GetNumRows(); ++i)
    {
        if (DBC::Structures::SpellEffectEntry const* spellEffect = sSpellEffectStore.LookupEntry(i))
        {
            switch (spellEffect->EffectApplyAuraName)
            {
                case SPELL_AURA_MOD_INCREASE_ENERGY:
                case SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT:
                case SPELL_AURA_PERIODIC_MANA_LEECH:
                case SPELL_AURA_PERIODIC_ENERGIZE:
                case SPELL_AURA_POWER_BURN:
                    ARCEMU_ASSERT(spellEffect->EffectMiscValue >= 0 && spellEffect->EffectMiscValue < MAX_POWERS);
                    break;
            }

            sSpellEffectMap[spellEffect->EffectSpellId].effects[spellEffect->EffectIndex] = spellEffect;
        }
    }

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellShapeshiftFormStore, dbc_path, "SpellShapeshiftForm.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemRandomPropertiesStore, dbc_path, "ItemRandomProperties.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAreaGroupStore, dbc_path, "AreaGroup.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAreaStore, dbc_path, "AreaTable.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sFactionTemplateStore, dbc_path, "FactionTemplate.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sFactionStore, dbc_path, "Faction.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGameObjectDisplayInfoStore, dbc_path, "GameObjectDisplayInfo.dbc");

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGuildPerkSpellsStore, dbc_path, "GuildPerkSpells.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiNodesStore, dbc_path, "TaxiNodes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiPathStore, dbc_path, "TaxiPath.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiPathNodeStore, dbc_path, "TaxiPathNode.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureSpellDataStore, dbc_path, "CreatureSpellData.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureFamilyStore, dbc_path, "CreatureFamily.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrRacesStore, dbc_path, "ChrRaces.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrClassesStore, dbc_path, "ChrClasses.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMapStore, dbc_path, "Map.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sHolidaysStore, dbc_path, "Holidays.dbc");       //loaded but not used
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAuctionHouseStore, dbc_path, "AuctionHouse.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemRandomSuffixStore, dbc_path, "ItemRandomSuffix.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtCombatRatingsStore, dbc_path, "gtCombatRatings.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChatChannelsStore, dbc_path, "ChatChannels.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureDisplayInfoStore, dbc_path, "CreatureDisplayInfo.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureDisplayInfoExtraStore, dbc_path, "CreatureDisplayInfoExtra.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sDurabilityQualityStore, dbc_path, "DurabilityQuality.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sDurabilityCostsStore, dbc_path, "DurabilityCosts.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sBankBagSlotPricesStore, dbc_path, "BankBagSlotPrices.dbc");
    //DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sStableSlotPricesStore, dbc_path, "StableSlotPrices.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sBarberShopCostBaseStore, dbc_path, "gtBarberShopCostBase.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritStore, dbc_path, "gtChanceToMeleeCrit.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritBaseStore, dbc_path, "gtChanceToMeleeCritBase.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritStore, dbc_path, "gtChanceToSpellCrit.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritBaseStore, dbc_path, "gtChanceToSpellCritBase.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtRegenMPPerSptStore, dbc_path, "gtRegenMPPerSpt.dbc");     //loaded but not used
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTRegenMPStore, dbc_path, "gtOCTRegenMP.dbc");
    //DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtRegenHPPerSptStore, dbc_path, "gtRegenHPPerSpt.dbc");
    //DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTRegenHPStore, dbc_path, "gtOCTRegenHP.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAreaTriggerStore, dbc_path, "AreaTrigger.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sScalingStatDistributionStore, dbc_path, "ScalingStatDistribution.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sScalingStatValuesStore, dbc_path, "ScalingStatValues.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemLimitCategoryStore, dbc_path, "ItemLimitCategory.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sQuestSortStore, dbc_path, "QuestSort.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sQuestXPStore, dbc_path, "QuestXP.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMailTemplateStore, dbc_path, "MailTemplate.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWMOAreaTableStore, dbc_path, "WMOAreaTable.dbc");
    for (uint32 i = 0; i < sWMOAreaTableStore.GetNumRows(); ++i)
    {
        if (DBC::Structures::WMOAreaTableEntry const* entry = sWMOAreaTableStore.LookupEntry(i))
        {
            sWMOAreaInfoByTripple.insert(WMOAreaInfoByTripple::value_type(WMOAreaTableTripple(entry->rootId, entry->adtId, entry->groupId), entry));
        }
    }
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSummonPropertiesStore, dbc_path, "SummonProperties.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sNameGenStore, dbc_path, "NameGen.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sPhaseStore, dbc_path, "Phase.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLFGDungeonStore, dbc_path, "LFGDungeons.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLiquidTypeStore, dbc_path, "LiquidType.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sVehicleStore, dbc_path, "Vehicle.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sVehicleSeatStore, dbc_path, "VehicleSeat.dbc");

    MapManagement::AreaManagement::AreaStorage::Initialise(&sAreaStore);
    auto area_map_collection = MapManagement::AreaManagement::AreaStorage::GetMapCollection();
    for (uint32 i = 0; i < sMapStore.GetNumRows(); ++i)
    {
        auto map_object = sMapStore.LookupEntry(i);
        if (map_object == nullptr)
            continue;

        area_map_collection->insert(std::pair<uint32, uint32>(map_object->id, map_object->linked_zone));
    }
    
    return true;
}

DBC::Structures::WMOAreaTableEntry const* GetWMOAreaTableEntryByTriple(int32 root_id, int32 adt_id, int32 group_id)
{
    auto iter = sWMOAreaInfoByTripple.find(WMOAreaTableTripple(root_id, adt_id, group_id));
    if (iter == sWMOAreaInfoByTripple.end())
        return nullptr;
    return iter->second;
}

DBC::Structures::SpellEffectEntry const* GetSpellEffectEntry(uint32 spellId, SpellEffectIndex effect)
{
    DBC::Structures::SpellEffectMap::const_iterator itr = sSpellEffectMap.find(spellId);
    if (itr == sSpellEffectMap.end())
        return NULL;

    return itr->second.effects[effect];
}
