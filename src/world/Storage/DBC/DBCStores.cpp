/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "DBCStores.hpp"
#include "WorldConf.h"
#include "Server/World.h"
#include "Storage/DBC/DBCGlobals.hpp"
#include "Map/Area/AreaStorage.hpp"
#if VERSION_STRING >= Cata
    #include "Objects/Units/Players/PlayerDefines.hpp"
    #include "Spell/SpellAuras.h"
#endif

typedef std::map<WMOAreaTableTripple, DBC::Structures::WMOAreaTableEntry const*> WMOAreaInfoByTripple;

struct NameGenData
{
    std::string name;
    uint32_t type;
};

std::vector<NameGenData> _namegenData[3];

SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTableEntry> sAreaStore;
static WMOAreaInfoByTripple sWMOAreaInfoByTripple;
SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTriggerEntry> sAreaTriggerStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::AuctionHouseEntry> sAuctionHouseStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::BankBagSlotPrices> sBankBagSlotPricesStore;

SERVER_DECL DBC::DBCStorage<DBC::Structures::CharStartOutfitEntry> sCharStartOutfitStore;
std::map<uint32_t, DBC::Structures::CharStartOutfitEntry const*> sCharStartOutfitMap;

SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrClassesEntry> sChrClassesStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrRacesEntry> sChrRacesStore;

SERVER_DECL DBC::DBCStorage<DBC::Structures::ChatChannelsEntry> sChatChannelsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtCombatRatingsEntry> sGtCombatRatingsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureDisplayInfoEntry> sCreatureDisplayInfoStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureModelDataEntry> sCreatureModelDataStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureSpellDataEntry> sCreatureSpellDataStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureFamilyEntry> sCreatureFamilyStore;

SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiNodesEntry> sTaxiNodesStore;
TaxiPathSetBySource sTaxiPathSetBySource;
SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathEntry> sTaxiPathStore;
TaxiPathNodesByPath sTaxiPathNodesByPath;
SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathNodeEntry> sTaxiPathNodeStore;

SERVER_DECL DBC::DBCStorage<DBC::Structures::TransportAnimationEntry> sTransportAnimationStore;

SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityCostsEntry> sDurabilityCostsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityQualityEntry> sDurabilityQualityStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionTemplateEntry> sFactionTemplateStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionEntry> sFactionStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::EmotesTextEntry> sEmotesTextStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellItemEnchantmentEntry> sSpellItemEnchantmentStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GemPropertiesEntry> sGemPropertiesStore;

SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemRandomSuffixEntry> sItemRandomSuffixStore;

SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemSetEntry> sItemSetStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::LockEntry> sLockStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::MapEntry> sMapStore;

SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemRandomPropertiesEntry> sItemRandomPropertiesStore;

SERVER_DECL DBC::DBCStorage<DBC::Structures::SkillLineAbilityEntry> sSkillLineAbilityStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SkillLineEntry> sSkillLineStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCastTimesEntry> sSpellCastTimesStore;

SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellDurationEntry> sSpellDurationStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellEntry> sSpellStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRadiusEntry> sSpellRadiusStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRangeEntry> sSpellRangeStore;

SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentEntry> sTalentStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentTabEntry> sTalentTabStore;
static uint32_t InspectTalentTabPages[12][3];
SERVER_DECL DBC::DBCStorage<DBC::Structures::WorldMapOverlayEntry> sWorldMapOverlayStore;

SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenMPEntry> sGtOCTRegenMPStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenMPPerSptEntry> sGtRegenMPPerSptStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritEntry> sGtChanceToMeleeCritStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritEntry> sGtChanceToSpellCritStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellShapeshiftFormEntry> sSpellShapeshiftFormStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::MailTemplateEntry> sMailTemplateStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::WMOAreaTableEntry> sWMOAreaTableStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SummonPropertiesEntry> sSummonPropertiesStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::NameGenEntry> sNameGenStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::LFGDungeonEntry> sLFGDungeonStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::LiquidTypeEntry> sLiquidTypeStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleEntry> sVehicleStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleSeatEntry> sVehicleSeatStore;

#if VERSION_STRING < Cata
SERVER_DECL DBC::DBCStorage<DBC::Structures::StableSlotPrices> sStableSlotPricesStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemExtendedCostEntry> sItemExtendedCostStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemEntry> sItemStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenHPEntry> sGtOCTRegenHPStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenHPPerSptEntry> sGtRegenHPPerSptStore;
#endif

#ifdef AE_TBC
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemDisplayInfo> sItemDisplayInfoStore;
#endif

#if VERSION_STRING >= TBC
SERVER_DECL DBC::DBCStorage<DBC::Structures::CharTitlesEntry> sCharTitlesStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::TotemCategoryEntry> sTotemCategoryStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::WorldMapAreaEntry> sWorldMapAreaStore;
#endif

#if VERSION_STRING >= WotLK
SERVER_DECL DBC::DBCStorage<DBC::Structures::BarberShopStyleEntry> sBarberShopStyleStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::CurrencyTypesEntry> sCurrencyTypesStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::TransportRotationEntry> sTransportRotationStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GlyphPropertiesEntry> sGlyphPropertiesStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GlyphSlotEntry> sGlyphSlotStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemLimitCategoryEntry> sItemLimitCategoryStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::HolidaysEntry> sHolidaysStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::ScalingStatDistributionEntry> sScalingStatDistributionStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::ScalingStatValuesEntry> sScalingStatValuesStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellDifficultyEntry> sSpellDifficultyStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRuneCostEntry> sSpellRuneCostStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtBarberShopCostBaseEntry> sBarberShopCostBaseStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::QuestXP> sQuestXPStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::DungeonEncounterEntry> sDungeonEncounterStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::AchievementEntry> sAchievementStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::AchievementCriteriaEntry> sAchievementCriteriaStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaGroupEntry> sAreaGroupStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::MapDifficultyEntry> sMapDifficultyStore;
#endif

MapDifficultyMap sMapDifficultyMap;

#if VERSION_STRING >= Cata
SERVER_DECL DBC::DBCStorage<DBC::Structures::MountCapabilityEntry> sMountCapabilityStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::MountTypeEntry> sMountTypeStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::BannedAddOnsEntry> sBannedAddOnsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrPowerTypesEntry> sChrPowerTypesEntry;
uint8_t powerIndexByClass[MAX_PLAYER_CLASSES][TOTAL_PLAYER_POWER_TYPES];
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTBaseHPByClassEntry> sGtOCTBaseHPByClassStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTBaseMPByClassEntry> sGtOCTBaseMPByClassStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GuildPerkSpellsEntry> sGuildPerkSpellsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::EmotesEntry> sEmotesStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellAuraOptionsEntry> sSpellAuraOptionsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellAuraRestrictionsEntry> sSpellAuraRestrictionsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCastingRequirementsEntry> sSpellCastingRequirementsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCategoriesEntry> sSpellCategoriesStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellClassOptionsEntry> sSpellClassOptionsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCooldownsEntry> sSpellCooldownsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellEquippedItemsEntry> sSpellEquippedItemsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellInterruptsEntry> sSpellInterruptsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellLevelsEntry> sSpellLevelsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellPowerEntry> sSpellPowerStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellScalingEntry> sSpellScalingStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellReagentsEntry> sSpellReagentsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellShapeshiftEntry> sSpellShapeshiftStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellTargetRestrictionsEntry> sSpellTargetRestrictionsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellTotemsEntry> sSpellTotemsStore;
DBC::Structures::SpellEffectMap sSpellEffectMap;
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellEffectEntry> sSpellEffectStore;
DBC::Structures::SpellCategoryStore sSpellCategoryStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentTreePrimarySpells> sTalentTreePrimarySpellsStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTClassCombatRatingScalarEntry> sGtOCTClassCombatRatingScalarStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::QuestSortEntry> sQuestSortStore;
SERVER_DECL DBC::DBCStorage<DBC::Structures::NumTalentsAtLevel> sNumTalentsAtLevel;
SERVER_DECL DBC::DBCStorage<DBC::Structures::PhaseEntry> sPhaseStore;
#endif

#if VERSION_STRING == Cata
SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemReforgeEntry> sItemReforgeStore;
#endif

#if VERSION_STRING >= Mop
SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellMiscEntry> sSpellMiscStore;
#endif

#ifdef AE_CLASSIC
bool LoadDBCs()
{
    uint32_t available_dbc_locales = 0xFFFFFFFF;
    DBC::StoreProblemList bad_dbc_files;
    std::string dbc_path = sWorld.settings.server.dataDir + "dbc/";

#if VERSION_STRING >= TBC
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCharTitlesStore, dbc_path, "CharTitles.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemStore, dbc_path, "Item.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGemPropertiesStore, dbc_path, "GemProperties.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemExtendedCostStore, dbc_path, "ItemExtendedCost.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemRandomSuffixStore, dbc_path, "ItemRandomSuffix.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtCombatRatingsStore, dbc_path, "gtCombatRatings.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritStore, dbc_path, "gtChanceToMeleeCrit.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritBaseStore, dbc_path, "gtChanceToMeleeCritBase.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtRegenMPPerSptStore, dbc_path, "gtRegenMPPerSpt.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritStore, dbc_path, "gtChanceToSpellCrit.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTRegenMPStore, dbc_path, "gtOCTRegenMP.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtRegenHPPerSptStore, dbc_path, "gtRegenHPPerSpt.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSummonPropertiesStore, dbc_path, "SummonProperties.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTRegenHPStore, dbc_path, "gtOCTRegenHP.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritBaseStore, dbc_path, "gtChanceToSpellCritBase.dbc");
#endif

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWorldMapOverlayStore, dbc_path, "WorldMapOverlay.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemSetStore, dbc_path, "ItemSet.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCharStartOutfitStore, dbc_path, "CharStartOutfit.dbc");
    for (uint32_t i = 0; i < sCharStartOutfitStore.GetNumRows(); ++i)
        if (DBC::Structures::CharStartOutfitEntry const* outfit = sCharStartOutfitStore.LookupEntry(i))
            sCharStartOutfitMap[outfit->Race | (outfit->Class << 8) | (outfit->Gender << 16)] = outfit;

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLockStore, dbc_path, "Lock.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sEmotesTextStore, dbc_path, "EmotesText.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSkillLineAbilityStore, dbc_path, "SkillLineAbility.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellItemEnchantmentStore, dbc_path, "SpellItemEnchantment.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSkillLineStore, dbc_path, "SkillLine.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellStore, dbc_path, "Spell.dbc");

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentStore, dbc_path, "Talent.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentTabStore, dbc_path, "TalentTab.dbc");
    {
        std::map<uint32_t, uint32_t> InspectTalentTabPos;
        std::map<uint32_t, uint32_t> InspectTalentTabSize;
        std::map<uint32_t, uint32_t> InspectTalentTabBit;

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

            for (std::map<uint32, uint32>::iterator itr = InspectTalentTabBit.begin(); itr != InspectTalentTabBit.end(); ++itr)
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
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRadiusStore, dbc_path, "SpellRadius.dbc");     ///\todo handle max and level radius
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRangeStore, dbc_path, "SpellRange.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellDurationStore, dbc_path, "SpellDuration.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellShapeshiftFormStore, dbc_path, "SpellShapeshiftForm.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemRandomPropertiesStore, dbc_path, "ItemRandomProperties.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAreaStore, dbc_path, "AreaTable.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sFactionTemplateStore, dbc_path, "FactionTemplate.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sFactionStore, dbc_path, "Faction.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGameObjectDisplayInfoStore, dbc_path, "GameObjectDisplayInfo.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiNodesStore, dbc_path, "TaxiNodes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiPathStore, dbc_path, "TaxiPath.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiPathNodeStore, dbc_path, "TaxiPathNode.dbc");
    {
        for (uint32_t i = 1; i < sTaxiPathStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
            {
                sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->id, entry->price);
            }
        }

        uint32_t pathCount = sTaxiPathStore.GetNumRows();

        // Calculate path nodes count
        std::vector<uint32_t> pathLength;
        pathLength.resize(pathCount);                           // 0 and some other indexes not used
        for (uint32_t i = 0; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            {
                if (pathLength[entry->pathId] < entry->NodeIndex + 1)
                    pathLength[entry->pathId] = entry->NodeIndex + 1;
            }
        }

        // Set path length
        sTaxiPathNodesByPath.resize(pathCount);                 // 0 and some other indexes not used
        for (uint32 i = 1; i < sTaxiPathNodesByPath.size(); ++i)
            sTaxiPathNodesByPath[i].resize(pathLength[i]);

        // fill data
        for (uint32_t i = 0; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            {
                sTaxiPathNodesByPath[entry->pathId][entry->NodeIndex] = entry;
            }
        }
    }
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTransportAnimationStore, dbc_path, "TransportAnimation.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureDisplayInfoStore, dbc_path, "CreatureDisplayInfo.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureDisplayInfoExtraStore, dbc_path, "CreatureDisplayInfoExtra.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureModelDataStore, dbc_path, "CreatureModelData.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureSpellDataStore, dbc_path, "CreatureSpellData.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureFamilyStore, dbc_path, "CreatureFamily.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrRacesStore, dbc_path, "ChrRaces.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrClassesStore, dbc_path, "ChrClasses.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMapStore, dbc_path, "Map.dbc");
    {
        for (uint32_t i = 0; i < sMapStore.GetNumRows(); ++i)
        {
            if (auto entry = sMapStore.LookupEntry(i))
            {
                //                                                               Classic Only has one Difficulty                                   Same reset Time      and only 5 or 40 man Raids
                sMapDifficultyMap[Util::MAKE_PAIR32(entry->id, InstanceDifficulty::Difficulties::DUNGEON_NORMAL)] = DBC::Structures::MapDifficulty(604800, entry->isRaid() ? 40 : 5, false);
            }
        }
    }
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAuctionHouseStore, dbc_path, "AuctionHouse.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChatChannelsStore, dbc_path, "ChatChannels.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sDurabilityQualityStore, dbc_path, "DurabilityQuality.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sDurabilityCostsStore, dbc_path, "DurabilityCosts.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sBankBagSlotPricesStore, dbc_path, "BankBagSlotPrices.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sStableSlotPricesStore, dbc_path, "StableSlotPrices.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAreaTriggerStore, dbc_path, "AreaTrigger.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMailTemplateStore, dbc_path, "MailTemplate.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWMOAreaTableStore, dbc_path, "WMOAreaTable.dbc");
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

    //DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLFGDungeonStore, dbc_path, "LFGDungeons.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLiquidTypeStore, dbc_path, "LiquidType.dbc");
    //DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sVehicleStore, dbc_path, "Vehicle.dbc");
    //DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sVehicleSeatStore, dbc_path, "VehicleSeat.dbc");

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
#endif

#ifdef AE_TBC
bool LoadDBCs()
{
    uint32_t available_dbc_locales = 0xFFFFFFFF;
    DBC::StoreProblemList bad_dbc_files;
    std::string dbc_path = sWorld.settings.server.dataDir + "dbc/";

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWorldMapOverlayStore, dbc_path, "WorldMapOverlay.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCharTitlesStore, dbc_path, "CharTitles.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemStore, dbc_path, "Item.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemSetStore, dbc_path, "ItemSet.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCharStartOutfitStore, dbc_path, "CharStartOutfit.dbc");
    for (uint32_t i = 0; i < sCharStartOutfitStore.GetNumRows(); ++i)
        if (DBC::Structures::CharStartOutfitEntry const* outfit = sCharStartOutfitStore.LookupEntry(i))
            sCharStartOutfitMap[outfit->Race | (outfit->Class << 8) | (outfit->Gender << 16)] = outfit;

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLockStore, dbc_path, "Lock.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sEmotesTextStore, dbc_path, "EmotesText.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSkillLineAbilityStore, dbc_path, "SkillLineAbility.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellItemEnchantmentStore, dbc_path, "SpellItemEnchantment.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGemPropertiesStore, dbc_path, "GemProperties.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSkillLineStore, dbc_path, "SkillLine.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellStore, dbc_path, "Spell.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemDisplayInfoStore, dbc_path, "ItemDisplayInfo.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemExtendedCostStore, dbc_path, "ItemExtendedCost.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentStore, dbc_path, "Talent.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentTabStore, dbc_path, "TalentTab.dbc");
    {
        std::map<uint32_t, uint32_t> InspectTalentTabPos;
        std::map<uint32_t, uint32_t> InspectTalentTabSize;
        std::map<uint32_t, uint32_t> InspectTalentTabBit;

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

            for (std::map<uint32, uint32>::iterator itr = InspectTalentTabBit.begin(); itr != InspectTalentTabBit.end(); ++itr)
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
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRadiusStore, dbc_path, "SpellRadius.dbc");     ///\todo handle max and level radius
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRangeStore, dbc_path, "SpellRange.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellDurationStore, dbc_path, "SpellDuration.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellShapeshiftFormStore, dbc_path, "SpellShapeshiftForm.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemRandomPropertiesStore, dbc_path, "ItemRandomProperties.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAreaStore, dbc_path, "AreaTable.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sFactionTemplateStore, dbc_path, "FactionTemplate.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sFactionStore, dbc_path, "Faction.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGameObjectDisplayInfoStore, dbc_path, "GameObjectDisplayInfo.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiNodesStore, dbc_path, "TaxiNodes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiPathStore, dbc_path, "TaxiPath.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiPathNodeStore, dbc_path, "TaxiPathNode.dbc");
    {
        for (uint32_t i = 1; i < sTaxiPathStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
            {
                sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->id, entry->price);
            }
        }

        uint32_t pathCount = sTaxiPathStore.GetNumRows();

        // Calculate path nodes count
        std::vector<uint32_t> pathLength;
        pathLength.resize(pathCount);                           // 0 and some other indexes not used
        for (uint32_t i = 0; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            {
                if (pathLength[entry->pathId] < entry->NodeIndex + 1)
                    pathLength[entry->pathId] = entry->NodeIndex + 1;
            }
        }

        // Set path length
        sTaxiPathNodesByPath.resize(pathCount);                 // 0 and some other indexes not used
        for (uint32 i = 1; i < sTaxiPathNodesByPath.size(); ++i)
            sTaxiPathNodesByPath[i].resize(pathLength[i]);

        // fill data
        for (uint32_t i = 0; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            {
                sTaxiPathNodesByPath[entry->pathId][entry->NodeIndex] = entry;
            }
        }
    }
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTotemCategoryStore, dbc_path, "TotemCategory.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTransportAnimationStore, dbc_path, "TransportAnimation.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureDisplayInfoStore, dbc_path, "CreatureDisplayInfo.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureDisplayInfoExtraStore, dbc_path, "CreatureDisplayInfoExtra.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureModelDataStore, dbc_path, "CreatureModelData.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureSpellDataStore, dbc_path, "CreatureSpellData.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureFamilyStore, dbc_path, "CreatureFamily.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrRacesStore, dbc_path, "ChrRaces.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrClassesStore, dbc_path, "ChrClasses.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMapStore, dbc_path, "Map.dbc");
    {
        for (uint32_t i = 0; i < sMapStore.GetNumRows(); ++i)
        {
            if (auto entry = sMapStore.LookupEntry(i))
            {
                uint32_t maxPlayers = 5;
                if (entry->addon < 1)
                    maxPlayers = entry->isRaid() ? 40 : 5;
                else
                    maxPlayers = entry->isRaid() ? 25 : 5;

                if (!entry->reset_heroic_tim)
                    sMapDifficultyMap[Util::MAKE_PAIR32(entry->id, InstanceDifficulty::Difficulties::DUNGEON_NORMAL)] = DBC::Structures::MapDifficulty(entry->reset_raid_time, maxPlayers, false);
                else
                    sMapDifficultyMap[Util::MAKE_PAIR32(entry->id, InstanceDifficulty::Difficulties::DUNGEON_HEROIC)] = DBC::Structures::MapDifficulty(entry->reset_heroic_tim, maxPlayers, false);
            }
        }
    }
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAuctionHouseStore, dbc_path, "AuctionHouse.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemRandomSuffixStore, dbc_path, "ItemRandomSuffix.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtCombatRatingsStore, dbc_path, "gtCombatRatings.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChatChannelsStore, dbc_path, "ChatChannels.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sDurabilityQualityStore, dbc_path, "DurabilityQuality.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sDurabilityCostsStore, dbc_path, "DurabilityCosts.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sBankBagSlotPricesStore, dbc_path, "BankBagSlotPrices.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sStableSlotPricesStore, dbc_path, "StableSlotPrices.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritStore, dbc_path, "gtChanceToMeleeCrit.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritBaseStore, dbc_path, "gtChanceToMeleeCritBase.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritStore, dbc_path, "gtChanceToSpellCrit.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritBaseStore, dbc_path, "gtChanceToSpellCritBase.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtRegenMPPerSptStore, dbc_path, "gtRegenMPPerSpt.dbc");     //loaded but not used
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTRegenMPStore, dbc_path, "gtOCTRegenMP.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtRegenHPPerSptStore, dbc_path, "gtRegenHPPerSpt.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTRegenHPStore, dbc_path, "gtOCTRegenHP.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAreaTriggerStore, dbc_path, "AreaTrigger.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMailTemplateStore, dbc_path, "MailTemplate.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWMOAreaTableStore, dbc_path, "WMOAreaTable.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSummonPropertiesStore, dbc_path, "SummonProperties.dbc");


    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLiquidTypeStore, dbc_path, "LiquidType.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWorldMapAreaStore, dbc_path, "WorldMapArea.dbc");


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
#endif

#ifdef AE_WOTLK
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
    {
        for (uint32_t i = 1; i < sTaxiPathStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
            {
                sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->id, entry->price);
            }
        }

        uint32_t pathCount = sTaxiPathStore.GetNumRows();

        // Calculate path nodes count
        std::vector<uint32_t> pathLength;
        pathLength.resize(pathCount);                           // 0 and some other indexes not used
        for (uint32_t i = 0; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            {
                if (pathLength[entry->pathId] < entry->NodeIndex + 1)
                    pathLength[entry->pathId] = entry->NodeIndex + 1;
            }
        }

        // Set path length
        sTaxiPathNodesByPath.resize(pathCount);                 // 0 and some other indexes not used
        for (uint32 i = 1; i < sTaxiPathNodesByPath.size(); ++i)
            sTaxiPathNodesByPath[i].resize(pathLength[i]);

        // fill data
        for (uint32_t i = 0; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            {
                sTaxiPathNodesByPath[entry->pathId][entry->NodeIndex] = entry;
            }
        }
    }
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureDisplayInfoStore, dbc_path, "CreatureDisplayInfo.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureDisplayInfoExtraStore, dbc_path, "CreatureDisplayInfoExtra.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureModelDataStore, dbc_path, "CreatureModelData.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureSpellDataStore, dbc_path, "CreatureSpellData.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureFamilyStore, dbc_path, "CreatureFamily.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrRacesStore, dbc_path, "ChrRaces.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrClassesStore, dbc_path, "ChrClasses.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMapStore, dbc_path, "Map.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMapDifficultyStore, dbc_path, "MapDifficulty.dbc");
    {
        for (uint32_t i = 0; i < sMapDifficultyStore.GetNumRows(); ++i)
        {
            if (auto entry = sMapDifficultyStore.LookupEntry(i))
                sMapDifficultyMap[Util::MAKE_PAIR32(static_cast<uint16_t>(entry->MapID), static_cast<uint16_t>(entry->Difficulty))] = DBC::Structures::MapDifficulty(entry->RaidDuration, entry->MaxPlayers, entry->Message[0] != '\0');
        }
    }
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
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sDungeonEncounterStore, dbc_path, "DungeonEncounter.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLiquidTypeStore, dbc_path, "LiquidType.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sVehicleStore, dbc_path, "Vehicle.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sVehicleSeatStore, dbc_path, "VehicleSeat.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWorldMapAreaStore, dbc_path, "WorldMapArea.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTotemCategoryStore, dbc_path, "TotemCategory.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTransportAnimationStore, dbc_path, "TransportAnimation.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTransportRotationStore, dbc_path, "TransportRotation.dbc");

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
#endif

#ifdef AE_CATA
bool LoadDBCs()
{
    uint32 available_dbc_locales = 0xFFFFFFFF;
    DBC::StoreProblemList bad_dbc_files;
    std::string dbc_path = sWorld.settings.server.dataDir + "dbc/";

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMountCapabilityStore, dbc_path, "MountCapability.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMountTypeStore, dbc_path, "MountType.dbc");

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWorldMapOverlayStore, dbc_path, "WorldMapOverlay.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAchievementCriteriaStore, dbc_path, "Achievement_Criteria.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAchievementStore, dbc_path, "Achievement.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCharTitlesStore, dbc_path, "CharTitles.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCurrencyTypesStore, dbc_path, "CurrencyTypes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sBarberShopStyleStore, dbc_path, "BarberShopStyle.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sBannedAddOnsStore, dbc_path, "BannedAddOns.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCharStartOutfitStore, dbc_path, "CharStartOutfit.dbc");
    for (uint32_t i = 0; i < sCharStartOutfitStore.GetNumRows(); ++i)
        if (DBC::Structures::CharStartOutfitEntry const* outfit = sCharStartOutfitStore.LookupEntry(i))
            sCharStartOutfitMap[outfit->Race | (outfit->Class << 8) | (outfit->Gender << 16)] = outfit;

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
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentStore, dbc_path, "Talent.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentTabStore, dbc_path, "TalentTab.dbc");
    {
        std::map< uint32, uint32 > InspectTalentTabPos;
        std::map< uint32, uint32 > InspectTalentTabSize;
        std::map< uint32, uint32 > InspectTalentTabBit;

        uint32 talent_max_rank;
        uint32 talent_pos;
        uint32 talent_class;

        for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
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
            for (uint32 j = 5; j > 0; --j)
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

        for (uint32 i = 0; i < sTalentTabStore.GetNumRows(); ++i)
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

            for (std::map<uint32, uint32>::iterator itr = InspectTalentTabBit.begin(); itr != InspectTalentTabBit.end(); ++itr)
            {
                uint32 talent_id = itr->first & 0xFFFF;
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

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentTreePrimarySpellsStore, dbc_path, "TalentTreePrimarySpells.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellCastTimesStore, dbc_path, "SpellCastTimes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellDifficultyStore, dbc_path, "SpellDifficulty.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRadiusStore, dbc_path, "SpellRadius.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRangeStore, dbc_path, "SpellRange.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRuneCostStore, dbc_path, "SpellRuneCost.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellDurationStore, dbc_path, "SpellDuration.dbc");

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellAuraOptionsStore, dbc_path, "SpellAuraOptions.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellAuraRestrictionsStore, dbc_path, "SpellAuraRestrictions.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellCastingRequirementsStore, dbc_path, "SpellCastingRequirements.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellCategoriesStore, dbc_path, "SpellCategories.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellClassOptionsStore, dbc_path, "SpellClassOptions.dbc");
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
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellEffectStore, dbc_path, "SpellEffect.dbc");

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
    {
        for (uint32_t i = 1; i < sTaxiPathStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
            {
                sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->id, entry->price);
            }
        }

        uint32_t pathCount = sTaxiPathStore.GetNumRows();
        DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiPathNodeStore, dbc_path, "TaxiPathNode.dbc");
        // Calculate path nodes count
        std::vector<uint32_t> pathLength;
        pathLength.resize(pathCount);                           // 0 and some other indexes not used
        for (uint32_t i = 0; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            {
                if (pathLength[entry->pathId] < entry->NodeIndex + 1)
                    pathLength[entry->pathId] = entry->NodeIndex + 1;
            }
        }

        // Set path length
        sTaxiPathNodesByPath.resize(pathCount);                 // 0 and some other indexes not used
        for (uint32 i = 1; i < sTaxiPathNodesByPath.size(); ++i)
            sTaxiPathNodesByPath[i].resize(pathLength[i]);

        // fill data
        for (uint32_t i = 0; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            {
                sTaxiPathNodesByPath[entry->pathId][entry->NodeIndex] = entry;
            }
        }
    }
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTotemCategoryStore, dbc_path, "TotemCategory.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureSpellDataStore, dbc_path, "CreatureSpellData.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureFamilyStore, dbc_path, "CreatureFamily.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrRacesStore, dbc_path, "ChrRaces.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrClassesStore, dbc_path, "ChrClasses.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMapStore, dbc_path, "Map.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMapDifficultyStore, dbc_path, "MapDifficulty.dbc");
    {
        for (uint32_t i = 0; i < sMapDifficultyStore.GetNumRows(); ++i)
        {
            if (auto entry = sMapDifficultyStore.LookupEntry(i))
                sMapDifficultyMap[Util::MAKE_PAIR32(entry->MapID, entry->Difficulty)] = DBC::Structures::MapDifficulty(entry->RaidDuration, entry->MaxPlayers, entry->Message[0] != '\0');
        }
    }
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sHolidaysStore, dbc_path, "Holidays.dbc");       //loaded but not used
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAuctionHouseStore, dbc_path, "AuctionHouse.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemRandomSuffixStore, dbc_path, "ItemRandomSuffix.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemReforgeStore, dbc_path, "ItemReforge.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtCombatRatingsStore, dbc_path, "gtCombatRatings.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTBaseHPByClassStore, dbc_path, "gtOCTBaseHPByClass.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTBaseMPByClassStore, dbc_path, "gtOCTBaseMPByClass.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChatChannelsStore, dbc_path, "ChatChannels.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureDisplayInfoStore, dbc_path, "CreatureDisplayInfo.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureModelDataStore, dbc_path, "CreatureModelData.dbc");
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
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTClassCombatRatingScalarStore, dbc_path, "gtOCTClassCombatRatingScalar.dbc");
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
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sNumTalentsAtLevel, dbc_path, "NumTalentsAtLevel.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sPhaseStore, dbc_path, "Phase.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLFGDungeonStore, dbc_path, "LFGDungeons.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sDungeonEncounterStore, dbc_path, "DungeonEncounter.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLiquidTypeStore, dbc_path, "LiquidType.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sVehicleStore, dbc_path, "Vehicle.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sVehicleSeatStore, dbc_path, "VehicleSeat.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWorldMapAreaStore, dbc_path, "WorldMapArea.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTransportAnimationStore, dbc_path, "TransportAnimation.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTransportRotationStore, dbc_path, "TransportRotation.dbc");

    MapManagement::AreaManagement::AreaStorage::Initialise(&sAreaStore);
    auto area_map_collection = MapManagement::AreaManagement::AreaStorage::GetMapCollection();
    for (uint32 i = 0; i < sMapStore.GetNumRows(); ++i)
    {
        auto map_object = sMapStore.LookupEntry(i);
        if (map_object == nullptr)
            continue;

        area_map_collection->insert(std::pair<uint32, uint32>(map_object->id, map_object->linked_zone));
    }

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrPowerTypesEntry, dbc_path, "ChrClassesXPowerTypes.dbc");

    // Initialize power index array
    for (uint8_t i = 0; i < MAX_PLAYER_CLASSES; ++i)
    {
        for (uint8_t power = POWER_TYPE_MANA; power < TOTAL_PLAYER_POWER_TYPES; ++power)
            powerIndexByClass[i][power] = TOTAL_PLAYER_POWER_TYPES;
    }

    // Insert data into the array
    for (uint32_t i = 0; i < sChrPowerTypesEntry.GetNumRows(); ++i)
    {
        const auto powerEntry = sChrPowerTypesEntry.LookupEntry(i);
        if (powerEntry != nullptr)
        {
            uint8_t index = 1;
            for (uint8_t power = POWER_TYPE_MANA; power < TOTAL_PLAYER_POWER_TYPES; ++power)
            {
                if (powerIndexByClass[powerEntry->classId][power] != TOTAL_PLAYER_POWER_TYPES)
                    ++index;
            }

            powerIndexByClass[powerEntry->classId][powerEntry->power] = index;
        }
    }

    return true;
}
#endif

#ifdef AE_MOP
bool LoadDBCs()
{
    uint32 available_dbc_locales = 0xFFFFFFFF;
    DBC::StoreProblemList bad_dbc_files;
    std::string dbc_path = sWorld.settings.server.dataDir + "dbc/";

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMountCapabilityStore, dbc_path, "MountCapability.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMountTypeStore, dbc_path, "MountType.dbc");

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWorldMapOverlayStore, dbc_path, "WorldMapOverlay.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAchievementCriteriaStore, dbc_path, "Achievement_Criteria.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAchievementStore, dbc_path, "Achievement.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCharTitlesStore, dbc_path, "CharTitles.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCurrencyTypesStore, dbc_path, "CurrencyTypes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sBarberShopStyleStore, dbc_path, "BarberShopStyle.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sBannedAddOnsStore, dbc_path, "BannedAddOns.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCharStartOutfitStore, dbc_path, "CharStartOutfit.dbc");
    for (uint32_t i = 0; i < sCharStartOutfitStore.GetNumRows(); ++i)
        if (DBC::Structures::CharStartOutfitEntry const* outfit = sCharStartOutfitStore.LookupEntry(i))
            sCharStartOutfitMap[outfit->Race | (outfit->Class << 8) | (outfit->Gender << 16)] = outfit;

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
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentStore, dbc_path, "Talent.dbc");
    //DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentTabStore, dbc_path, "TalentTab.dbc");
    //{
    //    std::map< uint32, uint32 > InspectTalentTabPos;
    //    std::map< uint32, uint32 > InspectTalentTabSize;
    //    std::map< uint32, uint32 > InspectTalentTabBit;

    //    uint32 talent_max_rank;
    //    uint32 talent_pos;
    //    uint32 talent_class;

    //    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    //    {
    //        auto talent_info = sTalentStore.LookupEntry(i);
    //        if (talent_info == nullptr)
    //            continue;

    //        // Don't add invalid talents or Hunter Pet talents (trees 409, 410 and 411) to the inspect table
    //        if (talent_info->TalentTree == 409 || talent_info->TalentTree == 410 || talent_info->TalentTree == 411)
    //            continue;

    //        auto talent_tab = sTalentTabStore.LookupEntry(talent_info->TalentTree);
    //        if (talent_tab == nullptr)
    //            continue;

    //        talent_max_rank = 0;
    //        for (uint32 j = 5; j > 0; --j)
    //        {
    //            if (talent_info->RankID[j - 1])
    //            {
    //                talent_max_rank = j;
    //                break;
    //            }
    //        }

    //        InspectTalentTabBit[(talent_info->Row << 24) + (talent_info->Col << 16) + talent_info->TalentID] = talent_max_rank;
    //        InspectTalentTabSize[talent_info->TalentTree] += talent_max_rank;
    //    }

    //    for (uint32 i = 0; i < sTalentTabStore.GetNumRows(); ++i)
    //    {
    //        auto talent_tab = sTalentTabStore.LookupEntry(i);
    //        if (talent_tab == nullptr)
    //            continue;

    //        if (talent_tab->ClassMask == 0)
    //            continue;

    //        talent_pos = 0;

    //        for (talent_class = 0; talent_class < 12; ++talent_class)
    //        {
    //            if (talent_tab->ClassMask & (1 << talent_class))
    //                break;
    //        }

    //        if (talent_class > 0 && talent_class < 12)
    //            InspectTalentTabPages[talent_class][talent_tab->TabPage] = talent_tab->TalentTabID;

    //        for (std::map<uint32, uint32>::iterator itr = InspectTalentTabBit.begin(); itr != InspectTalentTabBit.end(); ++itr)
    //        {
    //            uint32 talent_id = itr->first & 0xFFFF;
    //            auto talent_info = sTalentStore.LookupEntry(talent_id);
    //            if (talent_info == nullptr)
    //                continue;

    //            if (talent_info->TalentTree != talent_tab->TalentTabID)
    //                continue;

    //            InspectTalentTabPos[talent_id] = talent_pos;
    //            talent_pos += itr->second;
    //        }
    //    }
    //}

    //DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTalentTreePrimarySpellsStore, dbc_path, "TalentTreePrimarySpells.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellCastTimesStore, dbc_path, "SpellCastTimes.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellMiscStore, dbc_path, "SpellMisc.dbc");
    //DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellDifficultyStore, dbc_path, "SpellDifficulty.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRadiusStore, dbc_path, "SpellRadius.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRangeStore, dbc_path, "SpellRange.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellRuneCostStore, dbc_path, "SpellRuneCost.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellDurationStore, dbc_path, "SpellDuration.dbc");

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellAuraOptionsStore, dbc_path, "SpellAuraOptions.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellAuraRestrictionsStore, dbc_path, "SpellAuraRestrictions.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellCastingRequirementsStore, dbc_path, "SpellCastingRequirements.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellCategoriesStore, dbc_path, "SpellCategories.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellClassOptionsStore, dbc_path, "SpellClassOptions.dbc");
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
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sSpellEffectStore, dbc_path, "SpellEffect.dbc");

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
    {
        for (uint32_t i = 1; i < sTaxiPathStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
            {
                sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->id, entry->price);
            }
        }

        uint32_t pathCount = sTaxiPathStore.GetNumRows();
        DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTaxiPathNodeStore, dbc_path, "TaxiPathNode.dbc");
        // Calculate path nodes count
        std::vector<uint32_t> pathLength;
        pathLength.resize(pathCount);                           // 0 and some other indexes not used
        for (uint32_t i = 0; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            {
                if (pathLength[entry->pathId] < entry->NodeIndex + 1)
                    pathLength[entry->pathId] = entry->NodeIndex + 1;
            }
        }

        // Set path length
        sTaxiPathNodesByPath.resize(pathCount);                 // 0 and some other indexes not used
        for (uint32 i = 1; i < sTaxiPathNodesByPath.size(); ++i)
            sTaxiPathNodesByPath[i].resize(pathLength[i]);

        // fill data
        for (uint32_t i = 0; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            {
                sTaxiPathNodesByPath[entry->pathId][entry->NodeIndex] = entry;
            }
        }
    }
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTotemCategoryStore, dbc_path, "TotemCategory.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureSpellDataStore, dbc_path, "CreatureSpellData.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sCreatureFamilyStore, dbc_path, "CreatureFamily.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrRacesStore, dbc_path, "ChrRaces.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrClassesStore, dbc_path, "ChrClasses.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMapStore, dbc_path, "Map.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sMapDifficultyStore, dbc_path, "MapDifficulty.dbc");
    {
        for (uint32_t i = 0; i < sMapDifficultyStore.GetNumRows(); ++i)
        {
            if (auto entry = sMapDifficultyStore.LookupEntry(i))
                sMapDifficultyMap[Util::MAKE_PAIR32(entry->MapID, entry->Difficulty)] = DBC::Structures::MapDifficulty(entry->RaidDuration, entry->MaxPlayers, entry->Message[0] != '\0');
        }
    }
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sHolidaysStore, dbc_path, "Holidays.dbc");       //loaded but not used
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sAuctionHouseStore, dbc_path, "AuctionHouse.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sItemRandomSuffixStore, dbc_path, "ItemRandomSuffix.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtCombatRatingsStore, dbc_path, "gtCombatRatings.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTBaseHPByClassStore, dbc_path, "gtOCTBaseHPByClass.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTBaseMPByClassStore, dbc_path, "gtOCTBaseMPByClass.dbc");
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
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTClassCombatRatingScalarStore, dbc_path, "gtOCTClassCombatRatingScalar.dbc");
    //DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sGtOCTRegenMPStore, dbc_path, "gtOCTRegenMP.dbc");
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
    //DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sNumTalentsAtLevel, dbc_path, "NumTalentsAtLevel.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sPhaseStore, dbc_path, "Phase.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLFGDungeonStore, dbc_path, "LFGDungeons.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sDungeonEncounterStore, dbc_path, "DungeonEncounter.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sLiquidTypeStore, dbc_path, "LiquidType.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sVehicleStore, dbc_path, "Vehicle.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sVehicleSeatStore, dbc_path, "VehicleSeat.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sWorldMapAreaStore, dbc_path, "WorldMapArea.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTransportAnimationStore, dbc_path, "TransportAnimation.dbc");
    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sTransportRotationStore, dbc_path, "TransportRotation.dbc");

    MapManagement::AreaManagement::AreaStorage::Initialise(&sAreaStore);
    auto area_map_collection = MapManagement::AreaManagement::AreaStorage::GetMapCollection();
    for (uint32 i = 0; i < sMapStore.GetNumRows(); ++i)
    {
        auto map_object = sMapStore.LookupEntry(i);
        if (map_object == nullptr)
            continue;

        area_map_collection->insert(std::pair<uint32, uint32>(map_object->id, map_object->linked_zone));
    }

    DBC::LoadDBC(available_dbc_locales, bad_dbc_files, sChrPowerTypesEntry, dbc_path, "ChrClassesXPowerTypes.dbc");

    // Initialize power index array
    for (uint8_t i = 0; i < MAX_PLAYER_CLASSES; ++i)
    {
        for (uint8_t power = POWER_TYPE_MANA; power < TOTAL_PLAYER_POWER_TYPES; ++power)
            powerIndexByClass[i][power] = TOTAL_PLAYER_POWER_TYPES;
    }

    // Insert data into the array
    for (uint32_t i = 0; i < sChrPowerTypesEntry.GetNumRows(); ++i)
    {
        const auto powerEntry = sChrPowerTypesEntry.LookupEntry(i);
        if (powerEntry != nullptr)
        {
            uint8_t index = 1;
            for (uint8_t power = POWER_TYPE_MANA; power < TOTAL_PLAYER_POWER_TYPES; ++power)
            {
                if (powerIndexByClass[powerEntry->classId][power] != TOTAL_PLAYER_POWER_TYPES)
                    ++index;
            }

            powerIndexByClass[powerEntry->classId][powerEntry->power] = index;
        }
    }

    return true;
}
#endif


#if VERSION_STRING >= Cata
DBC::Structures::SpellEffectEntry const* GetSpellEffectEntry(uint32_t spellId, uint8_t effect)
{
    DBC::Structures::SpellEffectMap::const_iterator itr = sSpellEffectMap.find(spellId);
    if (itr == sSpellEffectMap.end())
        return nullptr;

    return itr->second.effects[effect];
}

uint8_t getPowerIndexByClass(uint8_t playerClass, uint8_t powerType)
{
    return powerIndexByClass[playerClass][powerType];
}
#endif

#if VERSION_STRING >= WotLK
DBC::Structures::MapDifficulty const* getDownscaledMapDifficultyData(uint32_t mapId, InstanceDifficulty::Difficulties& difficulty)
{
    uint32_t tmpDiff = difficulty;
    DBC::Structures::MapDifficulty const* mapDiff = getMapDifficultyData(mapId, InstanceDifficulty::Difficulties(tmpDiff));
    if (!mapDiff)
    {
        if (tmpDiff > 1) // heroic, downscale to normal
            tmpDiff -= 2;
        else
            tmpDiff -= 1;   // any non-normal mode for raids like tbc (only one mode)

        // pull new data
        mapDiff = getMapDifficultyData(mapId, InstanceDifficulty::Difficulties(tmpDiff)); // we are 10 normal or 25 normal
        if (!mapDiff)
        {
            tmpDiff -= 1;
            mapDiff = getMapDifficultyData(mapId, InstanceDifficulty::Difficulties(tmpDiff)); // 10 normal
        }
    }

    difficulty = InstanceDifficulty::Difficulties(tmpDiff);
    return mapDiff;
}
#endif

DBC::Structures::WMOAreaTableEntry const* GetWMOAreaTableEntryByTriple(int32_t root_id, int32_t adt_id, int32_t group_id)
{
    auto iter = sWMOAreaInfoByTripple.find(WMOAreaTableTripple(root_id, adt_id, group_id));
    if (iter == sWMOAreaInfoByTripple.end())
        return nullptr;
    return iter->second;
}

DBC::Structures::CharStartOutfitEntry const* getStartOutfitByRaceClass(uint8_t race, uint8_t class_, uint8_t gender)
{
    const auto itr = sCharStartOutfitMap.find(race | (class_ << 8) | (gender << 16));
    if (itr != sCharStartOutfitMap.end())
        return itr->second;

    return nullptr;
}

DBC::Structures::MapDifficulty const* getMapDifficultyData(uint32_t mapId, InstanceDifficulty::Difficulties difficulty)
{
    MapDifficultyMap::const_iterator itr = sMapDifficultyMap.find(Util::MAKE_PAIR32(mapId, difficulty));
    return itr != sMapDifficultyMap.end() ? &itr->second : nullptr;
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

uint32_t getLiquidFlags(uint32_t liquidType)
{
    if (DBC::Structures::LiquidTypeEntry const* liq = sLiquidTypeStore.LookupEntry(liquidType))
        return 1 << liq->Type;

    return 0;
}
