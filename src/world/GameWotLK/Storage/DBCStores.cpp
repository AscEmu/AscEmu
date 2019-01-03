/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WorldConf.h"
#include "Server/World.h"

#if VERSION_STRING == WotLK

#include "StdAfx.h"
#include "../world/Storage/DBC/DBCGlobals.hpp"
#include "Map/Area/AreaStorage.hpp"

typedef std::map<WMOAreaTableTripple, DBC::Structures::WMOAreaTableEntry const*> WMOAreaInfoByTripple;

struct NameGenData
{
    std::string name;
    uint32_t type;
};

std::vector<NameGenData> _namegenData[3];

SERVER_DECL DBC::DBCStorage<DBC::Structures::AchievementEntry> sAchievementStore(DBC::Structures::achievement_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::AchievementCriteriaEntry> sAchievementCriteriaStore(DBC::Structures::achievement_criteria_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaGroupEntry> sAreaGroupStore(DBC::Structures::area_group_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTableEntry> sAreaStore(DBC::Structures::area_table_entry_format);
static WMOAreaInfoByTripple sWMOAreaInfoByTripple;
SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTriggerEntry> sAreaTriggerStore(DBC::Structures::area_trigger_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::AuctionHouseEntry> sAuctionHouseStore(DBC::Structures::auction_house_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::BankBagSlotPrices> sBankBagSlotPricesStore(DBC::Structures::bank_bag_slot_prices_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::StableSlotPrices> sStableSlotPricesStore(DBC::Structures::stable_slot_prices_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::BarberShopStyleEntry> sBarberShopStyleStore(DBC::Structures::barber_shop_style_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::CharStartOutfitEntry> sCharStartOutfitStore(DBC::Structures::char_start_outfit_format);
std::map<uint32_t, DBC::Structures::CharStartOutfitEntry const*> sCharStartOutfitMap;
SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrClassesEntry> sChrClassesStore(DBC::Structures::chr_classes_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrRacesEntry> sChrRacesStore(DBC::Structures::chr_races_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::CharTitlesEntry> sCharTitlesStore(DBC::Structures::char_titles_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ChatChannelsEntry> sChatChannelsStore(DBC::Structures::chat_channels_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtCombatRatingsEntry> sGtCombatRatingsStore(DBC::Structures::gt_combat_ratings_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureDisplayInfoEntry> sCreatureDisplayInfoStore(DBC::Structures::creature_display_info_format);
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
SERVER_DECL DBC::DBCStorage<DBC::Structures::EmotesTextEntry> sEmotesTextStore(DBC::Structures::emotes_text_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellItemEnchantmentEntry> sSpellItemEnchantmentStore(DBC::Structures::spell_item_enchantment_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GemPropertiesEntry> sGemPropertiesStore(DBC::Structures::gem_properties_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GlyphPropertiesEntry> sGlyphPropertiesStore(DBC::Structures::glyph_properties_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GlyphSlotEntry> sGlyphSlotStore(DBC::Structures::glyph_slot_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemExtendedCostEntry> sItemExtendedCostStore(DBC::Structures::item_extended_cost_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemLimitCategoryEntry> sItemLimitCategoryStore(DBC::Structures::item_limit_category_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemRandomSuffixEntry> sItemRandomSuffixStore(DBC::Structures::item_random_suffix_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemEntry> sItemStore(DBC::Structures::item_entry_format);
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
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellEntry> sSpellStore(DBC::Structures::spell_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRadiusEntry> sSpellRadiusStore(DBC::Structures::spell_radius_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRangeEntry> sSpellRangeStore(DBC::Structures::spell_range_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRuneCostEntry> sSpellRuneCostStore(DBC::Structures::spell_rune_cost_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentEntry> sTalentStore(DBC::Structures::talent_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentTabEntry> sTalentTabStore(DBC::Structures::talent_tab_format);
static uint32_t InspectTalentTabPages[12][3];
SERVER_DECL DBC::DBCStorage<DBC::Structures::WorldMapOverlayEntry> sWorldMapOverlayStore(DBC::Structures::world_map_overlay_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtBarberShopCostBaseEntry> sBarberShopCostBaseStore(DBC::Structures::gt_barber_shop_cost_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenHPEntry> sGtOCTRegenHPStore(DBC::Structures::gt_oct_regen_hp_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenHPPerSptEntry> sGtRegenHPPerSptStore(DBC::Structures::gt_regen_hp_per_spt_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenMPEntry> sGtOCTRegenMPStore(DBC::Structures::gt_oct_regen_mp_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenMPPerSptEntry> sGtRegenMPPerSptStore(DBC::Structures::gt_regen_mp_per_spt_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritEntry> sGtChanceToMeleeCritStore(DBC::Structures::gt_chance_to_melee_crit_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore(DBC::Structures::gt_chance_to_melee_crit_base_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritEntry> sGtChanceToSpellCritStore(DBC::Structures::gt_chance_to_spell_crit_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore(DBC::Structures::gt_chance_to_spell_crit_base_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellShapeshiftFormEntry> sSpellShapeshiftFormStore(DBC::Structures::spell_shapeshift_form_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::QuestXP> sQuestXPStore(DBC::Structures::quest_xp_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::MailTemplateEntry> sMailTemplateStore(DBC::Structures::mail_template_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::WMOAreaTableEntry> sWMOAreaTableStore(DBC::Structures::wmo_area_table_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::SummonPropertiesEntry> sSummonPropertiesStore(DBC::Structures::summon_properties_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::NameGenEntry> sNameGenStore(DBC::Structures::name_gen_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::LFGDungeonEntry> sLFGDungeonStore(DBC::Structures::lfg_dungeon_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::LiquidTypeEntry> sLiquidTypeStore(DBC::Structures::liquid_type_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleEntry> sVehicleStore(DBC::Structures::vehicle_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleSeatEntry> sVehicleSeatStore(DBC::Structures::vehicle_seat_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::WorldMapAreaEntry> sWorldMapAreaStore(DBC::Structures::world_map_area_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::TotemCategoryEntry> sTotemCategoryStore(DBC::Structures::totem_category_format);

bool LoadDBCs()
{
    uint32_t available_dbc_locales = 0xFFFFFFFF;
    DBC::StoreProblemList bad_dbc_files;
    std::string dbc_path = sWorld.settings.server.dataDir + "dbc/";

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWorldMapOverlayStore, dbc_path, "WorldMapOverlay.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAchievementCriteriaStore, dbc_path, "Achievement_Criteria.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAchievementStore, dbc_path, "Achievement.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCharTitlesStore, dbc_path, "CharTitles.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCurrencyTypesStore, dbc_path, "CurrencyTypes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sBarberShopStyleStore, dbc_path, "BarberShopStyle.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCharStartOutfitStore, dbc_path, "CharStartOutfit.dbc");
    for (uint32_t i = 0; i < sCharStartOutfitStore.GetNumRows(); ++i)
        if (DBC::Structures::CharStartOutfitEntry const* outfit = sCharStartOutfitStore.LookupEntry(i))
            sCharStartOutfitMap[outfit->Race | (outfit->Class << 8) | (outfit->Gender << 16)] = outfit;

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemStore, dbc_path, "Item.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemSetStore, dbc_path, "ItemSet.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLockStore, dbc_path, "Lock.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sEmotesTextStore, dbc_path, "EmotesText.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSkillLineAbilityStore, dbc_path, "SkillLineAbility.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellItemEnchantmentStore, dbc_path, "SpellItemEnchantment.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGemPropertiesStore, dbc_path, "GemProperties.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGlyphPropertiesStore, dbc_path, "GlyphProperties.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGlyphSlotStore, dbc_path, "GlyphSlot.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSkillLineStore, dbc_path, "SkillLine.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellStore, dbc_path, "Spell.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemExtendedCostStore, dbc_path, "ItemExtendedCost.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentStore, dbc_path, "Talent.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentTabStore, dbc_path, "TalentTab.dbc");
    {
        std::map< uint32_t, uint32_t > InspectTalentTabPos;
        std::map< uint32_t, uint32_t > InspectTalentTabSize;
        std::map< uint32_t, uint32_t > InspectTalentTabBit;

        uint32_t talent_max_rank;
        uint32_t talent_pos;
        uint32_t talent_class;

        for (uint32_t i = 0; i < sTalentStore.GetNumRows(); ++i)
        {
            auto talent_info = sTalentStore.LookupEntry(i);
            if (talent_info == nullptr)
                continue;

            // Don't add invalid talents or Hunter Pet talents (trees 409, 410 and 411) to the inspect table
            if (talent_info->TalentTree == 409 || talent_info->TalentTree == 410 || talent_info->TalentTree == 411)
                continue;

            auto talent_tab = sTalentTabStore.LookupEntry(talent_info->TalentTree);
            if (talent_tab == nullptr)
                continue;

            talent_max_rank = 0;
            for (uint32_t j = 5; j > 0; --j)
            {
                if (talent_info->RankID[j - 1])
                {
                    talent_max_rank = j;
                    break;
                }
            }

            InspectTalentTabBit[(talent_info->Row << 24) + (talent_info->Col << 16) + talent_info->TalentID] = talent_max_rank;
            InspectTalentTabSize[talent_info->TalentTree] += talent_max_rank;
        }

        for (uint32_t i = 0; i < sTalentTabStore.GetNumRows(); ++i)
        {
            auto talent_tab = sTalentTabStore.LookupEntry(i);
            if (talent_tab == nullptr)
                continue;

            if (talent_tab->ClassMask == 0)
                continue;

            talent_pos = 0;

            for (talent_class = 0; talent_class < 12; ++talent_class)
            {
                if (talent_tab->ClassMask & (1 << talent_class))
                    break;
            }

            if (talent_class > 0 && talent_class < 12)
                InspectTalentTabPages[talent_class][talent_tab->TabPage] = talent_tab->TalentTabID;

            for (std::map<uint32_t, uint32_t>::iterator itr = InspectTalentTabBit.begin(); itr != InspectTalentTabBit.end(); ++itr)
            {
                uint32_t talent_id = itr->first & 0xFFFF;
                auto talent_info = sTalentStore.LookupEntry(talent_id);
                if (talent_info == nullptr)
                    continue;

                if (talent_info->TalentTree != talent_tab->TalentTabID)
                    continue;

                InspectTalentTabPos[talent_id] = talent_pos;
                talent_pos += itr->second;
            }
        }
    }

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellCastTimesStore, dbc_path, "SpellCastTimes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellDifficultyStore, dbc_path, "SpellDifficulty.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRadiusStore, dbc_path, "SpellRadius.dbc");     ///\todo handle max and level radius
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRangeStore, dbc_path, "SpellRange.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRuneCostStore, dbc_path, "SpellRuneCost.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellDurationStore, dbc_path, "SpellDuration.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellShapeshiftFormStore, dbc_path, "SpellShapeshiftForm.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemRandomPropertiesStore, dbc_path, "ItemRandomProperties.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAreaGroupStore, dbc_path, "AreaGroup.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAreaStore, dbc_path, "AreaTable.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sFactionTemplateStore, dbc_path, "FactionTemplate.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sFactionStore, dbc_path, "Faction.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGameObjectDisplayInfoStore, dbc_path, "GameObjectDisplayInfo.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiNodesStore, dbc_path, "TaxiNodes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiPathStore, dbc_path, "TaxiPath.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiPathNodeStore, dbc_path, "TaxiPathNode.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureDisplayInfoStore, dbc_path, "CreatureDisplayInfo.dbc");
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
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sDurabilityQualityStore, dbc_path, "DurabilityQuality.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sDurabilityCostsStore, dbc_path, "DurabilityCosts.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sBankBagSlotPricesStore, dbc_path, "BankBagSlotPrices.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sStableSlotPricesStore, dbc_path, "StableSlotPrices.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sBarberShopCostBaseStore, dbc_path, "gtBarberShopCostBase.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritStore, dbc_path, "gtChanceToMeleeCrit.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritBaseStore, dbc_path, "gtChanceToMeleeCritBase.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritStore, dbc_path, "gtChanceToSpellCrit.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritBaseStore, dbc_path, "gtChanceToSpellCritBase.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtRegenMPPerSptStore, dbc_path, "gtRegenMPPerSpt.dbc");     //loaded but not used
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTRegenMPStore, dbc_path, "gtOCTRegenMP.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtRegenHPPerSptStore, dbc_path, "gtRegenHPPerSpt.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTRegenHPStore, dbc_path, "gtOCTRegenHP.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAreaTriggerStore, dbc_path, "AreaTrigger.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sScalingStatDistributionStore, dbc_path, "ScalingStatDistribution.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sScalingStatValuesStore, dbc_path, "ScalingStatValues.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemLimitCategoryStore, dbc_path, "ItemLimitCategory.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sQuestXPStore, dbc_path, "QuestXP.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMailTemplateStore, dbc_path, "MailTemplate.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWMOAreaTableStore, dbc_path, "WMOAreaTable.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSummonPropertiesStore, dbc_path, "SummonProperties.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sNameGenStore, dbc_path, "NameGen.dbc");
    for (uint32_t i = 0; i < sNameGenStore.GetNumRows(); ++i)
    {
        auto name_gen_entry = sNameGenStore.LookupEntry(i);
        if (name_gen_entry == nullptr)
            continue;

        NameGenData nameGenData;
        nameGenData.name = std::string(name_gen_entry->Name);
        nameGenData.type = name_gen_entry->type;
        _namegenData[nameGenData.type].push_back(nameGenData);
    }

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLFGDungeonStore, dbc_path, "LFGDungeons.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLiquidTypeStore, dbc_path, "LiquidType.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sVehicleStore, dbc_path, "Vehicle.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sVehicleSeatStore, dbc_path, "VehicleSeat.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWorldMapAreaStore, dbc_path, "WorldMapArea.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTotemCategoryStore, dbc_path, "TotemCategory.dbc");

    MapManagement::AreaManagement::AreaStorage::Initialise(&sAreaStore);
    auto area_map_collection = MapManagement::AreaManagement::AreaStorage::GetMapCollection();
    for (uint32_t i = 0; i < sMapStore.GetNumRows(); ++i)
    {
        auto map_object = sMapStore.LookupEntry(i);
        if (map_object == nullptr)
            continue;

        area_map_collection->insert(std::pair<uint32_t, uint32_t>(map_object->id, map_object->linked_zone));
    }
    auto wmo_row_count = sWMOAreaTableStore.GetNumRows();
    for (uint32_t i = 0; i < wmo_row_count; ++i)
    {
        if (auto entry = sWMOAreaTableStore.LookupEntry(i))
        {
            sWMOAreaInfoByTripple.insert(WMOAreaInfoByTripple::value_type(WMOAreaTableTripple(entry->rootId, entry->adtId, entry->groupId), entry));
        }
    }

    return true;
}

DBC::Structures::CharStartOutfitEntry const* getStartOutfitByRaceClass(uint8_t race, uint8_t class_, uint8_t gender)
{
    const auto itr = sCharStartOutfitMap.find(race | (class_ << 8) | (gender << 16));
    if (itr != sCharStartOutfitMap.end())
        return itr->second;

    return nullptr;
}

DBC::Structures::WMOAreaTableEntry const* GetWMOAreaTableEntryByTriple(int32 root_id, int32 adt_id, int32 group_id)
{
    auto iter = sWMOAreaInfoByTripple.find(WMOAreaTableTripple(root_id, adt_id, group_id));
    if (iter == sWMOAreaInfoByTripple.end())
        return nullptr;
    return iter->second;
}

std::string generateName(uint32_t type)
{
    if (_namegenData[type].size() == 0)
        return "ERR";

    uint32_t ent = Util::getRandomUInt((uint32_t)_namegenData[type].size() - 1);
    return _namegenData[type].at(ent).name;
}

uint32_t const* getTalentTabPages(uint8_t playerClass)
{
    return InspectTalentTabPages[playerClass];
}

#endif
