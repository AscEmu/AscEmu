/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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

typedef std::map<WMOAreaTableTripple, WMOAreaTableEntry const*> WMOAreaInfoByTripple;

#ifdef ENABLE_ACHIEVEMENTS
SERVER_DECL DBCStorage<AchievementEntry> dbcAchievementStore;
SERVER_DECL DBCStorage<AchievementCriteriaEntry> dbcAchievementCriteriaStore;
SERVER_DECL DBCStorage<AchievementCategoryEntry> dbcAchievementCategoryStore;
#endif
SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaGroupEntry> sAreaGroupStore(DBC::Structures::area_group_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTableEntry> sAreaStore(DBC::Structures::area_table_entry_format);
static WMOAreaInfoByTripple sWMOAreaInfoByTripple;
SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTriggerEntry> sAreaTriggerStore(DBC::Structures::area_trigger_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::AuctionHouseEntry> sAuctionHouseStore(DBC::Structures::auction_house_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::BankBagSlotPrices> sBankBagSlotPricesStore(DBC::Structures::bank_bag_slot_prices_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::StableSlotPrices> sStableSlotPricesStore(DBC::Structures::stable_slot_prices_format);

SERVER_DECL DBC::DBCStorage<DBC::Structures::BarberShopStyleEntry> sBarberShopStyleStore(DBC::Structures::barber_shop_style_entry_format);
//SERVER_DECL DBCStorage<BattlemasterListEntry> dbcBattlemasterListStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrClassesEntry> sChrClassesStore(DBC::Structures::chr_classes_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrRacesEntry> sChrRacesStore(DBC::Structures::chr_races_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::CharTitlesEntry> sCharTitlesStore(DBC::Structures::char_titles_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ChatChannelsEntry> sChatChannelsStore(DBC::Structures::chat_channels_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtCombatRatingsEntry> sGtCombatRatingsStore(DBC::Structures::gt_combat_ratings_format);
SERVER_DECL DBCStorage<CreatureSpellDataEntry> dbcCreatureSpellData;
SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureFamilyEntry> sCreatureFamilyStore(DBC::Structures::creature_family_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::CurrencyTypesEntry> sCurrencyTypesStore(DBC::Structures::currency_types_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiNodesEntry> sTaxiNodesStore(DBC::Structures::taxi_nodes_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathEntry> sTaxiPathStore(DBC::Structures::taxi_path_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathNodeEntry> sTaxiPathNodeStore(DBC::Structures::taxi_path_node_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityCostsEntry> sDurabilityCostsStore(DBC::Structures::durability_costs_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityQualityEntry> sDurabilityQualityStore(DBC::Structures::durability_quality_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionTemplateEntry> sFactionTemplateStore(DBC::Structures::faction_template_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionEntry> sFactionStore(DBC::Structures::faction_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::EmotesTextEntry> sEmotesTextStore(DBC::Structures::emotes_text_format);
SERVER_DECL DBCStorage<EnchantEntry> dbcEnchant;
SERVER_DECL DBCStorage<GemPropertyEntry> dbcGemProperty;
SERVER_DECL DBCStorage<GlyphPropertyEntry> dbcGlyphProperty;
SERVER_DECL DBCStorage<GlyphSlotEntry> dbcGlyphSlot;
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemExtendedCostEntry> sItemExtendedCostStore(DBC::Structures::item_extended_cost_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemLimitCategoryEntry> sItemLimitCategoryStore(DBC::Structures::item_limit_category_format);
SERVER_DECL DBCStorage<ItemRandomSuffixEntry> dbcItemRandomSuffix;

SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemEntry> sItemStore(DBC::Structures::item_entry_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemSetEntry> sItemSetStore(DBC::Structures::item_set_format);

SERVER_DECL DBC::DBCStorage<DBC::Structures::LockEntry> sLockStore(DBC::Structures::lock_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::MapEntry> sMapStore(DBC::Structures::map_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::HolidaysEntry> sHolidaysStore(DBC::Structures::holidays_format);
SERVER_DECL DBCStorage<RandomProps> dbcRandomProps;
SERVER_DECL DBC::DBCStorage<DBC::Structures::ScalingStatDistributionEntry> sScalingStatDistributionStore(DBC::Structures::scaling_stat_distribution_format);
SERVER_DECL DBCStorage<ScalingStatValuesEntry> dbcScalingStatValues;
SERVER_DECL DBCStorage<skilllinespell> dbcSkillLineSpell;
SERVER_DECL DBCStorage<skilllineentry> dbcSkillLine;
SERVER_DECL DBCStorage<SpellCastTime> dbcSpellCastTime;
SERVER_DECL DBCStorage<SpellDifficultyEntry> dbcSpellDifficultyEntry;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellDurationEntry> sSpellDurationStore(DBC::Structures::spell_duration_format);
SERVER_DECL DBCStorage<SpellEntry> dbcSpell;
SERVER_DECL DBCStorage<SpellRadius> dbcSpellRadius;
SERVER_DECL DBCStorage<SpellRange> dbcSpellRange;
SERVER_DECL DBCStorage<SpellRuneCostEntry> dbcSpellRuneCost;
SERVER_DECL DBCStorage<TalentEntry> dbcTalent;
SERVER_DECL DBCStorage<TalentTabEntry> dbcTalentTab;
SERVER_DECL DBCStorage<WorldMapOverlay> dbcWorldMapOverlayStore;

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
SERVER_DECL DBCStorage<WMOAreaTableEntry> dbcWMOAreaTable;
SERVER_DECL DBCStorage< SummonPropertiesEntry > dbcSummonProperties;
SERVER_DECL DBC::DBCStorage<DBC::Structures::NameGenEntry> sNameGenStore(DBC::Structures::name_gen_format);
SERVER_DECL DBC::DBCStorage<DBC::Structures::LFGDungeonEntry> sLFGDungeonStore(DBC::Structures::lfg_dungeon_entry_format);
SERVER_DECL DBCStorage< VehicleEntry > dbcVehicle;
SERVER_DECL DBCStorage< VehicleSeatEntry > dbcVehicleSeat;

const char* WorldMapOverlayStoreFormat = "nxiiiixxxxxxxxxxx";

const char* skilllinespellFormat = "uuuxxxxuuuuuxx";
const char* EnchantEntrYFormat = "uxuuuuuuuuuuuusxxxxxxxxxxxxxxxxuuuuxxx";
const char* GemPropertyEntryFormat = "uuuuu";
const char* GlyphPropertyEntryFormat = "uuuu";
const char* GlyphSlotEntryFormat = "uuu";
const char* skilllineentrYFormat = "uuulxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char* VehicleEntryfmt = "niffffiiiiiiiifffffffffffffffssssfifiixx";
const char* VehicleSeatEntryfmt = "niiffffffffffiiiiiifffffffiiifffiiiiiiiffiiiiixxxxxxxxxxxx";

// const char* BattlemasterListEntryFormat = "uiiiiiiiiuuuuuiiiiiiiiiiiiiiiiiiuux";
// const char* BattlemasterListEntryFormat = "uiiiiiiiiuuiiiiiiiiiiiiiiiiiuux";
//const char* BattlemasterListEntryFormat = "uiiiiiiiiuuiiiiiiiiiiiiiiiiiuux";


#ifdef ENABLE_ACHIEVEMENTS
const char* AchievementStoreFormat =
"n" // ID
"i" // factionFlag
"i" // mapID
"u" // unknown1
"lxxxxxxxxxxxxxxx" // name
"u" // name_flags
"lxxxxxxxxxxxxxxx" // description
"u" // desc_flags
"i" // categoryId
"i" // points
"u" // orderInCategory
"i" // flags
"u" // flags2
"lxxxxxxxxxxxxxxx" // rewardName
"u" // rewardName_flags
"u" // count
"u" // refAchievement
;

const char* AchievementCategoryStoreFormat =
"n" // ID
"u" // parentCategory
"lxxxxxxxxxxxxxxx" // name
"u" // name_flags
"u" // sortOrder
;

const char* AchievementCriteriaStoreFormat =
"n" // ID
"i" // referredAchievement
"i" // requiredType
"i" // raw.field3
"i" // raw.field4
"i" // raw.additionalRequirement1_type
"i" // raw.additionalRequirement1_value
"i" // raw.additionalRequirement2_type
"i" // raw.additionalRequirement2_value
"lxxxxxxxxxxxxxxx" // name
"u" // name_flags
"i" // completionFlag
"i" // groupFlag
"u" // unk1
"i" // timeLimit
"u" // index
;
#endif

const char* spelldifficultyentryformat = "niiii";

const char* spellentryFormat =
"u" // Id
"u" // Category
"u" // DispelType
"u" // MechanicsType
"u" // Attributes
"u" // AttributesEx
"u" // AttributesExB
"u" // AttributesExC
"u" // AttributesExD
"u" // AttributesExE
"u" // AttributesExF
"u" // AttributesExG
"u" // RequiredShapeShift
"x" // unk 3.2.0
"u" // ShapeshiftExclude
"x" // unk 3.2.0
"u" // Targets
"u" // TargetCreatureType
"u" // RequiresSpellFocus
"u" // FacingCasterFlags
"u" // CasterAuraState
"u" // TargetAuraState
"u" // ExcludeCasterAuraState
"u" // ExcludeTargetAuraState
"u" // casterAuraSpell
"u" // targetAuraSpell
"u" // ExcludeCasterAuraState
"u" // ExcludeTargetAuraState
"u" // CastingTimeIndex
"u" // RecoveryTime
"u" // CategoryRecoveryTime
"u" // InterruptFlags
"u" // AuraInterruptFlags
"u" // ChannelInterruptFlags
"u" // procFlags
"u" // procChance
"u" // procCharges
"u" // maxLevel
"u" // baseLevel
"u" // spellLevel
"u" // DurationIndex
"u" // powerType
"u" // manaCost
"u" // manaCostPerlevel
"u" // manaPerSecond
"u" // manaPerSecondPerLevel
"u" // rangeIndex
"f" // speed
"u" // modalNextSpell
"u" // maxstack
"uu" // Totem[2]
"uuuuuuuu" // Reagent[8]
"uuuuuuuu" // ReagentCount[8]
"u" // EquippedItemClass
"u" // EquippedItemSubClass
"u" // RequiredItemFlags
"uuu" // Effect[3]
"uuu" // EffectDieSides[3]
"uuu" // EffectRealPointsPerLevel[3]
"uuu" // EffectBasePoints[3]
"uuu" // EffectMechanic[3]
"uuu" // EffectImplicitTargetA[3]
"uuu" // EffectImplicitTargetB[3]
"uuu" // EffectRadiusIndex[3]
"uuu" // EffectApplyAuraName[3]
"uuu" // EffectAmplitude[3]
"uuu" // Effectunknown[3]
"uuu" // EffectChainTarget[3]
"uuu" // EffectSpellGroupRelation[3]
"uuu" // EffectMiscValue[3]
"uuu" // EffectMiscValueB[3]
"uuu" // EffectTriggerSpell[3]
"uuu" // EffectPointsPerComboPoint[3]
"uuu" // EffectUnk0[3]
"uuu" // EffectUnk1[3]
"uuu" // EffectUnk2[3]
"u" // SpellVisual
"u" // field114
"u" // spellIconID
"u" // activeIconID
"u" // spellPriority
"lxxxxxxxxxxxxxxxx" // Name
"lxxxxxxxxxxxxxxxx" // Rank
"lxxxxxxxxxxxxxxxx" // Description
"lxxxxxxxxxxxxxxxx" // BuffDescription
"u" // ManaCostPercentage
"u" // unkflags
"u" // StartRecoveryTime
"u" // StartRecoveryCategory
"u" // MaxTargetLevel
"u" // SpellFamilyName
"uu" // SpellGroupType
"u" // MaxTargets
"u" // Spell_Dmg_Type
"u" // PreventionType
"u" // StanceBarOrder
"fff" // dmg_multiplier[3]
"u" // MinFactionID
"u" // MinReputation
"u" // RequiredAuraVision
"uu" // TotemCategory[2]
"i" // RequiresAreaId
"u" // School
"ux"
"x" //Added in 3.1
"xxx" // unk 3.2.0, float!
"x" // unk 3.2.0
"i"
;

const char* talententryFormat = "uuuuuuuuuxxxxuxxuxxxxxx";
const char* talenttabentryFormat = "uxxxxxxxxxxxxxxxxxxxuuux";
const char* spellcasttimeFormat = "uuxx";
const char* spellradiusFormat = "ufxf";
const char* spellrangeFormat = "uffffxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char* SpellRuneCostFormat = "uuuuu";

const char* randompropsFormat = "uxuuuxxxxxxxxxxxxxxxxxxx";

const char* areatableFormat = "uuuuuxxxuxulxxxxxxxxxxxxxxxxuxxxxxxx";

const char* creaturespelldataFormat = "uuuuuuuuu";

const char* creaturefamilyFormat = "ufufuuuuuxlxxxxxxxxxxxxxxxxx";

const char* itemrandomsuffixformat = "uxxxxxxxxxxxxxxxxxxuuuxxuuuxx";//19, 20, 21, 24, 25, 26
const char* chatchannelformat = "iixssssssssssssssslxxxxxxxxxxxxxxxxxx";

const char* scalingstatvaluesformat = "uuuuuuuuuuuuuuuuuuxxxxxx";

const char* wmoareaformat = "uiiixxxxxuuxxxxxxxxxxxxxxxxx";
const char* summonpropertiesformat = "uuuuuu";


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

    LOAD_DBC("DBC/WorldMapOverlay.dbc", WorldMapOverlayStoreFormat, true, dbcWorldMapOverlayStore, true);
#ifdef ENABLE_ACHIEVEMENTS
    LOAD_DBC("DBC/Achievement_Category.dbc", AchievementCategoryStoreFormat, true, dbcAchievementCategoryStore, true);
    LOAD_DBC("DBC/Achievement_Criteria.dbc", AchievementCriteriaStoreFormat, true, dbcAchievementCriteriaStore, true);
    LOAD_DBC("DBC/Achievement.dbc", AchievementStoreFormat, true, dbcAchievementStore, true);
#endif
    //LOAD_DBC("DBC/BattlemasterList.dbc", BattlemasterListEntryFormat, true, dbcBattlemasterListStore, true);
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCharTitlesStore, dbc_path, "CharTitles.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCurrencyTypesStore, dbc_path, "CurrencyTypes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sBarberShopStyleStore, dbc_path, "BarberShopStyle.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemStore, dbc_path, "Item.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemSetStore, dbc_path, "ItemSet.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLockStore, dbc_path, "Lock.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sEmotesTextStore, dbc_path, "EmotesText.dbc");

    LOAD_DBC("DBC/SkillLineAbility.dbc", skilllinespellFormat, false, dbcSkillLineSpell, false);
    LOAD_DBC("DBC/SpellItemEnchantment.dbc", EnchantEntrYFormat, true, dbcEnchant, true);
    LOAD_DBC("DBC/GemProperties.dbc", GemPropertyEntryFormat, true, dbcGemProperty, false);
    LOAD_DBC("DBC/GlyphProperties.dbc", GlyphPropertyEntryFormat, true, dbcGlyphProperty, false);
    LOAD_DBC("DBC/GlyphSlot.dbc", GlyphSlotEntryFormat, true, dbcGlyphSlot, false);
    LOAD_DBC("DBC/SkillLine.dbc", skilllineentrYFormat, true, dbcSkillLine, true);
    LOAD_DBC("DBC/Spell.dbc", spellentryFormat, true, dbcSpell, true);

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemExtendedCostStore, dbc_path, "ItemExtendedCost.dbc");

    LOAD_DBC("DBC/Talent.dbc", talententryFormat, true, dbcTalent, false);
    LOAD_DBC("DBC/TalentTab.dbc", talenttabentryFormat, true, dbcTalentTab, false);
    LOAD_DBC("DBC/SpellCastTimes.dbc", spellcasttimeFormat, true, dbcSpellCastTime, false);
    LOAD_DBC("DBC/SpellDifficulty.dbc", spelldifficultyentryformat, true, dbcSpellDifficultyEntry, false);
    LOAD_DBC("DBC/SpellRadius.dbc", spellradiusFormat, true, dbcSpellRadius, false);
    LOAD_DBC("DBC/SpellRange.dbc", spellrangeFormat, true, dbcSpellRange, false);
    LOAD_DBC("DBC/SpellRuneCost.dbc", SpellRuneCostFormat, true, dbcSpellRuneCost, false);
    //LOAD_DBC("DBC/SpellDuration.dbc", spelldurationFormat, true, dbcSpellDuration, false);
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellDurationStore, dbc_path, "SpellDuration.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellShapeshiftFormStore, dbc_path, "SpellShapeshiftForm.dbc");

    LOAD_DBC("DBC/ItemRandomProperties.dbc", randompropsFormat, true, dbcRandomProps, false);

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAreaGroupStore, dbc_path, "AreaGroup.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAreaStore, dbc_path, "AreaTable.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sFactionTemplateStore, dbc_path, "FactionTemplate.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sFactionStore, dbc_path, "Faction.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiNodesStore, dbc_path, "TaxiNodes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiPathStore, dbc_path, "TaxiPath.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiPathNodeStore, dbc_path, "TaxiPathNode.dbc");

    LOAD_DBC("DBC/CreatureSpellData.dbc", creaturespelldataFormat, true, dbcCreatureSpellData, false);

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureFamilyStore, dbc_path, "CreatureFamily.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrRacesStore, dbc_path, "ChrRaces.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrClassesStore, dbc_path, "ChrClasses.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMapStore, dbc_path, "Map.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sHolidaysStore, dbc_path, "Holidays.dbc");       //loaded but not used
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAuctionHouseStore, dbc_path, "AuctionHouse.dbc");

    LOAD_DBC("DBC/ItemRandomSuffix.dbc", itemrandomsuffixformat, true, dbcItemRandomSuffix, false);

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

    LOAD_DBC("DBC/ScalingStatValues.dbc", scalingstatvaluesformat, true, dbcScalingStatValues, false);

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemLimitCategoryStore, dbc_path, "ItemLimitCategory.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sQuestXPStore, dbc_path, "QuestXP.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMailTemplateStore, dbc_path, "MailTemplate.dbc");

    LOAD_DBC("DBC/WMOAreaTable.dbc", wmoareaformat, true, dbcWMOAreaTable, false);
    LOAD_DBC("DBC/SummonProperties.dbc", summonpropertiesformat, true, dbcSummonProperties, false);

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sNameGenStore, dbc_path, "NameGen.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLFGDungeonStore, dbc_path, "LFGDungeons.dbc");

    LOAD_DBC("DBC/Vehicle.dbc", VehicleEntryfmt, true, dbcVehicle, true);
    LOAD_DBC("DBC/VehicleSeat.dbc", VehicleSeatEntryfmt, true, dbcVehicleSeat, false);

    MapManagement::AreaManagement::AreaStorage::Initialise(&sAreaStore);
    auto area_map_collection = MapManagement::AreaManagement::AreaStorage::GetMapCollection();
    for (uint32 i = 0; i < sMapStore.GetNumRows(); ++i)
    {
        auto map_object = sMapStore.LookupEntry(i);
        if (map_object == nullptr)
            continue;

        area_map_collection->insert(std::pair<uint32, uint32>(map_object->id, map_object->linked_zone));
    }
    //auto wmo_row_count = dbcWMOAreaTable.GetNumRows();
    for (auto i = 0; i < 51119; ++i) // This is a hack, dbc loading needs rework
    {
        if (auto entry = dbcWMOAreaTable.LookupEntry(i))
        {
            sWMOAreaInfoByTripple.insert(WMOAreaInfoByTripple::value_type(WMOAreaTableTripple(entry->rootId, entry->adtId, entry->groupId), entry));
            MapManagement::AreaManagement::AreaStorage::AddWMOTripleEntry(entry->groupId, entry->rootId, entry->adtId, entry->areaId);
        }
    }
    return true;
}

const WMOAreaTableEntry* GetWMOAreaTableEntryByTriple(int32 root_id, int32 adt_id, int32 group_id)
{
    auto iter = sWMOAreaInfoByTripple.find(WMOAreaTableTripple(root_id, adt_id, group_id));
    if (iter == sWMOAreaInfoByTripple.end())
        return 0;
    return iter->second;
}
