/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/TaxiMgr.hpp"
#include "WDBGlobals.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "AEVersion.hpp"

namespace WDB::Structures
{
    struct ItemExtendedCostEntry;
    struct ItemEntry;
    struct VehicleSeatEntry;
    struct VehicleEntry;
    struct SummonPropertiesEntry;
    struct ItemRandomSuffixEntry;
    struct GtRegenMPPerSptEntry;
    struct GtOCTRegenMPEntry;
    struct GtCombatRatingsEntry;
    struct GtChanceToSpellCritBaseEntry;
    struct GtChanceToSpellCritEntry;
    struct GtChanceToMeleeCritBaseEntry;
    struct GtChanceToMeleeCritEntry;
    struct WorldMapOverlayEntry;
    struct WMOAreaTableEntry;
    struct TransportAnimationEntry;
    struct TaxiPathEntry;
    struct TaxiNodesEntry;
    struct TalentTabEntry;
    struct TalentEntry;
    struct SpellShapeshiftFormEntry;
    struct SpellRangeEntry;
    struct SpellItemEnchantmentEntry;
    struct SpellEntry;
    struct SkillLineEntry;
    struct SkillLineAbilityEntry;
    struct NameGenEntry;
    struct MapEntry;
    struct MailTemplateEntry;
    struct LockEntry;
    struct LiquidTypeEntry;
    struct LFGDungeonEntry;
    struct ItemRandomPropertiesEntry;
    struct ItemSetEntry;
    struct GameObjectDisplayInfoEntry;
    struct FactionTemplateEntry;
    struct FactionEntry;
    struct EmotesTextEntry;
    struct DurabilityQualityEntry;
    struct DurabilityCostsEntry;
    struct CreatureFamilyEntry;
    struct CreatureSpellDataEntry;
    struct CreatureDisplayInfoEntry;
    struct ChrRacesEntry;
    struct CreatureModelDataEntry;
    struct CreatureDisplayInfoExtraEntry;
    struct ChrClassesEntry;
    struct CharStartOutfitEntry;
    struct ChatChannelsEntry;
    struct BankBagSlotPrices;
    struct AuctionHouseEntry;
    struct AreaTriggerEntry;
    struct AreaTableEntry;
    struct SpellDurationEntry;
    struct SpellCastTimesEntry;
    struct SpellRadiusEntry;
    struct TaxiPathNodeEntry;
    struct MapDifficulty;

#if VERSION_STRING < Cata
    struct GtOCTRegenHPEntry;
    struct GtRegenHPPerSptEntry;

    struct StableSlotPrices;
#endif

#ifdef AE_TBC
    struct ItemDisplayInfo;
#endif

#if VERSION_STRING >= TBC
    struct CharTitlesEntry;
    struct GemPropertiesEntry;
    struct TotemCategoryEntry;
    struct WorldMapAreaEntry;
#endif

#if VERSION_STRING >= WotLK
    struct AchievementEntry;
    struct AchievementCriteriaEntry;
    struct AreaGroupEntry;
    struct BarberShopStyleEntry;
    struct CurrencyTypesEntry;
    struct DungeonEncounterEntry;
    struct TransportRotationEntry;
    struct GlyphPropertiesEntry;
    struct GlyphSlotEntry;
    struct GtBarberShopCostBaseEntry;
    struct HolidaysEntry;
    struct ItemLimitCategoryEntry;
    struct MapDifficultyEntry;
    struct QuestXP;
    struct ScalingStatDistributionEntry;
    struct ScalingStatValuesEntry;
    struct SpellDifficultyEntry;
    struct SpellRuneCostEntry;
#endif

#if VERSION_STRING >= Cata
    struct BannedAddOnsEntry;
    struct ChrPowerTypesEntry;
    struct GtOCTBaseHPByClassEntry;
    struct GtOCTBaseMPByClassEntry;
    struct GtOCTClassCombatRatingScalarEntry;
    struct GuildPerkSpellsEntry;
    struct EmotesEntry;
    struct ItemCurrencyCostEntry;
    struct MountCapabilityEntry;
    struct MountTypeEntry;
    struct NumTalentsAtLevel;
    struct PhaseEntry;
    struct QuestSortEntry;
    struct SpellAuraOptionsEntry;
    struct SpellAuraRestrictionsEntry;
    struct SpellCastingRequirementsEntry;
    struct SpellCategoriesEntry;
    struct SpellClassOptionsEntry;
    struct SpellCooldownsEntry;
    struct SpellEffectEntry;
    struct SpellEquippedItemsEntry;
    struct SpellInterruptsEntry;
    struct SpellLevelsEntry;
    struct SpellPowerEntry;
    struct SpellScalingEntry;
    struct SpellShapeshiftEntry;
    struct SpellTargetRestrictionsEntry;
    struct SpellTotemsEntry;
    struct TalentTreePrimarySpells;
    struct SpellReagentsEntry;
#endif

#ifdef AE_CATA
    struct ItemReforgeEntry;
#endif

#ifdef AE_MOP
    struct SpellMiscEntry;
#endif
}

typedef std::map<uint32_t, WDB::Structures::MapDifficulty> MapDifficultyMap;
typedef std::vector<WDB::Structures::TaxiPathNodeEntry const*> TaxiPathNodeList;
typedef std::vector<TaxiPathNodeList> TaxiPathNodesByPath;

extern float SERVER_DECL GetRadius(WDB::Structures::SpellRadiusEntry const* radius);
extern uint32_t SERVER_DECL GetCastTime(WDB::Structures::SpellCastTimesEntry const* time);
extern uint32_t SERVER_DECL GetDuration(WDB::Structures::SpellDurationEntry const* dur);

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::AreaTableEntry> sAreaStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::AreaTriggerEntry> sAreaTriggerStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::AuctionHouseEntry> sAuctionHouseStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::BankBagSlotPrices> sBankBagSlotPricesStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ChatChannelsEntry> sChatChannelsStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::CharStartOutfitEntry> sCharStartOutfitStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ChrClassesEntry> sChrClassesStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ChrRacesEntry> sChrRacesStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::CreatureDisplayInfoEntry> sCreatureDisplayInfoStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::CreatureModelDataEntry> sCreatureModelDataStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::CreatureSpellDataEntry> sCreatureSpellDataStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::CreatureFamilyEntry> sCreatureFamilyStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::DurabilityCostsEntry> sDurabilityCostsStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::DurabilityQualityEntry> sDurabilityQualityStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::EmotesTextEntry> sEmotesTextStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::FactionEntry> sFactionStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::FactionTemplateEntry> sFactionTemplateStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemSetEntry> sItemSetStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemRandomPropertiesEntry> sItemRandomPropertiesStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::LFGDungeonEntry> sLFGDungeonStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::LiquidTypeEntry> sLiquidTypeStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::LockEntry> sLockStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::MailTemplateEntry> sMailTemplateStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::MapEntry> sMapStore;
extern MapDifficultyMap sMapDifficultyMap;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::NameGenEntry> sNameGenStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SkillLineAbilityEntry> sSkillLineAbilityStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SkillLineEntry> sSkillLineStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellEntry> sSpellStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellCastTimesEntry> sSpellCastTimesStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellDurationEntry> sSpellDurationStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellItemEnchantmentEntry> sSpellItemEnchantmentStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellRadiusEntry> sSpellRadiusStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellRangeEntry> sSpellRangeStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellShapeshiftFormEntry> sSpellShapeshiftFormStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::TalentEntry> sTalentStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::TalentTabEntry> sTalentTabStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::TaxiNodesEntry> sTaxiNodesStore;
extern TaxiPathSetBySource sTaxiPathSetBySource;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::TaxiPathEntry> sTaxiPathStore;
extern TaxiPathNodesByPath sTaxiPathNodesByPath;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::TaxiPathNodeEntry> sTaxiPathNodeStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::TransportAnimationEntry> sTransportAnimationStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::WMOAreaTableEntry> sWMOAreaTableStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::WorldMapOverlayEntry> sWorldMapOverlayStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtChanceToMeleeCritEntry> sGtChanceToMeleeCritStore; // todo: available for versions > Classic
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore; // todo: available for versions > Classic
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtChanceToSpellCritEntry> sGtChanceToSpellCritStore; // todo: available for versions > Classic
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore; // todo: available for versions > Classic
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtCombatRatingsEntry> sGtCombatRatingsStore; // todo: available for versions > Classic
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTRegenMPEntry> sGtOCTRegenMPStore; // todo: available for versions > Classic
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtRegenMPPerSptEntry> sGtRegenMPPerSptStore; // todo: available for versions > Classic
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemRandomSuffixEntry> sItemRandomSuffixStore; // todo: available for versions > Classic
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SummonPropertiesEntry> sSummonPropertiesStore; // todo: available for versions > Classic
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::VehicleEntry> sVehicleStore; // todo: available for versions > WotLK
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::VehicleSeatEntry> sVehicleSeatStore; // todo: available for versions > WotLK

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemEntry> sItemStore; // todo: available for versions > Classic
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemExtendedCostEntry> sItemExtendedCostStore; // todo: available for versions > Classic

#if VERSION_STRING < Cata
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTRegenHPEntry> sGtOCTRegenHPStore; // todo: available for versions > Classic
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtRegenHPPerSptEntry> sGtRegenHPPerSptStore; // todo: available for versions > Classic

    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::StableSlotPrices> sStableSlotPricesStore;
#endif

#ifdef AE_TBC
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemDisplayInfo> sItemDisplayInfoStore;
#endif

#if VERSION_STRING >= TBC
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::CharTitlesEntry> sCharTitlesStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GemPropertiesEntry> sGemPropertiesStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::TotemCategoryEntry> sTotemCategoryStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::WorldMapAreaEntry> sWorldMapAreaStore;
#endif

#if VERSION_STRING >= WotLK
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::AchievementEntry> sAchievementStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::AchievementCriteriaEntry> sAchievementCriteriaStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::AreaGroupEntry> sAreaGroupStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::BarberShopStyleEntry> sBarberShopStyleStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::CurrencyTypesEntry> sCurrencyTypesStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::DungeonEncounterEntry> sDungeonEncounterStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::TransportRotationEntry> sTransportRotationStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GlyphPropertiesEntry> sGlyphPropertiesStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GlyphSlotEntry> sGlyphSlotStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtBarberShopCostBaseEntry> sBarberShopCostBaseStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::HolidaysEntry> sHolidaysStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemLimitCategoryEntry> sItemLimitCategoryStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::MapDifficultyEntry> sMapDifficultyStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::QuestXP> sQuestXPStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ScalingStatDistributionEntry> sScalingStatDistributionStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ScalingStatValuesEntry> sScalingStatValuesStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellDifficultyEntry> sSpellDifficultyStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellRuneCostEntry> sSpellRuneCostStore;
#endif

#if VERSION_STRING >= Cata
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::BannedAddOnsEntry> sBannedAddOnsStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ChrPowerTypesEntry> sChrPowerTypesEntry;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTBaseHPByClassEntry> sGtOCTBaseHPByClassStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTBaseMPByClassEntry> sGtOCTBaseMPByClassStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTClassCombatRatingScalarEntry> sGtOCTClassCombatRatingScalarStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GuildPerkSpellsEntry> sGuildPerkSpellsStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::EmotesEntry> sEmotesStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemCurrencyCostEntry> sItemCurrencyCostStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::MountCapabilityEntry> sMountCapabilityStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::MountTypeEntry> sMountTypeStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::NumTalentsAtLevel> sNumTalentsAtLevel;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::PhaseEntry> sPhaseStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::QuestSortEntry> sQuestSortStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellAuraOptionsEntry> sSpellAuraOptionsStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellAuraRestrictionsEntry> sSpellAuraRestrictionsStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellCastingRequirementsEntry> sSpellCastingRequirementsStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellCategoriesEntry> sSpellCategoriesStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellClassOptionsEntry> sSpellClassOptionsStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellCooldownsEntry> sSpellCooldownsStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellEffectEntry> sSpellEffectStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellEquippedItemsEntry> sSpellEquippedItemsStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellInterruptsEntry> sSpellInterruptsStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellLevelsEntry> sSpellLevelsStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellPowerEntry> sSpellPowerStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellScalingEntry> sSpellScalingStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellShapeshiftEntry> sSpellShapeshiftStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellTargetRestrictionsEntry> sSpellTargetRestrictionsStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellTotemsEntry> sSpellTotemsStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::TalentTreePrimarySpells> sTalentTreePrimarySpellsStore;
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellReagentsEntry> sSpellReagentsStore;
#endif

#ifdef AE_CATA
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemReforgeEntry> sItemReforgeStore;
#endif

#ifdef AE_MOP
    extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellMiscEntry> sSpellMiscStore;
#endif


#if VERSION_STRING >= Cata
    WDB::Structures::SpellEffectEntry const* GetSpellEffectEntry(uint32_t spellId, uint8_t effect);
    uint8_t getPowerIndexByClass(uint8_t playerClass, uint8_t powerIndex);
#endif

#if VERSION_STRING >= WotLK
    WDB::Structures::MapDifficulty const* getDownscaledMapDifficultyData(uint32_t mapId, InstanceDifficulty::Difficulties& difficulty);
#endif

WDB::Structures::WMOAreaTableEntry const* GetWMOAreaTableEntryByTriple(int32_t root_id, int32_t adt_id, int32_t group_id);

WDB::Structures::CharStartOutfitEntry const* getStartOutfitByRaceClass(uint8_t race, uint8_t class_, uint8_t gender);

WDB::Structures::MapDifficulty const* getMapDifficultyData(uint32_t mapId, InstanceDifficulty::Difficulties difficulty);

std::string generateName(uint32_t type = 0);

uint32_t const* getTalentTabPages(uint8_t playerClass);

uint32_t getLiquidFlags(uint32_t liquidId);

bool loadDBCs();
