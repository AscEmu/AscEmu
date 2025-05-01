/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WDBStores.hpp"

#include "AEVersion.hpp"
#include "Server/World.h"
#include "WDBGlobals.hpp"
#include "WDBStructures.hpp"
#include "Logging/Logger.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Random.hpp"
#if VERSION_STRING >= Cata
    #include "Objects/Units/Players/PlayerDefines.hpp"
    #include "Spell/SpellAura.hpp"
#endif

float SERVER_DECL GetRadius(WDB::Structures::SpellRadiusEntry const* radius)
{
    if (radius == nullptr)
        return 0;

    return radius->radius_min;
}

uint32_t SERVER_DECL GetCastTime(WDB::Structures::SpellCastTimesEntry const* time)
{
    if (time == nullptr)
        return 0;

    return time->CastTime;
}

uint32_t SERVER_DECL GetDuration(WDB::Structures::SpellDurationEntry const* dur)
{
    if (dur == nullptr)
        return 0;
    return dur->Duration1;
}

typedef std::map<WMOAreaTableTripple, WDB::Structures::WMOAreaTableEntry const*> WMOAreaInfoByTripple;

struct NameGenData
{
    std::string name;
    uint32_t type;
};

std::vector<NameGenData> _namegenData[3];

SERVER_DECL WDB::WDBContainer<WDB::Structures::AreaTableEntry> sAreaStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::AreaTriggerEntry> sAreaTriggerStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::AuctionHouseEntry> sAuctionHouseStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::BankBagSlotPrices> sBankBagSlotPricesStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::ChatChannelsEntry> sChatChannelsStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::CharStartOutfitEntry> sCharStartOutfitStore;
std::map<uint32_t, WDB::Structures::CharStartOutfitEntry const*> sCharStartOutfitMap;

SERVER_DECL WDB::WDBContainer<WDB::Structures::ChrClassesEntry> sChrClassesStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::ChrRacesEntry> sChrRacesStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::CreatureDisplayInfoEntry> sCreatureDisplayInfoStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::CreatureModelDataEntry> sCreatureModelDataStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::CreatureSpellDataEntry> sCreatureSpellDataStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::CreatureFamilyEntry> sCreatureFamilyStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::DurabilityCostsEntry> sDurabilityCostsStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::DurabilityQualityEntry> sDurabilityQualityStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::EmotesTextEntry> sEmotesTextStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::FactionEntry> sFactionStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::FactionTemplateEntry> sFactionTemplateStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemSetEntry> sItemSetStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemRandomPropertiesEntry> sItemRandomPropertiesStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::LFGDungeonEntry> sLFGDungeonStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::LiquidTypeEntry> sLiquidTypeStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::LockEntry> sLockStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::MailTemplateEntry> sMailTemplateStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::MapEntry> sMapStore;
MapDifficultyMap sMapDifficultyMap;

SERVER_DECL WDB::WDBContainer<WDB::Structures::NameGenEntry> sNameGenStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::SkillLineAbilityEntry> sSkillLineAbilityStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SkillLineEntry> sSkillLineStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellEntry> sSpellStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellCastTimesEntry> sSpellCastTimesStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellDurationEntry> sSpellDurationStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellItemEnchantmentEntry> sSpellItemEnchantmentStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellRadiusEntry> sSpellRadiusStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellRangeEntry> sSpellRangeStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellShapeshiftFormEntry> sSpellShapeshiftFormStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::TalentEntry> sTalentStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::TalentTabEntry> sTalentTabStore;
static uint32_t InspectTalentTabPages[12][3];
SERVER_DECL WDB::WDBContainer<WDB::Structures::TaxiNodesEntry> sTaxiNodesStore;
TaxiPathSetBySource sTaxiPathSetBySource;
SERVER_DECL WDB::WDBContainer<WDB::Structures::TaxiPathEntry> sTaxiPathStore;
TaxiPathNodesByPath sTaxiPathNodesByPath;
SERVER_DECL WDB::WDBContainer<WDB::Structures::TaxiPathNodeEntry> sTaxiPathNodeStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::TransportAnimationEntry> sTransportAnimationStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::WMOAreaTableEntry> sWMOAreaTableStore;
static WMOAreaInfoByTripple sWMOAreaInfoByTripple;
SERVER_DECL WDB::WDBContainer<WDB::Structures::WorldMapOverlayEntry> sWorldMapOverlayStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::GtChanceToMeleeCritEntry> sGtChanceToMeleeCritStore; // todo: available for versions > Classic
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore; // todo: available for versions > Classic
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtChanceToSpellCritEntry> sGtChanceToSpellCritStore; // todo: available for versions > Classic
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore; // todo: available for versions > Classic
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtCombatRatingsEntry> sGtCombatRatingsStore; // todo: available for versions > Classic
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTRegenMPEntry> sGtOCTRegenMPStore; // todo: available for versions > Classic
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtRegenMPPerSptEntry> sGtRegenMPPerSptStore; // todo: available for versions > Classic
SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemRandomSuffixEntry> sItemRandomSuffixStore; // todo: available for versions > Classic
SERVER_DECL WDB::WDBContainer<WDB::Structures::SummonPropertiesEntry> sSummonPropertiesStore; // todo: available for versions > Classic
SERVER_DECL WDB::WDBContainer<WDB::Structures::VehicleEntry> sVehicleStore; // todo: available for versions > WotLK
SERVER_DECL WDB::WDBContainer<WDB::Structures::VehicleSeatEntry> sVehicleSeatStore; // todo: available for versions > WotLK

SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemEntry> sItemStore; // todo: available for versions > Classic
SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemExtendedCostEntry> sItemExtendedCostStore; // todo: available for versions > Classic
#if VERSION_STRING < Cata
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTRegenHPEntry> sGtOCTRegenHPStore; // todo: available for versions > Classic
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtRegenHPPerSptEntry> sGtRegenHPPerSptStore; // todo: available for versions > Classic
SERVER_DECL WDB::WDBContainer<WDB::Structures::StableSlotPrices> sStableSlotPricesStore;
#endif

#ifdef AE_TBC
SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemDisplayInfo> sItemDisplayInfoStore;
#endif

#if VERSION_STRING >= TBC
SERVER_DECL WDB::WDBContainer<WDB::Structures::CharTitlesEntry> sCharTitlesStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::GemPropertiesEntry> sGemPropertiesStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::TotemCategoryEntry> sTotemCategoryStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::WorldMapAreaEntry> sWorldMapAreaStore;
#endif

#if VERSION_STRING >= WotLK
SERVER_DECL WDB::WDBContainer<WDB::Structures::AchievementEntry> sAchievementStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::AchievementCriteriaEntry> sAchievementCriteriaStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::AreaGroupEntry> sAreaGroupStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::BarberShopStyleEntry> sBarberShopStyleStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::CurrencyTypesEntry> sCurrencyTypesStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::DungeonEncounterEntry> sDungeonEncounterStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::TransportRotationEntry> sTransportRotationStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::GlyphPropertiesEntry> sGlyphPropertiesStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::GlyphSlotEntry> sGlyphSlotStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtBarberShopCostBaseEntry> sBarberShopCostBaseStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::HolidaysEntry> sHolidaysStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemLimitCategoryEntry> sItemLimitCategoryStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::MapDifficultyEntry> sMapDifficultyStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::QuestXP> sQuestXPStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::ScalingStatDistributionEntry> sScalingStatDistributionStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::ScalingStatValuesEntry> sScalingStatValuesStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellDifficultyEntry> sSpellDifficultyStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellRuneCostEntry> sSpellRuneCostStore;
#endif

#if VERSION_STRING >= Cata
SERVER_DECL WDB::WDBContainer<WDB::Structures::BannedAddOnsEntry> sBannedAddOnsStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::ChrPowerTypesEntry> sChrPowerTypesEntry;
uint8_t powerIndexByClass[MAX_PLAYER_CLASSES][TOTAL_PLAYER_POWER_TYPES];

SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTBaseHPByClassEntry> sGtOCTBaseHPByClassStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTBaseMPByClassEntry> sGtOCTBaseMPByClassStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTClassCombatRatingScalarEntry> sGtOCTClassCombatRatingScalarStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::GuildPerkSpellsEntry> sGuildPerkSpellsStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::EmotesEntry> sEmotesStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemCurrencyCostEntry> sItemCurrencyCostStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::MountCapabilityEntry> sMountCapabilityStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::MountTypeEntry> sMountTypeStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::NumTalentsAtLevel> sNumTalentsAtLevel;

SERVER_DECL WDB::WDBContainer<WDB::Structures::PhaseEntry> sPhaseStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::QuestSortEntry> sQuestSortStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellAuraOptionsEntry> sSpellAuraOptionsStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellAuraRestrictionsEntry> sSpellAuraRestrictionsStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellCastingRequirementsEntry> sSpellCastingRequirementsStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellCategoriesEntry> sSpellCategoriesStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellClassOptionsEntry> sSpellClassOptionsStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellCooldownsEntry> sSpellCooldownsStore;
WDB::Structures::SpellCategoryStore sSpellCategoryStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellEffectEntry> sSpellEffectStore;
WDB::Structures::SpellEffectMap sSpellEffectMap;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellEquippedItemsEntry> sSpellEquippedItemsStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellInterruptsEntry> sSpellInterruptsStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellLevelsEntry> sSpellLevelsStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellPowerEntry> sSpellPowerStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellScalingEntry> sSpellScalingStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellReagentsEntry> sSpellReagentsStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellShapeshiftEntry> sSpellShapeshiftStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellTargetRestrictionsEntry> sSpellTargetRestrictionsStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellTotemsEntry> sSpellTotemsStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::TalentTreePrimarySpells> sTalentTreePrimarySpellsStore;
#endif

#ifdef AE_CATA
SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemReforgeEntry> sItemReforgeStore;
#endif

#ifdef AE_MOP
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellMiscEntry> sSpellMiscStore;
#endif

bool loadDBCs()
{
    /////////////////////////////////////////////////////////////////////////////////////////
    // Load basic dbcs available in all versions
    uint32_t available_dbc_locales = 0xFFFFFFFF;
    WDB::StoreProblemList bad_dbc_files;
    std::string dbc_path = sWorld.settings.server.dataDir + "dbc/";

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sAreaStore, dbc_path, "AreaTable.dbc");
    MapManagement::AreaManagement::AreaStorage::Initialise(&sAreaStore);

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sAreaTriggerStore, dbc_path, "AreaTrigger.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sAuctionHouseStore, dbc_path, "AuctionHouse.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sBankBagSlotPricesStore, dbc_path, "BankBagSlotPrices.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sChatChannelsStore, dbc_path, "ChatChannels.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sCharStartOutfitStore, dbc_path, "CharStartOutfit.dbc");
    for (uint32_t i = 0; i < sCharStartOutfitStore.getNumRows(); ++i)
        if (WDB::Structures::CharStartOutfitEntry const* outfit = sCharStartOutfitStore.lookupEntry(i))
            sCharStartOutfitMap[outfit->Race | outfit->Class << 8 | outfit->Gender << 16] = outfit;

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sChrClassesStore, dbc_path, "ChrClasses.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sChrRacesStore, dbc_path, "ChrRaces.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sCreatureDisplayInfoStore, dbc_path, "CreatureDisplayInfo.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sCreatureDisplayInfoExtraStore, dbc_path, "CreatureDisplayInfoExtra.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sCreatureModelDataStore, dbc_path, "CreatureModelData.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sCreatureSpellDataStore, dbc_path, "CreatureSpellData.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sCreatureFamilyStore, dbc_path, "CreatureFamily.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sDurabilityCostsStore, dbc_path, "DurabilityCosts.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sDurabilityQualityStore, dbc_path, "DurabilityQuality.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sEmotesTextStore, dbc_path, "EmotesText.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sFactionStore, dbc_path, "Faction.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sFactionTemplateStore, dbc_path, "FactionTemplate.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGameObjectDisplayInfoStore, dbc_path, "GameObjectDisplayInfo.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemSetStore, dbc_path, "ItemSet.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemRandomPropertiesStore, dbc_path, "ItemRandomProperties.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sLFGDungeonStore, dbc_path, "LFGDungeons.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sLiquidTypeStore, dbc_path, "LiquidType.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sLockStore, dbc_path, "Lock.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sMailTemplateStore, dbc_path, "MailTemplate.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sMapStore, dbc_path, "Map.dbc");
    // note: generate map related data
    {
#if VERSION_STRING <= TBC
        // classic & BC has no MapDifficulty.dbc so generate that data
        for (uint32_t i = 0; i < sMapStore.getNumRows(); ++i)
        {
            if (auto entry = sMapStore.lookupEntry(i))
            {
                uint32_t maxPlayers;
                if (entry->getAddon() < 1)
                    maxPlayers = entry->isRaid() ? 40 : 5;
                else
                    maxPlayers = entry->isRaid() ? 25 : 5;

                // Classic has only one Difficulty - Same reset Time - and only 5 or 40 man Raids
                if (!entry->getResetTimeHeroic())
                    sMapDifficultyMap[Util::MAKE_PAIR32(entry->id, InstanceDifficulty::Difficulties::DUNGEON_NORMAL)] = WDB::Structures::MapDifficulty(entry->getResetTimeNormal(), maxPlayers, false);
                else
                    sMapDifficultyMap[Util::MAKE_PAIR32(entry->id, InstanceDifficulty::Difficulties::DUNGEON_HEROIC)] = WDB::Structures::MapDifficulty(entry->getResetTimeHeroic(), maxPlayers, false);
            }
        }
#endif

        // note: This was missing for TBC
        const auto area_map_collection = MapManagement::AreaManagement::AreaStorage::GetMapCollection();
        for (uint32_t i = 0; i < sMapStore.getNumRows(); ++i)
        {
            const auto map_object = sMapStore.lookupEntry(i);
            if (map_object == nullptr)
                continue;

            area_map_collection->insert(std::pair(map_object->id, map_object->linked_zone));
        }
    }

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sNameGenStore, dbc_path, "NameGen.dbc");
    // note: This was missing for TBC, Cata and Mop
    {
        for (uint32_t i = 0; i < sNameGenStore.getNumRows(); ++i)
        {
            const auto name_gen_entry = sNameGenStore.lookupEntry(i);
            if (name_gen_entry == nullptr)
                continue;

            NameGenData nameGenData;
            nameGenData.name = std::string(name_gen_entry->Name);
            nameGenData.type = name_gen_entry->type;
            _namegenData[nameGenData.type].push_back(nameGenData);
        }
    }

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSkillLineAbilityStore, dbc_path, "SkillLineAbility.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSkillLineStore, dbc_path, "SkillLine.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellStore, dbc_path, "Spell.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellCastTimesStore, dbc_path, "SpellCastTimes.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellDurationStore, dbc_path, "SpellDuration.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellItemEnchantmentStore, dbc_path, "SpellItemEnchantment.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellRadiusStore, dbc_path, "SpellRadius.dbc");     // todo handle max and level radius
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellRangeStore, dbc_path, "SpellRange.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellShapeshiftFormStore, dbc_path, "SpellShapeshiftForm.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTalentStore, dbc_path, "Talent.dbc");
#if VERSION_STRING < Mop
    // note: This is not valid for Mop
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTalentTabStore, dbc_path, "TalentTab.dbc");
    {
        std::map<uint32_t, uint32_t> InspectTalentTabPos;
        std::map<uint32_t, uint32_t> InspectTalentTabSize;
        std::map<uint32_t, uint32_t> InspectTalentTabBit;

        uint32_t talent_max_rank;
        uint32_t talent_pos;
        uint32_t talent_class;

        for (uint32_t i = 0; i < sTalentStore.getNumRows(); ++i)
        {
            auto talent_info = sTalentStore.lookupEntry(i);
            if (talent_info == nullptr)
                continue;

            // Don't add invalid talents or Hunter Pet talents (trees 409, 410 and 411) to the inspect table
            if (talent_info->TalentTree == 409 || talent_info->TalentTree == 410 || talent_info->TalentTree == 411)
                continue;

            auto talent_tab = sTalentTabStore.lookupEntry(talent_info->TalentTree);
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

        for (uint32_t i = 0; i < sTalentTabStore.getNumRows(); ++i)
        {
            auto talent_tab = sTalentTabStore.lookupEntry(i);
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
                auto talent_info = sTalentStore.lookupEntry(talent_id);
                if (talent_info == nullptr)
                    continue;

                if (talent_info->TalentTree != talent_tab->TalentTabID)
                    continue;

                InspectTalentTabPos[talent_id] = talent_pos;
                talent_pos += itr->second;
            }
        }
    }
#endif

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTaxiNodesStore, dbc_path, "TaxiNodes.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTaxiPathStore, dbc_path, "TaxiPath.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTaxiPathNodeStore, dbc_path, "TaxiPathNode.dbc");
    // note: Generate path data
    {
        for (uint32_t i = 1; i < sTaxiPathStore.getNumRows(); ++i)
        {
            if (WDB::Structures::TaxiPathEntry const* entry = sTaxiPathStore.lookupEntry(i))
                sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->id, entry->price);
        }

        uint32_t pathCount = sTaxiPathStore.getNumRows();

        // Calculate path nodes count
        std::vector<uint32_t> pathLength;
        pathLength.resize(pathCount);                           // 0 and some other indexes not used
        for (uint32_t i = 0; i < sTaxiPathNodeStore.getNumRows(); ++i)
        {
            if (WDB::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.lookupEntry(i))
            {
                if (pathLength[entry->pathId] < entry->NodeIndex + 1)
                    pathLength[entry->pathId] = entry->NodeIndex + 1;
            }
        }

        // Set path length
        sTaxiPathNodesByPath.resize(pathCount);                 // 0 and some other indexes not used
        for (uint32_t i = 1; i < sTaxiPathNodesByPath.size(); ++i)
            sTaxiPathNodesByPath[i].resize(pathLength[i]);

        // fill data
        for (uint32_t i = 0; i < sTaxiPathNodeStore.getNumRows(); ++i)
        {
            if (WDB::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.lookupEntry(i))
                sTaxiPathNodesByPath[entry->pathId][entry->NodeIndex] = entry;
        }
    }

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTransportAnimationStore, dbc_path, "TransportAnimation.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sWMOAreaTableStore, dbc_path, "WMOAreaTable.dbc");
    {
        for (uint32_t i = 0; i < sWMOAreaTableStore.getNumRows(); ++i)
        {
            if (auto entry = sWMOAreaTableStore.lookupEntry(i))
                sWMOAreaInfoByTripple.insert(WMOAreaInfoByTripple::value_type(WMOAreaTableTripple(entry->rootId, entry->adtId, entry->groupId), entry));
        }
    }

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sWorldMapOverlayStore, dbc_path, "WorldMapOverlay.dbc");

    /////////////////////////////////////////////////////////////////////////////////////////
    // Load single version specific dbcs
#ifdef AE_TBC
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemDisplayInfoStore, dbc_path, "ItemDisplayInfo.dbc");
#endif

#ifdef AE_CATA
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemReforgeStore, dbc_path, "ItemReforge.dbc");
#endif

#ifdef AE_MOP
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellMiscStore, dbc_path, "SpellMisc.dbc");
#endif

    /////////////////////////////////////////////////////////////////////////////////////////
    // Load multi version specific dbcs - WotLK, TBC and/or Classic
#if VERSION_STRING < Cata
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sStableSlotPricesStore, dbc_path, "StableSlotPrices.dbc");

#endif

    /////////////////////////////////////////////////////////////////////////////////////////
    // Load multi version specific dbcs available since TBC
#if VERSION_STRING >= TBC
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sCharTitlesStore, dbc_path, "CharTitles.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGemPropertiesStore, dbc_path, "GemProperties.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritStore, dbc_path, "gtChanceToMeleeCrit.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritBaseStore, dbc_path, "gtChanceToMeleeCritBase.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritStore, dbc_path, "gtChanceToSpellCrit.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritBaseStore, dbc_path, "gtChanceToSpellCritBase.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtCombatRatingsStore, dbc_path, "gtCombatRatings.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtRegenMPPerSptStore, dbc_path, "gtRegenMPPerSpt.dbc");     //loaded but not used
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemRandomSuffixStore, dbc_path, "ItemRandomSuffix.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSummonPropertiesStore, dbc_path, "SummonProperties.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTotemCategoryStore, dbc_path, "TotemCategory.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sWorldMapAreaStore, dbc_path, "WorldMapArea.dbc");

    #if VERSION_STRING < Cata
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtOCTRegenHPStore, dbc_path, "gtOCTRegenHP.dbc");
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtRegenHPPerSptStore, dbc_path, "gtRegenHPPerSpt.dbc");
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemStore, dbc_path, "Item.dbc");
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemExtendedCostStore, dbc_path, "ItemExtendedCost.dbc");
    #else
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemStore, dbc_path, "Item.db2");
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemExtendedCostStore, dbc_path, "ItemExtendedCost.db2");
    #endif

    #if VERSION_STRING < Mop
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtOCTRegenMPStore, dbc_path, "gtOCTRegenMP.dbc");
    #endif
#endif

    /////////////////////////////////////////////////////////////////////////////////////////
    // Load multi version specific dbcs available since WotLK
#if VERSION_STRING >= WotLK
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sAchievementStore, dbc_path, "Achievement.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sAchievementCriteriaStore, dbc_path, "Achievement_Criteria.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sAreaGroupStore, dbc_path, "AreaGroup.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sBarberShopStyleStore, dbc_path, "BarberShopStyle.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sCurrencyTypesStore, dbc_path, "CurrencyTypes.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sDungeonEncounterStore, dbc_path, "DungeonEncounter.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTransportRotationStore, dbc_path, "TransportRotation.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGlyphPropertiesStore, dbc_path, "GlyphProperties.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGlyphSlotStore, dbc_path, "GlyphSlot.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sBarberShopCostBaseStore, dbc_path, "gtBarberShopCostBase.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sHolidaysStore, dbc_path, "Holidays.dbc");       //loaded but not used
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemLimitCategoryStore, dbc_path, "ItemLimitCategory.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sMapDifficultyStore, dbc_path, "MapDifficulty.dbc");
    // note: generate map difficulty map in new storage
    {
        for (uint32_t i = 0; i < sMapDifficultyStore.getNumRows(); ++i)
        {
            if (auto entry = sMapDifficultyStore.lookupEntry(i))
                sMapDifficultyMap[Util::MAKE_PAIR32(static_cast<uint16_t>(entry->MapID), static_cast<uint16_t>(entry->Difficulty))] = WDB::Structures::MapDifficulty(entry->RaidDuration, entry->MaxPlayers, entry->Message[0] != '\0');
        }
    }

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sQuestXPStore, dbc_path, "QuestXP.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sScalingStatDistributionStore, dbc_path, "ScalingStatDistribution.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sScalingStatValuesStore, dbc_path, "ScalingStatValues.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellRuneCostStore, dbc_path, "SpellRuneCost.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sVehicleStore, dbc_path, "Vehicle.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sVehicleSeatStore, dbc_path, "VehicleSeat.dbc");

    #if VERSION_STRING < Mop
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellDifficultyStore, dbc_path, "SpellDifficulty.dbc");
    #endif
#endif

    /////////////////////////////////////////////////////////////////////////////////////////
    // Load multi version specific dbcs available since WotLK
#if VERSION_STRING >= Cata
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sBannedAddOnsStore, dbc_path, "BannedAddOns.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sChrPowerTypesEntry, dbc_path, "ChrClassesXPowerTypes.dbc");
    // note: generate powerIndex into new array
    {
        // Initialize power index array
        for (uint8_t i = 0; i < MAX_PLAYER_CLASSES; ++i)
        {
            for (uint8_t power = POWER_TYPE_MANA; power < TOTAL_PLAYER_POWER_TYPES; ++power)
                powerIndexByClass[i][power] = TOTAL_PLAYER_POWER_TYPES;
        }

        // Insert data into the array
        for (uint32_t i = 0; i < sChrPowerTypesEntry.getNumRows(); ++i)
        {
            const auto powerEntry = sChrPowerTypesEntry.lookupEntry(i);
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
    }

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtOCTBaseHPByClassStore, dbc_path, "gtOCTBaseHPByClass.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtOCTBaseMPByClassStore, dbc_path, "gtOCTBaseMPByClass.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtOCTClassCombatRatingScalarStore, dbc_path, "gtOCTClassCombatRatingScalar.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGuildPerkSpellsStore, dbc_path, "GuildPerkSpells.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sEmotesStore, dbc_path, "Emotes.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemCurrencyCostStore, dbc_path, "ItemCurrencyCost.db2");
    {
        sLogger.debug("Loaded {} rows from ItemCurrencyCost.db2", sItemCurrencyCostStore.getNumRows());
    }
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sMountCapabilityStore, dbc_path, "MountCapability.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sMountTypeStore, dbc_path, "MountType.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sPhaseStore, dbc_path, "Phase.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sQuestSortStore, dbc_path, "QuestSort.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellAuraRestrictionsStore, dbc_path, "SpellAuraRestrictions.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellCastingRequirementsStore, dbc_path, "SpellCastingRequirements.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellCategoriesStore, dbc_path, "SpellCategories.dbc");
    // note: generate spell categories and insert to helper storage
    {
        for (uint32_t i = 1; i < sSpellStore.getNumRows(); ++i)
        {
            if (WDB::Structures::SpellEntry const* spell = sSpellStore.lookupEntry(i))
            {
                if (WDB::Structures::SpellCategoriesEntry const* category = spell->GetSpellCategories())
                    if (uint32_t cat = category->Category)
                        sSpellCategoryStore[cat].insert(i);
            }
        }
    }

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellClassOptionsStore, dbc_path, "SpellClassOptions.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellCooldownsStore, dbc_path, "SpellCooldowns.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellEffectStore, dbc_path, "SpellEffect.dbc");
    // note: generate spell effects and add to helper map
    {
        for (uint32_t i = 1; i < sSpellEffectStore.getNumRows(); ++i)
        {
            if (WDB::Structures::SpellEffectEntry const* spellEffect = sSpellEffectStore.lookupEntry(i))
            {
                sSpellEffectMap[spellEffect->EffectSpellId].effects[spellEffect->EffectIndex] = spellEffect;
            }
        }
    }

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellEquippedItemsStore, dbc_path, "SpellEquippedItems.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellInterruptsStore, dbc_path, "SpellInterrupts.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellLevelsStore, dbc_path, "SpellLevels.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellPowerStore, dbc_path, "SpellPower.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellScalingStore, dbc_path, "SpellScaling.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellShapeshiftStore, dbc_path, "SpellShapeshift.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellTargetRestrictionsStore, dbc_path, "SpellTargetRestrictions.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellTotemsStore, dbc_path, "SpellTotems.dbc");

    #if VERSION_STRING < Mop
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sNumTalentsAtLevel, dbc_path, "NumTalentsAtLevel.dbc");
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTalentTreePrimarySpellsStore, dbc_path, "TalentTreePrimarySpells.dbc");
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellReagentsStore, dbc_path, "SpellReagents.dbc");
    #else
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellReagentsStore, dbc_path, "SpellReagents.db2");
    #endif
#endif

    return true;
}

#if VERSION_STRING >= Cata
WDB::Structures::SpellEffectEntry const* GetSpellEffectEntry(uint32_t spellId, uint8_t effect)
{
    WDB::Structures::SpellEffectMap::const_iterator itr = sSpellEffectMap.find(spellId);
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
WDB::Structures::MapDifficulty const* getDownscaledMapDifficultyData(uint32_t mapId, InstanceDifficulty::Difficulties& difficulty)
{
    uint32_t tmpDiff = difficulty;
    WDB::Structures::MapDifficulty const* mapDiff = getMapDifficultyData(mapId, InstanceDifficulty::Difficulties(tmpDiff));
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

WDB::Structures::WMOAreaTableEntry const* GetWMOAreaTableEntryByTriple(int32_t root_id, int32_t adt_id, int32_t group_id)
{
    auto iter = sWMOAreaInfoByTripple.find(WMOAreaTableTripple(root_id, adt_id, group_id));
    if (iter == sWMOAreaInfoByTripple.end())
        return nullptr;
    return iter->second;
}

WDB::Structures::CharStartOutfitEntry const* getStartOutfitByRaceClass(uint8_t race, uint8_t class_, uint8_t gender)
{
    const auto itr = sCharStartOutfitMap.find(race | (class_ << 8) | (gender << 16));
    if (itr != sCharStartOutfitMap.end())
        return itr->second;

    return nullptr;
}

WDB::Structures::MapDifficulty const* getMapDifficultyData(uint32_t mapId, InstanceDifficulty::Difficulties difficulty)
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
    if (WDB::Structures::LiquidTypeEntry const* liq = sLiquidTypeStore.lookupEntry(liquidType))
        return 1 << liq->Type;

    return 0;
}
