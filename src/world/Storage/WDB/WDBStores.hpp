/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "Management/TaxiMgr.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "WDBGlobals.hpp"
#include "WDBStructures.hpp"
#include "Spell/Definitions/PowerType.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace WDB::Structures
{
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
    struct TalentTabEntry;
    struct SkillLineAbilityEntry;
    struct NameGenEntry;
    struct MapEntry;
    struct LockEntry;
    struct LiquidTypeEntry;
    struct LFGDungeonEntry;
    struct ItemRandomPropertiesEntry;
    struct EmotesTextEntry;
    struct AreaTableEntry;
    struct SpellDurationEntry;
    struct SpellCastTimesEntry;
    struct SpellRadiusEntry;
    struct MapDifficulty;

#if VERSION_STRING < Cata
    struct GtOCTRegenHPEntry;
    struct GtRegenHPPerSptEntry;
#endif

    struct ChrSpecializationEntry;

#if VERSION_STRING >= WotLK
    struct DungeonEncounterEntry;
    struct TransportRotationEntry;
    struct GlyphPropertiesEntry;
    struct GlyphSlotEntry;
    struct GtBarberShopCostBaseEntry;
    struct QuestXP;
    struct ScalingStatDistributionEntry;
    struct ScalingStatValuesEntry;
    struct SpellDifficultyEntry;
    struct SpellRuneCostEntry;
#endif

#if VERSION_STRING >= Cata
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
    struct SpellAuraRestrictionsEntry;
    struct SpellCastingRequirementsEntry;
    struct SpellCategoriesEntry;
    struct SpellClassOptionsEntry;
    struct SpellCooldownsEntry;
    struct SpellEffectEntry;
    struct SpellEquippedItemsEntry;
    struct SpellInterruptsEntry;
    struct SpellLevelsEntry;
    struct SpellScalingEntry;
    struct SpellTargetRestrictionsEntry;
    struct SpellTotemsEntry;
    struct TalentTreePrimarySpells;
    struct SpellReagentsEntry;
#endif

#ifdef AE_CATA
    struct ItemReforgeEntry;
#endif

#if VERSION_STRING >= Mop
    struct BattlePetSpeciesEntry;
    struct BattlePetBreedStateEntry;
    struct BattlePetSpeciesStateEntry;
    struct BattlePetBreedQualityEntry;
    struct ItemToBattlePetEntry;
#endif
}

using MapDifficultyMap = std::map<uint32_t, WDB::Structures::MapDifficulty>;
using TaxiPathNodeList = std::vector<WDB::Structures::TaxiPathNodeEntry const*>;
using TaxiPathNodesByPath = std::vector<TaxiPathNodeList>;

[[nodiscard]] constexpr float getRadius(WDB::Structures::SpellRadiusEntry const* radius) noexcept
{
    return radius ? radius->radius_min : 0.0f;
}

[[nodiscard]] constexpr uint32_t getCastTime(WDB::Structures::SpellCastTimesEntry const* time) noexcept
{
    return time ? time->CastTime : 0;
}

[[nodiscard]] constexpr uint32_t getDuration(WDB::Structures::SpellDurationEntry const* dur) noexcept
{
    return dur ? dur->Duration1 : 0;
}

inline SERVER_DECL WDB::WDBStore<WDB::Structures::AchievementCategoryEntry> sAchievementCategoryStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::AchievementCriteriaEntry> sAchievementCriteriaStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::AchievementEntry> sAchievementStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::AreaTableEntry> sAreaStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::AreaTriggerEntry> sAreaTriggerStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::AuctionHouseEntry> sAuctionHouseStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::BankBagSlotPricesEntry> sBankBagSlotPricesStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::ChatChannelsEntry> sChatChannelsStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::CharStartOutfitEntry> sCharStartOutfitStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::ChrClassesEntry> sChrClassesStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::ChrRacesEntry> sChrRacesStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::CreatureDisplayInfoEntry> sCreatureDisplayInfoStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::CreatureModelDataEntry> sCreatureModelDataStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::CreatureSpellDataEntry> sCreatureSpellDataStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::CreatureFamilyEntry> sCreatureFamilyStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::CriteriaEntry> sCriteriaStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::CriteriaTreeEntry> sCriteriaTreeStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::CurrencyTypesEntry> sCurrencyTypesStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::DurabilityCostsEntry> sDurabilityCostsStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::DurabilityQualityEntry> sDurabilityQualityStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::EmotesTextEntry> sEmotesTextStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::FactionEntry> sFactionStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::FactionTemplateEntry> sFactionTemplateStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::FriendshipRepReactionEntry> sFriendshipRepReactionStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::FriendshipReputationEntry> sFriendshipReputationStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::ItemSetEntry> sItemSetStore;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemRandomPropertiesEntry> sItemRandomPropertiesStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::LFGDungeonEntry> sLFGDungeonStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::LiquidTypeEntry> sLiquidTypeStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::LockEntry> sLockStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::MailTemplateEntry> sMailTemplateStore;

inline SERVER_DECL WDB::WDBStore<WDB::Structures::MapEntry> sMapStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::ModifierTreeEntry> sModifierTreeStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::PlayerConditionEntry> sPlayerConditionStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::WorldStateExpressionEntry> sWorldStateExpressionStore;
inline MapDifficultyMap sMapDifficultyMap;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::NameGenEntry> sNameGenStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SkillLineAbilityEntry> sSkillLineAbilityStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::SkillLineEntry> sSkillLineStore;
extern SERVER_DECL std::unordered_map<uint32_t, std::vector<uint32_t>> sSkillLineChildren; // parent skill line -> expansion lines, own client data only

// an item effect of the client data: the spell an item casts by its trigger
struct ClientItemEffect
{
    uint32_t spellId = 0;
    uint8_t trigger = 0;
    int16_t charges = 0;
    int32_t cooldown = -1;
    uint16_t category = 0;
    int32_t categoryCooldown = -1;
};

extern SERVER_DECL std::unordered_map<uint32_t, std::vector<ClientItemEffect>> sItemEffects; // item -> effects, own client data only
inline SERVER_DECL WDB::WDBStore<WDB::Structures::SpellEntry> sSpellStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellCastTimesEntry> sSpellCastTimesStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellDurationEntry> sSpellDurationStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::SpellItemEnchantmentEntry> sSpellItemEnchantmentStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellRadiusEntry> sSpellRadiusStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::SpellRangeEntry> sSpellRangeStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::SpellShapeshiftFormEntry> sSpellShapeshiftFormStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::TalentEntry> sTalentStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::TalentTabEntry> sTalentTabStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::TaxiNodesEntry> sTaxiNodesStore;
extern TaxiPathSetBySource sTaxiPathSetBySource;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::TaxiPathEntry> sTaxiPathStore;
extern TaxiPathNodesByPath sTaxiPathNodesByPath;

inline SERVER_DECL WDB::WDBStore<WDB::Structures::TaxiPathNodeEntry> sTaxiPathNodeStore;
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

inline SERVER_DECL WDB::WDBStore<WDB::Structures::ItemEntry> sItemStore;
extern SERVER_DECL WDB::WDBStore<WDB::Structures::ItemExtendedCostEntry> sItemExtendedCostStore;

#if VERSION_STRING < Cata
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTRegenHPEntry> sGtOCTRegenHPStore; // todo: available for versions > Classic
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtRegenHPPerSptEntry> sGtRegenHPPerSptStore; // todo: available for versions > Classic
#endif

inline SERVER_DECL WDB::WDBStore<WDB::Structures::StableSlotPricesEntry> sStableSlotPricesStore;

inline SERVER_DECL WDB::WDBStore<WDB::Structures::ItemDisplayInfoEntry> sItemDisplayInfoStore;

inline SERVER_DECL WDB::WDBStore<WDB::Structures::CharTitlesEntry> sCharTitlesStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::GemPropertiesEntry> sGemPropertiesStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::TotemCategoryEntry> sTotemCategoryStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::WorldMapAreaEntry> sWorldMapAreaStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::AreaGroupEntry> sAreaGroupStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::BarberShopStyleEntry> sBarberShopStyleStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::MapDifficultyEntry> sMapDifficultyStore;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ChrSpecializationEntry> sChrSpecializationStore;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GlyphPropertiesEntry> sGlyphPropertiesStore;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::DungeonEncounterEntry> sDungeonEncounterStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::TransportRotationEntry> sTransportRotationStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GlyphSlotEntry> sGlyphSlotStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtBarberShopCostBaseEntry> sBarberShopCostBaseStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::HolidaysEntry> sHolidaysStore;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::QuestXP> sQuestXPStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ScalingStatDistributionEntry> sScalingStatDistributionStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ScalingStatValuesEntry> sScalingStatValuesStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellDifficultyEntry> sSpellDifficultyStore;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellRuneCostEntry> sSpellRuneCostStore;

inline SERVER_DECL WDB::WDBStore<WDB::Structures::ItemLimitCategoryEntry> sItemLimitCategoryStore;

inline SERVER_DECL WDB::WDBStore<WDB::Structures::BannedAddOnsEntry> sBannedAddOnsStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::ChrPowerTypesEntry> sChrPowerTypesStore;
inline std::array<std::array<uint8_t, TOTAL_PLAYER_POWER_TYPES>, MAX_PLAYER_CLASSES> powerIndexByClass;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTBaseHPByClassEntry> sGtOCTBaseHPByClassStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTBaseMPByClassEntry> sGtOCTBaseMPByClassStore;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GuildPerkSpellsEntry> sGuildPerkSpellsStore;

inline SERVER_DECL WDB::WDBStore<WDB::Structures::SpellPowerEntry> sSpellPowerStore;
inline SERVER_DECL WDB::WDBStore<WDB::Structures::SpellAuraOptionsEntry> sSpellAuraOptionsStore;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTClassCombatRatingScalarEntry> sGtOCTClassCombatRatingScalarStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::EmotesEntry> sEmotesStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemCurrencyCostEntry> sItemCurrencyCostStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::MountCapabilityEntry> sMountCapabilityStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::MountTypeEntry> sMountTypeStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::NumTalentsAtLevel> sNumTalentsAtLevel;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::PhaseEntry> sPhaseStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::QuestSortEntry> sQuestSortStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellAuraRestrictionsEntry> sSpellAuraRestrictionsStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellCastingRequirementsEntry> sSpellCastingRequirementsStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellCategoriesEntry> sSpellCategoriesStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellClassOptionsEntry> sSpellClassOptionsStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellCooldownsEntry> sSpellCooldownsStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellEffectEntry> sSpellEffectStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellEquippedItemsEntry> sSpellEquippedItemsStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellInterruptsEntry> sSpellInterruptsStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellLevelsEntry> sSpellLevelsStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellScalingEntry> sSpellScalingStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellShapeshiftEntry> sSpellShapeshiftStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellTargetRestrictionsEntry> sSpellTargetRestrictionsStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellTotemsEntry> sSpellTotemsStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::TalentTreePrimarySpells> sTalentTreePrimarySpellsStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellReagentsEntry> sSpellReagentsStore;

#ifdef AE_CATA
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemReforgeEntry> sItemReforgeStore;
#endif

inline SERVER_DECL WDB::WDBStore<WDB::Structures::SpellMiscEntry> sSpellMiscStore;

extern SERVER_DECL WDB::WDBContainer<WDB::Structures::BattlePetSpeciesEntry> sBattlePetSpeciesStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::BattlePetBreedStateEntry> sBattlePetBreedStateStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::BattlePetSpeciesStateEntry> sBattlePetSpeciesStateStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::BattlePetBreedQualityEntry> sBattlePetBreedQualityStore;
extern SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemToBattlePetEntry> sItemToBattlePetStore;

WDB::Structures::SpellPowerEntry const* getSpellPowerEntry(uint32_t spellId);

WDB::Structures::SpellEffectEntry const* GetSpellEffectEntry(uint32_t spellId, uint8_t effect);
uint8_t getPowerIndexByClass(uint8_t playerClass, uint8_t powerIndex);

WDB::Structures::MapDifficulty const* getDownscaledMapDifficultyData(uint32_t mapId, InstanceDifficulty::Difficulties& difficulty);
WDB::Structures::MapDifficulty const* getMapDifficultyData(uint32_t mapId, InstanceDifficulty::Difficulties difficulty);

WDB::Structures::WMOAreaTableEntry const* GetWMOAreaTableEntryByTriple(int32_t root_id, int32_t adt_id, int32_t group_id);

WDB::Structures::CharStartOutfitEntry const* getStartOutfitByRaceClass(uint8_t race, uint8_t class_, uint8_t gender);

std::string generateName(uint32_t type = 0);

uint32_t const* getTalentTabPages(uint8_t playerClass);
uint32_t getTalentInspectBitPosInTab(uint32_t talentId);
uint32_t getTalentTabInspectBitSize(uint32_t talentTabId);

#if VERSION_STRING >= Mop
uint32_t const* getClassSpecializations(uint8_t playerClass);
#endif

uint32_t getLiquidFlags(uint32_t liquidId);

bool loadDBCs();

// Fills the stores from the DB2 tables of a client with its own data (WoW::hasOwnClientData),
// the counterpart of the dbc loading in loadDBCs(). Defined in WDBStoresDb2.cpp.
bool loadDb2Stores(WDB::StoreProblemList& errors, std::string const& dbcPath);
