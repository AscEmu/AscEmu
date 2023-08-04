/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldConf.h"
#include "DBCStructures.hpp"
#include "Management/TaxiMgr.h"
#include "DBCGlobals.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "Server/Definitions.h"

typedef std::map<uint32_t, DBC::Structures::MapDifficulty> MapDifficultyMap;
typedef std::vector<DBC::Structures::TaxiPathNodeEntry const*> TaxiPathNodeList;
typedef std::vector<TaxiPathNodeList> TaxiPathNodesByPath;

inline float GetRadius(DBC::Structures::SpellRadiusEntry const* radius)
{
    if (radius == nullptr)
        return 0;

    return radius->radius_min;
}

inline uint32_t GetCastTime(DBC::Structures::SpellCastTimesEntry const* time)
{
    if (time == nullptr)
        return 0;

    return time->CastTime;
}
inline uint32_t GetDuration(DBC::Structures::SpellDurationEntry const* dur)
{
    if (dur == nullptr)
        return 0;
    return dur->Duration1;
}

extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTableEntry> sAreaStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTriggerEntry> sAreaTriggerStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AuctionHouseEntry> sAuctionHouseStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::BankBagSlotPrices> sBankBagSlotPricesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ChatChannelsEntry> sChatChannelsStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CharStartOutfitEntry> sCharStartOutfitStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrClassesEntry> sChrClassesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrRacesEntry> sChrRacesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureDisplayInfoEntry> sCreatureDisplayInfoStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureModelDataEntry> sCreatureModelDataStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureSpellDataEntry> sCreatureSpellDataStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureFamilyEntry> sCreatureFamilyStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityCostsEntry> sDurabilityCostsStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityQualityEntry> sDurabilityQualityStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::EmotesTextEntry> sEmotesTextStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionEntry> sFactionStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionTemplateEntry> sFactionTemplateStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemSetEntry> sItemSetStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemRandomPropertiesEntry> sItemRandomPropertiesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::LFGDungeonEntry> sLFGDungeonStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::LiquidTypeEntry> sLiquidTypeStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::LockEntry> sLockStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::MailTemplateEntry> sMailTemplateStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::MapEntry> sMapStore;
extern MapDifficultyMap sMapDifficultyMap;

extern SERVER_DECL DBC::DBCStorage<DBC::Structures::NameGenEntry> sNameGenStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SkillLineAbilityEntry> sSkillLineAbilityStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SkillLineEntry> sSkillLineStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellEntry> sSpellStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCastTimesEntry> sSpellCastTimesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellDurationEntry> sSpellDurationStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellItemEnchantmentEntry> sSpellItemEnchantmentStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRadiusEntry> sSpellRadiusStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRangeEntry> sSpellRangeStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellShapeshiftFormEntry> sSpellShapeshiftFormStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentEntry> sTalentStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentTabEntry> sTalentTabStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiNodesEntry> sTaxiNodesStore;
extern TaxiPathSetBySource sTaxiPathSetBySource;

extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathEntry> sTaxiPathStore;
extern TaxiPathNodesByPath sTaxiPathNodesByPath;

extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathNodeEntry> sTaxiPathNodeStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TransportAnimationEntry> sTransportAnimationStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::WMOAreaTableEntry> sWMOAreaTableStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::WorldMapOverlayEntry> sWorldMapOverlayStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritEntry> sGtChanceToMeleeCritStore; // todo: available for versions > Classic
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore; // todo: available for versions > Classic
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritEntry> sGtChanceToSpellCritStore; // todo: available for versions > Classic
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore; // todo: available for versions > Classic
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtCombatRatingsEntry> sGtCombatRatingsStore; // todo: available for versions > Classic
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenMPEntry> sGtOCTRegenMPStore; // todo: available for versions > Classic
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenMPPerSptEntry> sGtRegenMPPerSptStore; // todo: available for versions > Classic
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemRandomSuffixEntry> sItemRandomSuffixStore; // todo: available for versions > Classic
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SummonPropertiesEntry> sSummonPropertiesStore; // todo: available for versions > Classic
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleEntry> sVehicleStore; // todo: available for versions > WotLK
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleSeatEntry> sVehicleSeatStore; // todo: available for versions > WotLK

#if VERSION_STRING < Cata
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenHPEntry> sGtOCTRegenHPStore; // todo: available for versions > Classic
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenHPPerSptEntry> sGtRegenHPPerSptStore; // todo: available for versions > Classic
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemEntry> sItemStore; // todo: available for versions > Classic
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemExtendedCostEntry> sItemExtendedCostStore; // todo: available for versions > Classic
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::StableSlotPrices> sStableSlotPricesStore;
#endif

#ifdef AE_TBC
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemDisplayInfo> sItemDisplayInfoStore;
#endif

#if VERSION_STRING >= TBC
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CharTitlesEntry> sCharTitlesStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GemPropertiesEntry> sGemPropertiesStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TotemCategoryEntry> sTotemCategoryStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::WorldMapAreaEntry> sWorldMapAreaStore;
#endif

#if VERSION_STRING >= WotLK
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AchievementEntry> sAchievementStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AchievementCriteriaEntry> sAchievementCriteriaStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaGroupEntry> sAreaGroupStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::BarberShopStyleEntry> sBarberShopStyleStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CurrencyTypesEntry> sCurrencyTypesStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::DungeonEncounterEntry> sDungeonEncounterStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TransportRotationEntry> sTransportRotationStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GlyphPropertiesEntry> sGlyphPropertiesStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GlyphSlotEntry> sGlyphSlotStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtBarberShopCostBaseEntry> sBarberShopCostBaseStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::HolidaysEntry> sHolidaysStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemLimitCategoryEntry> sItemLimitCategoryStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::MapDifficultyEntry> sMapDifficultyStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::QuestXP> sQuestXPStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ScalingStatDistributionEntry> sScalingStatDistributionStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ScalingStatValuesEntry> sScalingStatValuesStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellDifficultyEntry> sSpellDifficultyStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRuneCostEntry> sSpellRuneCostStore;
#endif

#if VERSION_STRING >= Cata
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::BannedAddOnsEntry> sBannedAddOnsStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrPowerTypesEntry> sChrPowerTypesEntry;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTBaseHPByClassEntry> sGtOCTBaseHPByClassStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTBaseMPByClassEntry> sGtOCTBaseMPByClassStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTClassCombatRatingScalarEntry> sGtOCTClassCombatRatingScalarStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GuildPerkSpellsEntry> sGuildPerkSpellsStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::EmotesEntry> sEmotesStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::MountCapabilityEntry> sMountCapabilityStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::MountTypeEntry> sMountTypeStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::NumTalentsAtLevel> sNumTalentsAtLevel;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::PhaseEntry> sPhaseStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::QuestSortEntry> sQuestSortStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellAuraOptionsEntry> sSpellAuraOptionsStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellAuraRestrictionsEntry> sSpellAuraRestrictionsStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCastingRequirementsEntry> sSpellCastingRequirementsStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCategoriesEntry> sSpellCategoriesStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellClassOptionsEntry> sSpellClassOptionsStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCooldownsEntry> sSpellCooldownsStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellEffectEntry> sSpellEffectStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellEquippedItemsEntry> sSpellEquippedItemsStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellInterruptsEntry> sSpellInterruptsStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellLevelsEntry> sSpellLevelsStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellPowerEntry> sSpellPowerStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellScalingEntry> sSpellScalingStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellShapeshiftEntry> sSpellShapeshiftStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellTargetRestrictionsEntry> sSpellTargetRestrictionsStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellTotemsEntry> sSpellTotemsStore;
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentTreePrimarySpells> sTalentTreePrimarySpellsStore;
    #if VERSION_STRING < Mop
        extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellReagentsEntry> sSpellReagentsStore;
    #endif
#endif

#ifdef AE_CATA
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemReforgeEntry> sItemReforgeStore;
#endif

#ifdef AE_MOP
    extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellMiscEntry> sSpellMiscStore;
#endif


#if VERSION_STRING >= Cata
    DBC::Structures::SpellEffectEntry const* GetSpellEffectEntry(uint32_t spellId, uint8_t effect);
    uint8_t getPowerIndexByClass(uint8_t playerClass, uint8_t powerIndex);
#endif

#if VERSION_STRING >= WotLK
    DBC::Structures::MapDifficulty const* getDownscaledMapDifficultyData(uint32_t mapId, InstanceDifficulty::Difficulties& difficulty);
#endif

DBC::Structures::WMOAreaTableEntry const* GetWMOAreaTableEntryByTriple(int32_t root_id, int32_t adt_id, int32_t group_id);

DBC::Structures::CharStartOutfitEntry const* getStartOutfitByRaceClass(uint8_t race, uint8_t class_, uint8_t gender);

DBC::Structures::MapDifficulty const* getMapDifficultyData(uint32_t mapId, InstanceDifficulty::Difficulties difficulty);

std::string generateName(uint32_t type = 0);

uint32_t const* getTalentTabPages(uint8_t playerClass);

uint32_t getLiquidFlags(uint32_t liquidId);

bool loadDBCs();
