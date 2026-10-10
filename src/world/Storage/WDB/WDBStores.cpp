/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WDBStores.hpp"

#include "AEVersion.hpp"
#include "Server/World.h"
#include "WDBContainer.hpp"
#include "WDBGlobals.hpp"
#include "WDBFormat.hpp"
#include "WDBStructures.hpp"
#if defined(AE_FOREVER)
    #include "WDC5File.hpp"
#endif
#include "Logging/Logger.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Random.hpp"
#if VERSION_STRING >= Cata || defined(AE_FOREVER)
    #include "Objects/Units/Players/PlayerDefines.hpp"
#endif
#if VERSION_STRING >= Cata
    #include "Spell/SpellAura.hpp"
#endif

#include <limits>
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <deque>
#include <initializer_list>
#include <iterator>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

typedef std::map<WMOAreaTableTripple, WDB::Structures::WMOAreaTableEntry const*> WMOAreaInfoByTripple;

struct NameGenData
{
    std::string name;
    uint32_t type;
};

std::vector<NameGenData> _namegenData[3];

std::map<uint32_t, WDB::Structures::CharStartOutfitEntry const*> sCharStartOutfitMap;

SERVER_DECL WDB::WDBContainer<WDB::Structures::GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemSetEntry> sItemSetStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemRandomPropertiesEntry> sItemRandomPropertiesStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::LFGDungeonEntry> sLFGDungeonStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::LiquidTypeEntry> sLiquidTypeStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::LockEntry> sLockStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::MailTemplateEntry> sMailTemplateStore;

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

#if defined(AE_FOREVER)
static std::array<std::array<uint8_t, TOTAL_PLAYER_POWER_TYPES>, MAX_PLAYER_CLASSES> powerIndexByClass;
#endif

SERVER_DECL WDB::WDBContainer<WDB::Structures::TalentEntry> sTalentStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::TalentTabEntry> sTalentTabStore;
static uint32_t InspectTalentTabPages[12][3];
#if VERSION_STRING == Mop
static uint32_t ClassSpecializationTabs[12][4];
#elif defined(AE_FOREVER)
// Copied from MoP as a temporary baseline. Replace with dedicated Forever values once verified.
static uint32_t ClassSpecializationTabs[12][4];
#endif
SERVER_DECL WDB::WDBContainer<WDB::Structures::TaxiNodesEntry> sTaxiNodesStore;
TaxiPathSetBySource sTaxiPathSetBySource;
SERVER_DECL WDB::WDBContainer<WDB::Structures::TaxiPathEntry> sTaxiPathStore;
TaxiPathNodesByPath sTaxiPathNodesByPath;
SERVER_DECL WDB::WDBContainer<WDB::Structures::TaxiPathNodeEntry> sTaxiPathNodeStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::TransportAnimationEntry> sTransportAnimationStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::WMOAreaTableEntry> sWMOAreaTableStore;
static WMOAreaInfoByTripple sWMOAreaInfoByTripple;
#if !defined(AE_FOREVER)
SERVER_DECL WDB::WDBContainer<WDB::Structures::WorldMapOverlayEntry> sWorldMapOverlayStore;
#endif

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
#endif

#ifdef AE_TBC
SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemDisplayInfo> sItemDisplayInfoStore;
#endif

#if VERSION_STRING >= WotLK
SERVER_DECL WDB::WDBContainer<WDB::Structures::AchievementEntry> sAchievementStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::AchievementCriteriaEntry> sAchievementCriteriaStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::CurrencyTypesEntry> sCurrencyTypesStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::DungeonEncounterEntry> sDungeonEncounterStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::TransportRotationEntry> sTransportRotationStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::GlyphPropertiesEntry> sGlyphPropertiesStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::GlyphSlotEntry> sGlyphSlotStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtBarberShopCostBaseEntry> sBarberShopCostBaseStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::HolidaysEntry> sHolidaysStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemLimitCategoryEntry> sItemLimitCategoryStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::QuestXP> sQuestXPStore;
#if defined(AE_FOREVER)
SERVER_DECL ForeverQuestPOIStore sForeverQuestPOIStore;
#endif

SERVER_DECL WDB::WDBContainer<WDB::Structures::ScalingStatDistributionEntry> sScalingStatDistributionStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::ScalingStatValuesEntry> sScalingStatValuesStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellDifficultyEntry> sSpellDifficultyStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellRuneCostEntry> sSpellRuneCostStore;
#endif

#if VERSION_STRING >= Cata
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

#if VERSION_STRING == Mop
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellMiscEntry> sSpellMiscStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::ChrSpecializationEntry> sChrSpecializationStore;
WDB::Structures::SpellPowerMap sSpellPowerMap;
#elif defined(AE_FOREVER)
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellMiscEntry> sSpellMiscStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::ChrSpecializationEntry> sChrSpecializationStore;
WDB::Structures::SpellPowerMap sSpellPowerMap;
#endif

namespace {
#if defined(AE_FOREVER)
    namespace ForeverFormat = WDB::Formats::Forever;

    std::deque<std::string> foreverDb2StringPool;

    char* keepForeverDb2String(std::string_view value)
    {
        if (value.empty())
            return nullptr;

        foreverDb2StringPool.emplace_back(value);
        return foreverDb2StringPool.back().data();
    }

    struct ForeverWDC5Load
    {
        WDB::WDC5File& file;
        WDB::WDC5TableSchema const& format;
    };

    bool loadForeverWDC5(WDB::WDC5File& file, WDB::WDC5TableSchema const& format, WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        std::string error;
        if (file.load(dbcPath + format.filename, format, &error))
        {
            sLogger.info("DB2: {} loaded.", format.filename);
            return true;
        }

        errors.push_back("Forever WDC5: " + error);
        sLogger.failure("DB2: {} failed to load.", format.filename);
        return false;
    }

    bool loadForeverWDC5Optional(WDB::WDC5File& file, WDB::WDC5TableSchema const& format, std::string const& dbcPath)
    {
        std::string error;
        if (file.load(dbcPath + format.filename, format, &error))
        {
            sLogger.info("DB2: {} loaded (optional).", format.filename);
            return true;
        }

        sLogger.warning("DB2: {} not loaded (optional): {}", format.filename, error);
        return false;
    }

    bool loadForeverWDC5Group(std::initializer_list<ForeverWDC5Load> loads, WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        for (ForeverWDC5Load const& load : loads)
            if (!loadForeverWDC5(load.file, load.format, errors, dbcPath))
                return false;

        return true;
    }


    bool loadForeverGenericWDC5(WDB::WDC5File& file, char const* filename, WDB::StoreProblemList& errors, std::string const& dbcPath,
        std::initializer_list<std::pair<uint32_t, uint8_t>> arrays = {})
    {
        std::string error;
        if (file.loadGeneric(dbcPath + filename, arrays, &error))
        {
            sLogger.info("DB2: {} loaded (fields={}, layout=0x{:08X}).", filename, file.getFieldCount(), file.getLayoutHash());
            return true;
        }

        errors.push_back("Forever WDC5: " + error);
        sLogger.failure("DB2: {} failed to load.", filename);
        return false;
    }

    bool loadForeverGenericWDC5Optional(WDB::WDC5File& file, char const* filename, std::string const& dbcPath,
        std::initializer_list<std::pair<uint32_t, uint8_t>> arrays = {})
    {
        std::string error;
        if (file.loadGeneric(dbcPath + filename, arrays, &error))
        {
            sLogger.info("DB2: {} loaded (optional, fields={}, layout=0x{:08X}).", filename, file.getFieldCount(), file.getLayoutHash());
            return true;
        }

        sLogger.warning("DB2: {} not loaded (optional): {}", filename, error);
        return false;
    }

    bool loadForeverModernEmoteStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDB::WDC5File emotesText;
        if (!loadForeverGenericWDC5(emotesText, "EmotesText.db2", errors, dbcPath))
            return false;

        if (emotesText.getFieldCount() != 2)
        {
            errors.push_back("Forever DB2 EmotesText.db2: expected 2 fields, got " + std::to_string(emotesText.getFieldCount()));
            sLogger.failure("DB2: EmotesText.db2 invalid (fields={}, expected=2).", emotesText.getFieldCount());
            return false;
        }

        std::vector<std::pair<uint32_t, WDB::Structures::EmotesTextEntry>> entries;
        entries.reserve(emotesText.getRecordCount());
        for (uint32_t row = 0; row < emotesText.getRecordCount(); ++row)
        {
            WDB::Structures::EmotesTextEntry entry{};
            entry.id = emotesText.getRecordId(row);
            entry.textId[0] = emotesText.getUInt32(row, 1); // EmoteID; field 0 is Name
            entries.emplace_back(entry.id, entry);
        }

        sEmotesTextStore.assignEntries(entries);
        sLogger.info("DB2: EmotesText store ready (entries={}).", entries.size());
        return true;
    }

    bool loadForeverModernQuestStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDB::WDC5File questXP;
        if (!loadForeverGenericWDC5(questXP, "QuestXP.db2", errors, dbcPath, {{0, 10}}))
            return false;

        if (questXP.getFieldCount() != 1)
        {
            errors.push_back("Forever DB2 QuestXP.db2: expected 1 field, got " + std::to_string(questXP.getFieldCount()));
            sLogger.failure("DB2: QuestXP.db2 invalid (fields={}, expected=1).", questXP.getFieldCount());
            return false;
        }

        std::vector<std::pair<uint32_t, WDB::Structures::QuestXP>> entries;
        entries.reserve(questXP.getRecordCount());
        for (uint32_t row = 0; row < questXP.getRecordCount(); ++row)
        {
            WDB::Structures::QuestXP entry{};
            entry.questLevel = questXP.getRecordId(row);
            for (uint8_t i = 0; i < 10; ++i)
                entry.xpIndex[i] = questXP.getUInt16(row, 0, i);
            entries.emplace_back(entry.questLevel, entry);
        }

        sQuestXPStore.assignEntries(entries);
        sLogger.info("DB2: QuestXP store ready (levels={}).", sQuestXPStore.getNumRows());

        sForeverQuestPOIStore.clear();
        WDB::WDC5File questPOIBlob;
        WDB::WDC5File questPOIPoint;
        const bool haveQuestPOIBlob = loadForeverGenericWDC5Optional(questPOIBlob, "QuestPOIBlob.db2", dbcPath);
        const bool haveQuestPOIPoint = loadForeverGenericWDC5Optional(questPOIPoint, "QuestPOIPoint.db2", dbcPath);
        if (haveQuestPOIBlob && haveQuestPOIPoint)
        {
            if (questPOIBlob.getLayoutHash() != 0xFDC814CF || questPOIBlob.getFieldCount() != 10)
                sLogger.warning("DB2: QuestPOIBlob.db2 skipped (fields={}, layout=0x{:08X}, reason=layout mismatch).", questPOIBlob.getFieldCount(), questPOIBlob.getLayoutHash());
            else if (questPOIPoint.getLayoutHash() != 0x5CBBEFE7 || questPOIPoint.getFieldCount() != 4)
                sLogger.warning("DB2: QuestPOIPoint.db2 skipped (fields={}, layout=0x{:08X}, reason=layout mismatch).", questPOIPoint.getFieldCount(), questPOIPoint.getLayoutHash());
            else
            {
                std::unordered_map<uint32_t, std::pair<uint32_t, size_t>> blobsById;
                for (uint32_t row = 0; row < questPOIBlob.getRecordCount(); ++row)
                {
                    ForeverQuestPOIBlobData blob;
                    blob.id = questPOIBlob.getUInt32(row, 0);
                    blob.mapId = questPOIBlob.getUInt16(row, 1);
                    blob.uiMapId = questPOIBlob.getUInt32(row, 2);
                    blob.flags = questPOIBlob.getUInt32(row, 3);
                    blob.numPoints = questPOIBlob.getUInt8(row, 4);
                    blob.questId = questPOIBlob.getUInt32(row, 5);
                    blob.objectiveIndex = questPOIBlob.getInt32(row, 6);
                    blob.objectiveId = questPOIBlob.getUInt32(row, 7);
                    blob.playerConditionId = questPOIBlob.getUInt32(row, 8);
                    blob.navigationPlayerConditionId = questPOIBlob.getUInt32(row, 9);
                    const uint32_t blobId = blob.id;
                    const uint32_t questId = blob.questId;
                    auto& questBlobs = sForeverQuestPOIStore[questId];
                    const size_t blobIndex = questBlobs.size();
                    questBlobs.emplace_back(std::move(blob));
                    blobsById[blobId] = {questId, blobIndex};
                }

                for (uint32_t row = 0; row < questPOIPoint.getRecordCount(); ++row)
                {
                    const uint32_t blobId = questPOIPoint.getParentId(row);
                    const auto itr = blobsById.find(blobId);
                    if (itr == blobsById.end())
                        continue;

                    ForeverQuestPOIPointData point;
                    point.x = questPOIPoint.getInt16(row, 1);
                    point.y = questPOIPoint.getInt16(row, 2);
                    point.z = questPOIPoint.getInt16(row, 3);
                    sForeverQuestPOIStore[itr->second.first][itr->second.second].points.push_back(point);
                }

                uint32_t blobCount = 0;
                uint32_t pointCount = 0;
                for (auto const& [questId, blobs] : sForeverQuestPOIStore)
                {
                    blobCount += static_cast<uint32_t>(blobs.size());
                    for (ForeverQuestPOIBlobData const& blob : blobs)
                        pointCount += static_cast<uint32_t>(blob.points.size());
                }
                sLogger.info("DB2: QuestPOI store ready (quests={}, blobs={}, points={}).", sForeverQuestPOIStore.size(), blobCount, pointCount);
            }
        }
        return true;
    }

    bool loadForeverModernSpellSkillStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        foreverDb2StringPool.clear();

        WDB::WDC5File skillLine, skillLineAbility, spellName, spellAuraOptions, spellAuraRestrictions, spellCastTimes;
        WDB::WDC5File spellCastingRequirements, spellCategories, spellClassOptions, spellCooldowns, spellDuration, spellEffect;
        WDB::WDC5File spellEquippedItems, spellInterrupts, spellLevels, spellMisc, spellPower, spellRadius, spellRange;
        WDB::WDC5File spellReagents, spellScaling, spellShapeshift, spellShapeshiftForm, spellTargetRestrictions, spellTotems;
        WDB::WDC5File spellItemEnchantment, spellXSpellVisual, unitCondition, playerCondition;

        bool ok = true;
        auto load = [&](WDB::WDC5File& file, char const* name)
        {
            bool const loaded = loadForeverGenericWDC5(file, name, errors, dbcPath);
            ok = loaded && ok;
            return loaded;
        };

        auto loadArrays = [&](WDB::WDC5File& file, char const* name, std::initializer_list<std::pair<uint32_t, uint8_t>> arrays)
        {
            bool const loaded = loadForeverGenericWDC5(file, name, errors, dbcPath, arrays);
            ok = loaded && ok;
            return loaded;
        };

        bool const haveSkillLine = load(skillLine, "SkillLine.db2");
        bool const haveSkillLineAbility = loadArrays(skillLineAbility, "SkillLineAbility.db2", {{17, 2}});
        bool const haveSpellName = load(spellName, "SpellName.db2");
        bool const haveAuraOptions = loadArrays(spellAuraOptions, "SpellAuraOptions.db2", {{6, 2}});
        bool const haveAuraRestrictions = load(spellAuraRestrictions, "SpellAuraRestrictions.db2");
        bool const haveCastTimes = load(spellCastTimes, "SpellCastTimes.db2");
        bool const haveCastingRequirements = load(spellCastingRequirements, "SpellCastingRequirements.db2");
        bool const haveCategories = load(spellCategories, "SpellCategories.db2");
        bool const haveClassOptions = loadArrays(spellClassOptions, "SpellClassOptions.db2", {{3, 4}});
        bool const haveCooldowns = load(spellCooldowns, "SpellCooldowns.db2");
        bool const haveDuration = load(spellDuration, "SpellDuration.db2");
        bool const haveEffect = loadArrays(spellEffect, "SpellEffect.db2", {{25, 2}, {26, 2}, {27, 4}, {28, 2}});
        bool const haveEquippedItems = load(spellEquippedItems, "SpellEquippedItems.db2");
        bool const haveInterrupts = loadArrays(spellInterrupts, "SpellInterrupts.db2", {{2, 2}, {3, 2}});
        bool const haveLevels = load(spellLevels, "SpellLevels.db2");
        bool const haveMisc = loadArrays(spellMisc, "SpellMisc.db2", {{0, 17}});
        bool const havePower = load(spellPower, "SpellPower.db2");
        bool const haveRadius = load(spellRadius, "SpellRadius.db2");
        bool const haveRange = loadArrays(spellRange, "SpellRange.db2", {{3, 2}, {4, 2}});
        bool const haveReagents = loadArrays(spellReagents, "SpellReagents.db2", {{1, 8}, {2, 8}, {3, 8}, {4, 8}});
        bool const haveScaling = loadForeverGenericWDC5Optional(spellScaling, "SpellScaling.db2", dbcPath);
        bool const haveShapeshift = loadArrays(spellShapeshift, "SpellShapeshift.db2", {{2, 2}, {3, 2}});
        bool const haveShapeshiftForm = loadArrays(spellShapeshiftForm, "SpellShapeshiftForm.db2", {{9, 8}});
        bool const haveTargetRestrictions = load(spellTargetRestrictions, "SpellTargetRestrictions.db2");
        bool const haveTotems = loadArrays(spellTotems, "SpellTotems.db2", {{1, 2}, {2, 2}});
        bool const haveItemEnchantment = loadArrays(spellItemEnchantment, "SpellItemEnchantment.db2", {{4, 3}, {5, 3}, {6, 3}, {8, 3}});
        bool const haveSpellXSpellVisual = loadForeverGenericWDC5Optional(spellXSpellVisual, "SpellXSpellVisual.db2", dbcPath);
        bool const haveUnitCondition = loadForeverGenericWDC5Optional(unitCondition, "UnitCondition.db2", dbcPath, {{1, 8}, {2, 8}, {3, 8}});
        bool const havePlayerCondition = loadForeverGenericWDC5Optional(playerCondition, "PlayerCondition.db2", dbcPath, {{59, 4}, {60, 4}, {61, 4}, {62, 3}, {63, 3}, {64, 4}, {65, 4}, {66, 4}, {67, 4}, {68, 4}, {69, 4}, {70, 2}, {71, 2}, {72, 4}, {73, 4}, {74, 4}, {75, 4}, {76, 4}, {77, 4}, {78, 4}, {79, 4}, {80, 4}, {81, 6}, {82, 2}, {83, 4}, {84, 4}, {85, 4}});

        auto verifyFields = [&](WDB::WDC5File const& file, char const* name, uint32_t expected)
        {
            if (file.getFieldCount() == expected)
                return true;

            errors.push_back(std::string("Forever DB2 ") + name + ": expected " + std::to_string(expected) + " fields, got " + std::to_string(file.getFieldCount()));
            sLogger.failure("DB2: {} invalid (fields={}, expected={}).", name, file.getFieldCount(), expected);
            ok = false;
            return false;
        };

        if (haveSkillLine && verifyFields(skillLine, "SkillLine.db2", 15))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SkillLineEntry>> entries;
            entries.reserve(skillLine.getRecordCount());
            for (uint32_t row = 0; row < skillLine.getRecordCount(); ++row)
            {
                WDB::Structures::SkillLineEntry entry{};
                entry.id = skillLine.getRecordId(row);
                entry.Name[0] = keepForeverDb2String(skillLine.getString(row, 0));
                entry.type = skillLine.getUInt8(row, 6);              // CategoryID
                entry.spell_icon = skillLine.getUInt32(row, 7);      // SpellIconFileID
                entry.linkable = skillLine.getUInt8(row, 8);         // CanLink
                entries.emplace_back(entry.id, entry);
            }
            sSkillLineStore.assignEntries(entries);
        }

        if (haveSkillLineAbility && verifyFields(skillLineAbility, "SkillLineAbility.db2", 18))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SkillLineAbilityEntry>> entries;
            entries.reserve(skillLineAbility.getRecordCount());
            for (uint32_t row = 0; row < skillLineAbility.getRecordCount(); ++row)
            {
                WDB::Structures::SkillLineAbilityEntry entry{};
                entry.Id = skillLineAbility.getRecordId(row);
                entry.skilline = skillLineAbility.getUInt16(row, 3);
                entry.spell = skillLineAbility.getUInt32(row, 4);
                entry.minSkillLineRank = skillLineAbility.getUInt16(row, 5);
                entry.class_mask = skillLineAbility.getUInt32(row, 6);
                entry.next = skillLineAbility.getUInt32(row, 7);     // SupercedesSpell
                entry.acquireMethod = skillLineAbility.getUInt32(row, 8);
                entry.grey = skillLineAbility.getUInt16(row, 9);
                entry.green = skillLineAbility.getUInt16(row, 10);
                // 1.60.1 stores RaceMasks[2] in field 17. Convert once into the common runtime RaceSet.
                entry.races.setLow64(skillLineAbility.getUInt32(row, 17, 0), skillLineAbility.getUInt32(row, 17, 1));
                entries.emplace_back(entry.Id, entry);
            }
            sSkillLineAbilityStore.assignEntries(entries);
        }

        std::unordered_map<uint32_t, WDB::Structures::SpellEntry> spellEntries;
        if (haveSpellName && verifyFields(spellName, "SpellName.db2", 1))
        {
            spellEntries.reserve(spellName.getRecordCount());
            for (uint32_t row = 0; row < spellName.getRecordCount(); ++row)
            {
                WDB::Structures::SpellEntry entry{};
                entry.Id = spellName.getRecordId(row);
                char* const name = keepForeverDb2String(spellName.getString(row, 0));
                entry.Name = name != nullptr ? name : "";
                entry.Rank = "";
                spellEntries.emplace(entry.Id, entry);
            }
        }

        auto linkSpell = [&](uint32_t spellId, uint32_t rowId, auto member)
        {
            if (spellId == 0)
                return;

            auto itr = spellEntries.find(spellId);
            if (itr != spellEntries.end())
                itr->second.*member = rowId;
        };

        if (haveUnitCondition)
        {
            if (unitCondition.getFieldCount() != 4)
            {
                sLogger.warning("DB2: UnitCondition.db2 skipped (optional, fields={}, expected=4).", unitCondition.getFieldCount());
            }
            else
            {
                std::vector<std::pair<uint32_t, WDB::Structures::ForeverUnitConditionEntry>> entries;
                entries.reserve(unitCondition.getRecordCount());
                for (uint32_t row = 0; row < unitCondition.getRecordCount(); ++row)
                {
                    WDB::Structures::ForeverUnitConditionEntry entry{};
                    entry.Id = unitCondition.getRecordId(row);
                    entry.Flags = unitCondition.getUInt32(row, 0);
                    for (uint8_t i = 0; i < 8; ++i)
                    {
                        entry.Variable[i] = unitCondition.getUInt8(row, 1, i);
                        entry.Op[i] = unitCondition.getUInt8(row, 2, i);
                        entry.Value[i] = unitCondition.getInt32(row, 3, i);
                    }
                    entries.emplace_back(entry.Id, entry);
                }
                sForeverUnitConditionStore.assignEntries(entries);
                sLogger.info("DB2: UnitCondition store ready (entries={}).", entries.size());
            }
        }

        if (havePlayerCondition)
        {
            if (playerCondition.getFieldCount() != 86)
            {
                sLogger.warning("DB2: PlayerCondition.db2 skipped (optional, fields={}, expected=86).", playerCondition.getFieldCount());
            }
            else
            {
                std::vector<std::pair<uint32_t, WDB::Structures::ForeverPlayerConditionEntry>> entries;
                entries.reserve(playerCondition.getRecordCount());
                uint32_t supported = 0;
                uint32_t unsupported = 0;
                for (uint32_t row = 0; row < playerCondition.getRecordCount(); ++row)
                {
                    WDB::Structures::ForeverPlayerConditionEntry entry{};
                    entry.Id = playerCondition.getRecordId(row);
                    entry.MinLevel = playerCondition.getUInt16(row, 1);
                    entry.MaxLevel = playerCondition.getUInt16(row, 2);
                    entry.ClassMask = playerCondition.getUInt32(row, 3);
                    entry.ReputationLogic = playerCondition.getUInt32(row, 10);
                    entry.PrevQuestLogic = playerCondition.getUInt32(row, 13);
                    entry.CurrQuestLogic = playerCondition.getUInt32(row, 14);
                    entry.CurrentCompletedQuestLogic = playerCondition.getUInt32(row, 15);
                    entry.SpellLogic = playerCondition.getUInt32(row, 16);
                    entry.ItemLogic = playerCondition.getUInt32(row, 17);
                    entry.ItemFlags = playerCondition.getUInt32(row, 18);
                    entry.AuraSpellLogic = playerCondition.getUInt32(row, 19);
                    entry.Gender = playerCondition.getInt8(row, 25);
                    entry.NativeGender = playerCondition.getInt8(row, 26);
                    entry.Flags = playerCondition.getUInt32(row, 41);
                    for (uint8_t i = 0; i < 3; ++i)
                    {
                        entry.MinFactionId[i] = playerCondition.getUInt32(row, 62, i);
                        entry.MinReputation[i] = playerCondition.getUInt8(row, 63, i);
                    }
                    for (uint8_t i = 0; i < 4; ++i)
                    {
                        entry.PrevQuestId[i] = playerCondition.getInt32(row, 64, i);
                        entry.CurrQuestId[i] = playerCondition.getInt32(row, 65, i);
                        entry.CurrentCompletedQuestId[i] = playerCondition.getInt32(row, 66, i);
                        entry.SpellId[i] = playerCondition.getInt32(row, 67, i);
                        entry.ItemId[i] = playerCondition.getInt32(row, 68, i);
                        entry.ItemCount[i] = playerCondition.getUInt32(row, 69, i);
                        entry.AuraSpellId[i] = playerCondition.getInt32(row, 72, i);
                        entry.AuraStacks[i] = playerCondition.getUInt8(row, 73, i);
                    }
                    entry.RaceMask = static_cast<uint64_t>(playerCondition.getUInt32(row, 82, 0)) | (static_cast<uint64_t>(playerCondition.getUInt32(row, 82, 1)) << 32);

                    // Keep PlayerCondition selection conservative: any requirement we do not yet evaluate makes this row unsupported.
                    const bool unsupportedScalar = playerCondition.getUInt32(row, 4) != 0 || playerCondition.getUInt32(row, 5) != 0 || playerCondition.getUInt8(row, 6) != 0 || playerCondition.getUInt32(row, 7) != 0 ||
                        playerCondition.getUInt16(row, 8) != 0 || playerCondition.getUInt8(row, 9) != 0 || playerCondition.getInt8(row, 11) != 0 || playerCondition.getUInt8(row, 12) != 0 ||
                        playerCondition.getUInt16(row, 20) != 0 || playerCondition.getInt32(row, 21) != 0 || playerCondition.getUInt8(row, 22) != 0 || playerCondition.getInt8(row, 23) != 0 ||
                        playerCondition.getUInt32(row, 24) != 0 || entry.NativeGender >= 0 || playerCondition.getUInt32(row, 27) != 0 || playerCondition.getUInt32(row, 28) != 0 || playerCondition.getUInt32(row, 29) != 0 ||
                        playerCondition.getInt32(row, 30) != 0 || playerCondition.getUInt32(row, 31) != 0 || playerCondition.getInt8(row, 32) > 0 || playerCondition.getInt8(row, 33) > 0 ||
                        playerCondition.getInt32(row, 34) != 0 || playerCondition.getInt32(row, 35) != 0 || playerCondition.getUInt16(row, 36) != 0 || playerCondition.getUInt16(row, 37) != 0 ||
                        playerCondition.getInt32(row, 38) != 0 || playerCondition.getUInt16(row, 39) != 0 || playerCondition.getUInt32(row, 40) != 0 || playerCondition.getInt8(row, 42) >= 0 ||
                        playerCondition.getInt8(row, 43) >= 0 || playerCondition.getUInt32(row, 44) != 0 || playerCondition.getInt8(row, 45) >= 0 || playerCondition.getUInt8(row, 46) != 0 ||
                        playerCondition.getInt8(row, 47) != 0 || playerCondition.getUInt32(row, 48) != 0 || playerCondition.getInt32(row, 49) != 0 || playerCondition.getUInt8(row, 50) != 0 ||
                        playerCondition.getUInt8(row, 51) != 0 || playerCondition.getInt8(row, 52) > 0 || playerCondition.getInt8(row, 53) > 0 || playerCondition.getInt8(row, 54) > 0 ||
                        playerCondition.getInt8(row, 55) > 0 || playerCondition.getInt32(row, 56) != 0 || playerCondition.getInt32(row, 57) != 0 || playerCondition.getUInt32(row, 58) != 0;
                    bool unsupportedArrays = false;
                    for (uint8_t i = 0; i < 4; ++i)
                    {
                        unsupportedArrays = unsupportedArrays || playerCondition.getUInt16(row, 59, i) != 0 || playerCondition.getUInt16(row, 60, i) != 0 || playerCondition.getUInt16(row, 61, i) != 0 ||
                            playerCondition.getUInt32(row, 74, i) != 0 || playerCondition.getUInt16(row, 75, i) != 0 || playerCondition.getUInt8(row, 76, i) != 0 || playerCondition.getUInt8(row, 77, i) != 0 ||
                            playerCondition.getUInt32(row, 78, i) != 0 || playerCondition.getUInt32(row, 79, i) != 0 || playerCondition.getUInt32(row, 80, i) != 0 ||
                            playerCondition.getInt32(row, 83, i) != 0 || playerCondition.getUInt16(row, 84, i) != 0 || playerCondition.getUInt16(row, 85, i) != 0;
                    }
                    for (uint8_t i = 0; i < 2; ++i)
                        unsupportedArrays = unsupportedArrays || playerCondition.getUInt16(row, 70, i) != 0 || playerCondition.getUInt32(row, 71, i) != 0;
                    for (uint8_t i = 0; i < 6; ++i)
                        unsupportedArrays = unsupportedArrays || playerCondition.getUInt32(row, 81, i) != 0;

                    entry.HasUnsupportedRequirements = unsupportedScalar || unsupportedArrays || entry.Flags != 0;
                    if (entry.HasUnsupportedRequirements)
                        ++unsupported;
                    else
                        ++supported;
                    entries.emplace_back(entry.Id, entry);
                }
                sForeverPlayerConditionStore.assignEntries(entries);
                sLogger.info("DB2: PlayerCondition store ready (entries={}, supported={}, fallback={}).", entries.size(), supported, unsupported);
            }
        }

        if (haveSpellXSpellVisual)
        {
            if (spellXSpellVisual.getFieldCount() != 12)
            {
                sLogger.warning("DB2: SpellXSpellVisual.db2 skipped (optional, fields={}, expected=12).", spellXSpellVisual.getFieldCount());
            }
            else
            {
                struct VisualCandidate
                {
                    uint32_t rowId = 0;
                    int32_t priority = std::numeric_limits<int32_t>::min();
                };

                std::unordered_map<uint32_t, VisualCandidate> defaultVisuals;
                std::unordered_map<uint32_t, VisualCandidate> fallbackVisuals;
                uint32_t conditionalRows = 0;
                for (uint32_t row = 0; row < spellXSpellVisual.getRecordCount(); ++row)
                {
                    const uint32_t spellId = spellXSpellVisual.getParentId(row);
                    auto spellItr = spellEntries.find(spellId);
                    if (spellId == 0 || spellItr == spellEntries.end())
                        continue;

                    const int16_t difficultyId = spellXSpellVisual.getInt16(row, 1);
                    const uint32_t spellVisualId = spellXSpellVisual.getUInt32(row, 2);
                    if (difficultyId != 0 || spellVisualId == 0)
                        continue;

                    WDB::Structures::ForeverSpellXSpellVisualEntry visual{};
                    visual.Id = spellXSpellVisual.getRecordId(row);
                    visual.SpellVisualId = spellVisualId;
                    visual.Probability = spellXSpellVisual.getFloat(row, 3);
                    visual.Priority = spellXSpellVisual.getInt32(row, 5);
                    visual.ViewerUnitConditionId = spellXSpellVisual.getUInt16(row, 8);
                    visual.ViewerPlayerConditionId = spellXSpellVisual.getUInt32(row, 9);
                    visual.CasterUnitConditionId = spellXSpellVisual.getUInt16(row, 10);
                    visual.CasterPlayerConditionId = spellXSpellVisual.getUInt32(row, 11);
                    spellItr->second.SpellXSpellVisuals.push_back(visual);

                    const bool conditional = visual.ViewerUnitConditionId != 0 || visual.ViewerPlayerConditionId != 0 || visual.CasterUnitConditionId != 0 || visual.CasterPlayerConditionId != 0;
                    if (conditional)
                        ++conditionalRows;

                    auto& fallback = fallbackVisuals[spellId];
                    if (fallback.rowId == 0 || visual.Priority > fallback.priority)
                    {
                        fallback.rowId = visual.Id;
                        fallback.priority = visual.Priority;
                    }

                    if (conditional)
                        continue;

                    auto& candidate = defaultVisuals[spellId];
                    if (candidate.rowId == 0 || visual.Priority > candidate.priority)
                    {
                        candidate.rowId = visual.Id;
                        candidate.priority = visual.Priority;
                    }
                }

                uint32_t fallbackCount = 0;
                for (auto const& [spellId, fallback] : fallbackVisuals)
                {
                    auto& visuals = spellEntries[spellId].SpellXSpellVisuals;
                    std::sort(visuals.begin(), visuals.end(), [](auto const& left, auto const& right)
                    {
                        if (left.Priority != right.Priority)
                            return left.Priority > right.Priority;
                        return left.Id < right.Id;
                    });

                    auto const defaultItr = defaultVisuals.find(spellId);
                    if (defaultItr != defaultVisuals.end())
                    {
                        spellEntries[spellId].SpellXSpellVisualId = defaultItr->second.rowId;
                        continue;
                    }

                    spellEntries[spellId].SpellXSpellVisualId = fallback.rowId;
                    ++fallbackCount;
                }

                sLogger.info("DB2: SpellXSpellVisual store ready (defaults={}, conditionalFallbacks={}, conditionalRows={}).", defaultVisuals.size(), fallbackCount, conditionalRows);
            }
        }

        if (haveAuraOptions && verifyFields(spellAuraOptions, "SpellAuraOptions.db2", 7))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellAuraOptionsEntry>> entries;
            entries.reserve(spellAuraOptions.getRecordCount());
            for (uint32_t row = 0; row < spellAuraOptions.getRecordCount(); ++row)
            {
                WDB::Structures::SpellAuraOptionsEntry entry{};
                entry.Id = spellAuraOptions.getRecordId(row);
                entry.DifficultyId = spellAuraOptions.getUInt16(row, 0);
                entry.MaxStackAmount = spellAuraOptions.getUInt16(row, 1);
                entry.ProcCategoryRecovery = spellAuraOptions.getUInt32(row, 2);
                entry.procChance = spellAuraOptions.getUInt8(row, 3);
                entry.procCharges = spellAuraOptions.getUInt32(row, 4);
                entry.SpellProcsPerMinuteId = spellAuraOptions.getUInt16(row, 5);
                entry.ProcTypeMask[0] = spellAuraOptions.getUInt32(row, 6, 0);
                entry.ProcTypeMask[1] = spellAuraOptions.getUInt32(row, 6, 1);
                entry.procTypeMask = static_cast<uint64_t>(entry.ProcTypeMask[0]) | (static_cast<uint64_t>(entry.ProcTypeMask[1]) << 32);
                entry.procFlags = entry.ProcTypeMask[0];
                entries.emplace_back(entry.Id, entry);
                linkSpell(spellAuraOptions.getParentId(row), entry.Id, &WDB::Structures::SpellEntry::SpellAuraOptionsId);
            }
            sSpellAuraOptionsStore.assignEntries(entries);
        }

        if (haveAuraRestrictions && verifyFields(spellAuraRestrictions, "SpellAuraRestrictions.db2", 13))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellAuraRestrictionsEntry>> entries;
            entries.reserve(spellAuraRestrictions.getRecordCount());
            for (uint32_t row = 0; row < spellAuraRestrictions.getRecordCount(); ++row)
            {
                uint32_t const id = spellAuraRestrictions.getRecordId(row);
                WDB::Structures::SpellAuraRestrictionsEntry entry{};
                entry.DifficultyId = spellAuraRestrictions.getUInt16(row, 0);
                entry.CasterAuraState = spellAuraRestrictions.getUInt32(row, 1);
                entry.TargetAuraState = spellAuraRestrictions.getUInt32(row, 2);
                entry.CasterAuraStateNot = spellAuraRestrictions.getUInt32(row, 3);
                entry.TargetAuraStateNot = spellAuraRestrictions.getUInt32(row, 4);
                entry.casterAuraSpell = spellAuraRestrictions.getUInt32(row, 5);
                entry.targetAuraSpell = spellAuraRestrictions.getUInt32(row, 6);
                entry.CasterAuraSpellNot = spellAuraRestrictions.getUInt32(row, 7);
                entry.TargetAuraSpellNot = spellAuraRestrictions.getUInt32(row, 8);
                entry.CasterAuraType = spellAuraRestrictions.getUInt16(row, 9);
                entry.TargetAuraType = spellAuraRestrictions.getUInt16(row, 10);
                entry.CasterAuraTypeNot = spellAuraRestrictions.getUInt16(row, 11);
                entry.TargetAuraTypeNot = spellAuraRestrictions.getUInt16(row, 12);
                entries.emplace_back(id, entry);
                linkSpell(spellAuraRestrictions.getParentId(row), id, &WDB::Structures::SpellEntry::SpellAuraRestrictionsId);
            }
            sSpellAuraRestrictionsStore.assignEntries(entries);
        }

        if (haveCastTimes && verifyFields(spellCastTimes, "SpellCastTimes.db2", 2))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellCastTimesEntry>> entries;
            entries.reserve(spellCastTimes.getRecordCount());
            for (uint32_t row = 0; row < spellCastTimes.getRecordCount(); ++row)
            {
                WDB::Structures::SpellCastTimesEntry entry{};
                entry.ID = spellCastTimes.getRecordId(row);
                entry.CastTime = spellCastTimes.getUInt32(row, 0);       // Base
                entry.CastTimePerLevel = 0.0f;                           // removed from modern DB2
                entry.MinCastTime = spellCastTimes.getInt32(row, 1);    // Minimum
                entries.emplace_back(entry.ID, entry);
            }
            sSpellCastTimesStore.assignEntries(entries);
        }

        if (haveCastingRequirements && verifyFields(spellCastingRequirements, "SpellCastingRequirements.db2", 7))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellCastingRequirementsEntry>> entries;
            entries.reserve(spellCastingRequirements.getRecordCount());
            for (uint32_t row = 0; row < spellCastingRequirements.getRecordCount(); ++row)
            {
                uint32_t const id = spellCastingRequirements.getRecordId(row);
                WDB::Structures::SpellCastingRequirementsEntry entry{};
                entry.FacingCasterFlags = spellCastingRequirements.getUInt32(row, 1);
                entry.MinFactionId = spellCastingRequirements.getUInt16(row, 2);
                entry.MinReputation = spellCastingRequirements.getInt32(row, 3);
                entry.AreaGroupId = static_cast<int32_t>(spellCastingRequirements.getUInt16(row, 4));
                entry.RequiredAuraVision = spellCastingRequirements.getUInt8(row, 5);
                entry.RequiresSpellFocus = spellCastingRequirements.getUInt16(row, 6);
                entries.emplace_back(id, entry);
                linkSpell(spellCastingRequirements.getUInt32(row, 0), id, &WDB::Structures::SpellEntry::SpellCastingRequirementsId);
            }
            sSpellCastingRequirementsStore.assignEntries(entries);
        }

        if (haveCategories && verifyFields(spellCategories, "SpellCategories.db2", 9))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellCategoriesEntry>> entries;
            entries.reserve(spellCategories.getRecordCount());
            for (uint32_t row = 0; row < spellCategories.getRecordCount(); ++row)
            {
                uint32_t const id = spellCategories.getRecordId(row);
                WDB::Structures::SpellCategoriesEntry entry{};
                entry.DifficultyId = spellCategories.getUInt16(row, 0);
                entry.Category = spellCategories.getUInt16(row, 1);
                entry.DmgClass = spellCategories.getUInt8(row, 2);
                entry.DiminishType = spellCategories.getUInt32(row, 3);
                entry.DispelType = spellCategories.getUInt8(row, 4);
                entry.MechanicsType = spellCategories.getUInt8(row, 5);
                entry.PreventionType = spellCategories.getUInt32(row, 6);
                entry.StartRecoveryCategory = spellCategories.getUInt16(row, 7);
                entry.ChargeCategory = spellCategories.getUInt16(row, 8);
                entries.emplace_back(id, entry);
                linkSpell(spellCategories.getParentId(row), id, &WDB::Structures::SpellEntry::SpellCategoriesId);
            }
            sSpellCategoriesStore.assignEntries(entries);
        }

        if (haveClassOptions && verifyFields(spellClassOptions, "SpellClassOptions.db2", 4))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellClassOptionsEntry>> entries;
            entries.reserve(spellClassOptions.getRecordCount());
            for (uint32_t row = 0; row < spellClassOptions.getRecordCount(); ++row)
            {
                uint32_t const id = spellClassOptions.getRecordId(row);
                WDB::Structures::SpellClassOptionsEntry entry{};
                entry.ModalNextSpell = spellClassOptions.getUInt32(row, 1);
                entry.SpellFamilyName = spellClassOptions.getUInt32(row, 2);
                for (uint32_t i = 0; i < MAX_SPELL_CLASS_MASKS; ++i)
                    entry.SpellFamilyFlags[i] = spellClassOptions.getUInt32(row, 3, i);
                entries.emplace_back(id, entry);
                linkSpell(spellClassOptions.getUInt32(row, 0), id, &WDB::Structures::SpellEntry::SpellClassOptionsId);
            }
            sSpellClassOptionsStore.assignEntries(entries);
        }

        if (haveCooldowns && verifyFields(spellCooldowns, "SpellCooldowns.db2", 5))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellCooldownsEntry>> entries;
            entries.reserve(spellCooldowns.getRecordCount());
            for (uint32_t row = 0; row < spellCooldowns.getRecordCount(); ++row)
            {
                uint32_t const id = spellCooldowns.getRecordId(row);
                WDB::Structures::SpellCooldownsEntry entry{};
                entry.DifficultyId = spellCooldowns.getUInt16(row, 0);
                entry.CategoryRecoveryTime = spellCooldowns.getUInt32(row, 1);
                entry.RecoveryTime = spellCooldowns.getUInt32(row, 2);
                entry.StartRecoveryTime = spellCooldowns.getUInt32(row, 3);
                entry.AuraSpellId = spellCooldowns.getUInt32(row, 4);
                entries.emplace_back(id, entry);
                linkSpell(spellCooldowns.getParentId(row), id, &WDB::Structures::SpellEntry::SpellCooldownsId);
            }
            sSpellCooldownsStore.assignEntries(entries);
        }

        if (haveDuration && verifyFields(spellDuration, "SpellDuration.db2", 3))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellDurationEntry>> entries;
            entries.reserve(spellDuration.getRecordCount());
            for (uint32_t row = 0; row < spellDuration.getRecordCount(); ++row)
            {
                WDB::Structures::SpellDurationEntry entry{};
                entry.ID = spellDuration.getRecordId(row);
                entry.Duration1 = spellDuration.getInt32(row, 0); // Duration
                entry.Duration2 = spellDuration.getInt32(row, 1); // MaxDuration
                entry.Duration3 = spellDuration.getInt32(row, 2); // DurationPerResource
                entries.emplace_back(entry.ID, entry);
            }
            sSpellDurationStore.assignEntries(entries);
        }

        if (haveEffect && verifyFields(spellEffect, "SpellEffect.db2", 29))
        {
            uint32_t maxForeverEffectId = 0;
            uint32_t maxForeverAuraId = 0;
            std::vector<std::pair<uint32_t, WDB::Structures::SpellEffectEntry>> entries;
            entries.reserve(spellEffect.getRecordCount());
            for (uint32_t row = 0; row < spellEffect.getRecordCount(); ++row)
            {
                WDB::Structures::SpellEffectEntry entry{};
                entry.id = spellEffect.getRecordId(row);
                entry.EffectApplyAuraName = spellEffect.getUInt16(row, 0);
                entry.DifficultyId = spellEffect.getUInt16(row, 1);
                entry.EffectIndex = spellEffect.getUInt32(row, 2);
                entry.Effect = spellEffect.getUInt32(row, 3);
                maxForeverEffectId = std::max(maxForeverEffectId, entry.Effect);
                maxForeverAuraId = std::max(maxForeverAuraId, static_cast<uint32_t>(entry.EffectApplyAuraName));
                entry.EffectAmplitudeFloat = spellEffect.getFloat(row, 4);
                entry.EffectAttributes = spellEffect.getUInt32(row, 5);
                entry.EffectAmplitude = spellEffect.getUInt32(row, 6); // EffectAuraPeriod
                entry.EffectBonusCoefficient = spellEffect.getFloat(row, 7);
                entry.EffectSpellPowerCoefficient = entry.EffectBonusCoefficient;
                entry.EffectChainAmplitude = spellEffect.getFloat(row, 8);
                entry.EffectChainTarget = spellEffect.getUInt32(row, 9);
                entry.EffectItemType = spellEffect.getUInt32(row, 10);
                entry.EffectMechanic = spellEffect.getUInt32(row, 11);
                entry.EffectPointsPerResource = spellEffect.getFloat(row, 12);
                entry.EffectPointsPerComboPoint = entry.EffectPointsPerResource; // legacy compatibility
                entry.EffectPosFacing = spellEffect.getFloat(row, 13);
                entry.EffectRealPointsPerLevel = spellEffect.getFloat(row, 14);
                entry.EffectTriggerSpell = spellEffect.getUInt32(row, 15);
                entry.BonusCoefficientFromAP = spellEffect.getFloat(row, 16);
                entry.PvpMultiplier = spellEffect.getFloat(row, 17);
                entry.Coefficient = spellEffect.getFloat(row, 18);
                entry.EffectDamageMultiplier = entry.Coefficient; // legacy compatibility
                entry.Variance = spellEffect.getFloat(row, 19);
                entry.ResourceCoefficient = spellEffect.getFloat(row, 20);
                entry.GroupSizeBasePointsCoefficient = spellEffect.getFloat(row, 21);
                entry.EffectBasePointsF = spellEffect.getFloat(row, 22);
                entry.EffectBasePoints = static_cast<int32_t>(entry.EffectBasePointsF); // legacy compatibility
                entry.ScalingClass = spellEffect.getInt32(row, 23);
                entry.UnknownField24 = spellEffect.getUInt32(row, 24);
                entry.EffectMiscValue = spellEffect.getInt32(row, 25, 0);
                entry.EffectMiscValueB = spellEffect.getInt32(row, 25, 1);
                entry.EffectRadiusIndex = spellEffect.getUInt32(row, 26, 0);
                entry.EffectRadiusMaxIndex = spellEffect.getUInt32(row, 26, 1);
                for (uint32_t i = 0; i < 4; ++i)
                    entry.EffectSpellClassMask[i] = spellEffect.getUInt32(row, 27, i);
                entry.EffectImplicitTargetA = spellEffect.getUInt16(row, 28, 0);
                entry.EffectImplicitTargetB = spellEffect.getUInt16(row, 28, 1);
                entry.EffectSpellId = spellEffect.getParentId(row);
                entries.emplace_back(entry.id, entry);
            }
            sSpellEffectStore.assignEntries(entries);
            sLogger.info("DB2: SpellEffect coverage (maxEffect={}, effectEnumLimit={}, maxAura={}, auraEnumLimit={}).", maxForeverEffectId, TOTAL_SPELL_EFFECTS - 1, maxForeverAuraId, TOTAL_SPELL_AURAS - 1);
            sSpellEffectMap.clear();
            for (uint32_t id = 0; id < sSpellEffectStore.getNumRows(); ++id)
            {
                auto const* effect = sSpellEffectStore.lookupEntry(id);
                if (effect != nullptr && effect->EffectSpellId != 0 && effect->EffectIndex < 32)
                    sSpellEffectMap[effect->EffectSpellId].effects[effect->EffectIndex] = effect;
            }
        }

        if (haveEquippedItems && verifyFields(spellEquippedItems, "SpellEquippedItems.db2", 4))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellEquippedItemsEntry>> entries;
            entries.reserve(spellEquippedItems.getRecordCount());
            for (uint32_t row = 0; row < spellEquippedItems.getRecordCount(); ++row)
            {
                uint32_t const id = spellEquippedItems.getRecordId(row);
                WDB::Structures::SpellEquippedItemsEntry entry{};
                entry.EquippedItemClass = spellEquippedItems.getInt32(row, 1);
                entry.EquippedItemInventoryTypeMask = spellEquippedItems.getInt32(row, 2);
                entry.EquippedItemSubClassMask = spellEquippedItems.getInt32(row, 3);
                entries.emplace_back(id, entry);
                linkSpell(spellEquippedItems.getUInt32(row, 0), id, &WDB::Structures::SpellEntry::SpellEquippedItemsId);
            }
            sSpellEquippedItemsStore.assignEntries(entries);
        }

        if (haveInterrupts && verifyFields(spellInterrupts, "SpellInterrupts.db2", 4))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellInterruptsEntry>> entries;
            entries.reserve(spellInterrupts.getRecordCount());
            for (uint32_t row = 0; row < spellInterrupts.getRecordCount(); ++row)
            {
                uint32_t const id = spellInterrupts.getRecordId(row);
                WDB::Structures::SpellInterruptsEntry entry{};
                entry.DifficultyId = spellInterrupts.getUInt16(row, 0);
                entry.InterruptFlags = spellInterrupts.getUInt32(row, 1);
                entry.AuraInterruptFlagsRaw[0] = spellInterrupts.getUInt32(row, 2, 0);
                entry.AuraInterruptFlagsRaw[1] = spellInterrupts.getUInt32(row, 2, 1);
                entry.ChannelInterruptFlagsRaw[0] = spellInterrupts.getUInt32(row, 3, 0);
                entry.ChannelInterruptFlagsRaw[1] = spellInterrupts.getUInt32(row, 3, 1);
                entry.AuraInterruptFlags = static_cast<uint64_t>(entry.AuraInterruptFlagsRaw[0]) | (static_cast<uint64_t>(entry.AuraInterruptFlagsRaw[1]) << 32);
                entry.ChannelInterruptFlags = static_cast<uint64_t>(entry.ChannelInterruptFlagsRaw[0]) | (static_cast<uint64_t>(entry.ChannelInterruptFlagsRaw[1]) << 32);
                entries.emplace_back(id, entry);
                linkSpell(spellInterrupts.getParentId(row), id, &WDB::Structures::SpellEntry::SpellInterruptsId);
            }
            sSpellInterruptsStore.assignEntries(entries);
        }

        if (haveLevels && verifyFields(spellLevels, "SpellLevels.db2", 5))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellLevelsEntry>> entries;
            entries.reserve(spellLevels.getRecordCount());
            for (uint32_t row = 0; row < spellLevels.getRecordCount(); ++row)
            {
                uint32_t const id = spellLevels.getRecordId(row);
                WDB::Structures::SpellLevelsEntry entry{};
                entry.DifficultyId = spellLevels.getUInt16(row, 0);
                entry.maxLevel = spellLevels.getUInt16(row, 1);
                entry.MaxPassiveAuraLevel = spellLevels.getUInt8(row, 2);
                entry.baseLevel = spellLevels.getUInt32(row, 3);
                entry.spellLevel = spellLevels.getUInt32(row, 4);
                entries.emplace_back(id, entry);
                linkSpell(spellLevels.getParentId(row), id, &WDB::Structures::SpellEntry::SpellLevelsId);
            }
            sSpellLevelsStore.assignEntries(entries);
        }

        if (haveMisc && verifyFields(spellMisc, "SpellMisc.db2", 16))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellMiscEntry>> entries;
            entries.reserve(spellMisc.getRecordCount());
            for (uint32_t row = 0; row < spellMisc.getRecordCount(); ++row)
            {
                WDB::Structures::SpellMiscEntry entry{};

                entry.Id = spellMisc.getRecordId(row);

                entry.Attributes = spellMisc.getUInt32(row, 0, 0);
                entry.AttributesEx = spellMisc.getUInt32(row, 0, 1);
                entry.AttributesExB = spellMisc.getUInt32(row, 0, 2);
                entry.AttributesExC = spellMisc.getUInt32(row, 0, 3);
                entry.AttributesExD = spellMisc.getUInt32(row, 0, 4);
                entry.AttributesExE = spellMisc.getUInt32(row, 0, 5);
                entry.AttributesExF = spellMisc.getUInt32(row, 0, 6);
                entry.AttributesExG = spellMisc.getUInt32(row, 0, 7);
                entry.AttributesExH = spellMisc.getUInt32(row, 0, 8);
                entry.AttributesExI = spellMisc.getUInt32(row, 0, 9);
                entry.AttributesExJ = spellMisc.getUInt32(row, 0, 10);
                entry.AttributesExK = spellMisc.getUInt32(row, 0, 11);
                entry.AttributesExL = spellMisc.getUInt32(row, 0, 12);
                entry.AttributesExM = spellMisc.getUInt32(row, 0, 13);
                entry.AttributesExN = spellMisc.getUInt32(row, 0, 14);
                entry.AttributesExO = spellMisc.getUInt32(row, 0, 15);
                entry.AttributesExP = spellMisc.getUInt32(row, 0, 16);

                entry.SpellDifficultyId = spellMisc.getUInt16(row, 1);
                entry.CastingTimeIndex = spellMisc.getUInt16(row, 2);
                entry.DurationIndex = spellMisc.getUInt16(row, 3);
                entry.PvPDurationIndex = spellMisc.getUInt16(row, 4);
                entry.RangeIndex = spellMisc.getUInt16(row, 5);
                entry.SchoolMask = spellMisc.getUInt8(row, 6);

                entry.Speed = spellMisc.getFloat(row, 7);
                entry.LaunchDelay = spellMisc.getFloat(row, 8);
                entry.MinDuration = spellMisc.getFloat(row, 9);

                entry.SpellIconFileDataId = spellMisc.getUInt32(row, 10);
                entry.ActiveIconFileDataId = spellMisc.getUInt32(row, 11);
                entry.ContentTuningId = spellMisc.getUInt32(row, 12);
                entry.ShowFutureSpellPlayerConditionId = spellMisc.getUInt32(row, 13);
                entry.SpellVisualScript = spellMisc.getUInt32(row, 14);
                entry.ActiveSpellVisualScript = spellMisc.getUInt32(row, 15);

                entries.emplace_back(entry.Id, entry);

                linkSpell(spellMisc.getParentId(row), entry.Id, &WDB::Structures::SpellEntry::SpellMiscId);
            }
            sSpellMiscStore.assignEntries(entries);
        }

        if (havePower && verifyFields(spellPower, "SpellPower.db2", 14))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellPowerEntry>> entries;
            entries.reserve(spellPower.getRecordCount());
            for (uint32_t row = 0; row < spellPower.getRecordCount(); ++row)
            {
                WDB::Structures::SpellPowerEntry entry{};
                uint32_t const id = spellPower.getRecordId(row);
                // Forever 1.60.1.70009, SpellPower.db2 layout 0x61AD223F:
                // 0 ID, 1 OrderIndex, 2 ManaCost, 3 ManaCostPerLevel, 4 ManaPerSecond,
                // 5 PowerDisplayID, 6 AltPowerBarID, 7 PowerCostPct, 8 PowerCostMaxPct,
                // 9 OptionalCostPct, 10 PowerPctPerSecond, 11 PowerType,
                // 12 RequiredAuraSpellID, 13 OptionalCost. SpellID is the relation/parent id.
                entry.spellId = spellPower.getParentId(row);
                entry.orderIndex = spellPower.getUInt8(row, 1);
                entry.manaCost = spellPower.getUInt32(row, 2);
                entry.manaCostPerlevel = spellPower.getUInt32(row, 3);
                entry.manaPerSecond = spellPower.getUInt32(row, 4);
                entry.powerDisplayId = spellPower.getUInt32(row, 5);
                entry.altPowerBarId = spellPower.getUInt32(row, 6);
                entry.ManaCostPercentageFloat = spellPower.getFloat(row, 7);
                entry.ManaCostMaxPercentageFloat = spellPower.getFloat(row, 8);
                entry.OptionalCostPercentageFloat = spellPower.getFloat(row, 9);
                entry.PowerPercentagePerSecondFloat = spellPower.getFloat(row, 10);
                entry.powerType = static_cast<uint32_t>(static_cast<int32_t>(spellPower.getInt8(row, 11)));
                entry.requiredAuraSpellId = spellPower.getUInt32(row, 12);
                entry.optionalCost = spellPower.getUInt32(row, 13);
                entries.emplace_back(id, entry);
            }
            sSpellPowerStore.assignEntries(entries);
            sSpellPowerMap.clear();
            for (uint32_t id = 0; id < sSpellPowerStore.getNumRows(); ++id)
            {
                auto const* power = sSpellPowerStore.lookupEntry(id);
                if (power == nullptr || power->spellId == 0)
                    continue;

                auto itr = sSpellPowerMap.find(power->spellId);
                if (itr == sSpellPowerMap.end() || (itr->second->requiredAuraSpellId != 0 && power->requiredAuraSpellId == 0))
                    sSpellPowerMap[power->spellId] = power;
            }
        }

        if (haveRadius && verifyFields(spellRadius, "SpellRadius.db2", 4))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellRadiusEntry>> entries;
            entries.reserve(spellRadius.getRecordCount());
            for (uint32_t row = 0; row < spellRadius.getRecordCount(); ++row)
            {
                WDB::Structures::SpellRadiusEntry entry{};
                entry.ID = spellRadius.getRecordId(row);
                entry.radius = spellRadius.getFloat(row, 0);
                entry.radius_per_level = spellRadius.getFloat(row, 1);
                entry.radius_min = spellRadius.getFloat(row, 2);
                entry.radius_max = spellRadius.getFloat(row, 3);
                entries.emplace_back(entry.ID, entry);
            }
            sSpellRadiusStore.assignEntries(entries);
        }

        if (haveRange && verifyFields(spellRange, "SpellRange.db2", 5))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellRangeEntry>> entries;
            entries.reserve(spellRange.getRecordCount());
            for (uint32_t row = 0; row < spellRange.getRecordCount(); ++row)
            {
                WDB::Structures::SpellRangeEntry entry{};
                entry.ID = spellRange.getRecordId(row);
                entry.DisplayName = keepForeverDb2String(spellRange.getString(row, 0));
                entry.DisplayNameShort = keepForeverDb2String(spellRange.getString(row, 1));
                entry.range_type = spellRange.getUInt32(row, 2); // Flags
                entry.minRange = spellRange.getFloat(row, 3, 0);
                entry.minRangeFriendly = spellRange.getFloat(row, 3, 1);
                entry.maxRange = spellRange.getFloat(row, 4, 0);
                entry.maxRangeFriendly = spellRange.getFloat(row, 4, 1);
                entries.emplace_back(entry.ID, entry);
            }
            sSpellRangeStore.assignEntries(entries);
        }

        if (haveReagents && verifyFields(spellReagents, "SpellReagents.db2", 5))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellReagentsEntry>> entries;
            entries.reserve(spellReagents.getRecordCount());
            for (uint32_t row = 0; row < spellReagents.getRecordCount(); ++row)
            {
                uint32_t const id = spellReagents.getRecordId(row);
                WDB::Structures::SpellReagentsEntry entry{};
                entry.SpellId = spellReagents.getUInt32(row, 0);
                for (uint32_t i = 0; i < MAX_SPELL_REAGENTS; ++i)
                {
                    entry.Reagent[i] = spellReagents.getInt32(row, 1, i);
                    entry.ReagentCount[i] = spellReagents.getUInt16(row, 2, i);
                    entry.ReagentReCraftCount[i] = spellReagents.getUInt16(row, 3, i);
                    entry.ReagentSource[i] = spellReagents.getUInt8(row, 4, i);
                }
                entries.emplace_back(id, entry);
                linkSpell(entry.SpellId, id, &WDB::Structures::SpellEntry::SpellReagentsId);
            }
            sSpellReagentsStore.assignEntries(entries);
        }

        if (haveScaling && verifyFields(spellScaling, "SpellScaling.db2", 3))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellScalingEntry>> entries;
            entries.reserve(spellScaling.getRecordCount());
            for (uint32_t row = 0; row < spellScaling.getRecordCount(); ++row)
            {
                uint32_t const id = spellScaling.getRecordId(row);
                WDB::Structures::SpellScalingEntry entry{};
                entry.SpellId = spellScaling.getUInt32(row, 0);
                entry.MinScalingLevel = spellScaling.getUInt32(row, 1);
                entry.MaxScalingLevel = spellScaling.getUInt32(row, 2);
                entries.emplace_back(id, entry);
                linkSpell(entry.SpellId, id, &WDB::Structures::SpellEntry::SpellScalingId);
            }
            sSpellScalingStore.assignEntries(entries);
        }

        if (haveShapeshift && verifyFields(spellShapeshift, "SpellShapeshift.db2", 4))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellShapeshiftEntry>> entries;
            entries.reserve(spellShapeshift.getRecordCount());
            for (uint32_t row = 0; row < spellShapeshift.getRecordCount(); ++row)
            {
                uint32_t const id = spellShapeshift.getRecordId(row);
                WDB::Structures::SpellShapeshiftEntry entry{};
                entry.StanceBarOrder = spellShapeshift.getUInt8(row, 1);
                entry.ShapeshiftsExcludedRaw[0] = spellShapeshift.getUInt32(row, 2, 0);
                entry.ShapeshiftsExcludedRaw[1] = spellShapeshift.getUInt32(row, 2, 1);
                entry.ShapeshiftsRaw[0] = spellShapeshift.getUInt32(row, 3, 0);
                entry.ShapeshiftsRaw[1] = spellShapeshift.getUInt32(row, 3, 1);
                entry.ShapeshiftsExcluded = static_cast<uint64_t>(entry.ShapeshiftsExcludedRaw[0]) | (static_cast<uint64_t>(entry.ShapeshiftsExcludedRaw[1]) << 32);
                entry.Shapeshifts = static_cast<uint64_t>(entry.ShapeshiftsRaw[0]) | (static_cast<uint64_t>(entry.ShapeshiftsRaw[1]) << 32);
                entries.emplace_back(id, entry);
                linkSpell(spellShapeshift.getUInt32(row, 0), id, &WDB::Structures::SpellEntry::SpellShapeshiftId);
            }
            sSpellShapeshiftStore.assignEntries(entries);
        }

        if (haveTargetRestrictions && verifyFields(spellTargetRestrictions, "SpellTargetRestrictions.db2", 7))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellTargetRestrictionsEntry>> entries;
            entries.reserve(spellTargetRestrictions.getRecordCount());
            for (uint32_t row = 0; row < spellTargetRestrictions.getRecordCount(); ++row)
            {
                WDB::Structures::SpellTargetRestrictionsEntry entry{};
                entry.Id = spellTargetRestrictions.getRecordId(row);
                entry.DifficultyId = spellTargetRestrictions.getUInt16(row, 0);
                entry.ConeDegrees = spellTargetRestrictions.getFloat(row, 1);
                entry.MaxTargetRadius = 0.0f; // legacy field, absent from Forever DB2
                entry.MaxAffectedTargets = spellTargetRestrictions.getUInt8(row, 2);
                entry.MaxTargetLevel = spellTargetRestrictions.getUInt32(row, 3);
                entry.TargetCreatureType = spellTargetRestrictions.getUInt16(row, 4);
                entry.Targets = spellTargetRestrictions.getUInt32(row, 5);
                entry.Width = spellTargetRestrictions.getFloat(row, 6);
                entries.emplace_back(entry.Id, entry);
                linkSpell(spellTargetRestrictions.getParentId(row), entry.Id, &WDB::Structures::SpellEntry::SpellTargetRestrictionsId);
            }
            sSpellTargetRestrictionsStore.assignEntries(entries);
        }

        if (haveTotems && verifyFields(spellTotems, "SpellTotems.db2", 3))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellTotemsEntry>> entries;
            entries.reserve(spellTotems.getRecordCount());
            for (uint32_t row = 0; row < spellTotems.getRecordCount(); ++row)
            {
                uint32_t const id = spellTotems.getRecordId(row);
                WDB::Structures::SpellTotemsEntry entry{};
                for (uint32_t i = 0; i < MAX_SPELL_TOTEM_CATEGORIES; ++i)
                    entry.TotemCategory[i] = spellTotems.getUInt16(row, 1, i);
                for (uint32_t i = 0; i < MAX_SPELL_TOTEMS; ++i)
                    entry.Totem[i] = spellTotems.getUInt32(row, 2, i);
                entries.emplace_back(id, entry);
                linkSpell(spellTotems.getUInt32(row, 0), id, &WDB::Structures::SpellEntry::SpellTotemsId);
            }
            sSpellTotemsStore.assignEntries(entries);
        }

        if (haveItemEnchantment && verifyFields(spellItemEnchantment, "SpellItemEnchantment.db2", 24))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellItemEnchantmentEntry>> entries;
            entries.reserve(spellItemEnchantment.getRecordCount());
            for (uint32_t row = 0; row < spellItemEnchantment.getRecordCount(); ++row)
            {
                WDB::Structures::SpellItemEnchantmentEntry entry{};
                entry.Id = spellItemEnchantment.getRecordId(row);
                entry.Name[0] = keepForeverDb2String(spellItemEnchantment.getString(row, 0));
                entry.HordeName = keepForeverDb2String(spellItemEnchantment.getString(row, 1));
                entry.Duration = spellItemEnchantment.getUInt32(row, 2);
                entry.Charges = spellItemEnchantment.getUInt32(row, 3);
                for (uint32_t i = 0; i < MAX_ITEM_ENCHANTMENT_EFFECTS; ++i)
                {
                    entry.type[i] = spellItemEnchantment.getUInt32(row, 4, i);
                    entry.min[i] = spellItemEnchantment.getUInt32(row, 5, i);
                    entry.spell[i] = spellItemEnchantment.getUInt32(row, 6, i);
                    entry.EffectScalingPoints[i] = spellItemEnchantment.getFloat(row, 8, i);
                }
                entry.Flags = spellItemEnchantment.getUInt32(row, 7);
                entry.ScalingClass = spellItemEnchantment.getUInt32(row, 9);
                entry.ScalingClassRestricted = spellItemEnchantment.getUInt32(row, 10);
                entry.Unknown11 = spellItemEnchantment.getUInt32(row, 11);
                entry.req_skill = spellItemEnchantment.getUInt32(row, 12);
                entry.req_skill_value = spellItemEnchantment.getUInt32(row, 13);
                entry.req_level = spellItemEnchantment.getUInt32(row, 14);
                entry.MaxLevel = spellItemEnchantment.getUInt32(row, 15);
                entry.IconFileDataId = spellItemEnchantment.getUInt32(row, 16);
                entry.ItemLevelMin = spellItemEnchantment.getUInt32(row, 17);
                entry.ItemLevelMax = spellItemEnchantment.getUInt32(row, 18);
                entry.TransmogUseConditionId = spellItemEnchantment.getUInt32(row, 19);
                entry.TransmogCost = spellItemEnchantment.getUInt32(row, 20);
                entry.Unknown21 = spellItemEnchantment.getUInt32(row, 21);
                entry.visual = spellItemEnchantment.getUInt16(row, 22);
                entry.ItemLevel = spellItemEnchantment.getUInt16(row, 23);
                entries.emplace_back(entry.Id, entry);
            }
            sSpellItemEnchantmentStore.assignEntries(entries);
        }

        if (haveShapeshiftForm && verifyFields(spellShapeshiftForm, "SpellShapeshiftForm.db2", 10))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellShapeshiftFormEntry>> entries;
            entries.reserve(spellShapeshiftForm.getRecordCount());
            for (uint32_t row = 0; row < spellShapeshiftForm.getRecordCount(); ++row)
            {
                WDB::Structures::SpellShapeshiftFormEntry entry{};
                entry.id = spellShapeshiftForm.getRecordId(row);
                entry.Name = keepForeverDb2String(spellShapeshiftForm.getString(row, 0));
                entry.modelId = spellShapeshiftForm.getUInt32(row, 1);
                entry.modelId2 = entry.modelId;
                entry.unit_type = spellShapeshiftForm.getUInt8(row, 2);
                entry.Flags = spellShapeshiftForm.getUInt32(row, 3);
                entry.AttackIconFileId = spellShapeshiftForm.getUInt32(row, 4);
                entry.BonusActionBar = spellShapeshiftForm.getUInt8(row, 5);
                entry.AttackSpeed = spellShapeshiftForm.getUInt16(row, 6);
                entry.DamageVariance = spellShapeshiftForm.getFloat(row, 7);
                entry.MountTypeId = spellShapeshiftForm.getUInt16(row, 8);
                for (uint32_t i = 0; i < 8; ++i)
                    entry.spells[i] = spellShapeshiftForm.getUInt32(row, 9, i);
                entries.emplace_back(entry.id, entry);
            }
            sSpellShapeshiftFormStore.assignEntries(entries);
        }

        if (haveSpellName)
        {
            std::vector<std::pair<uint32_t, WDB::Structures::SpellEntry>> entries;
            entries.reserve(spellEntries.size());
            for (auto const& [id, entry] : spellEntries)
                entries.emplace_back(id, entry);
            sSpellStore.assignEntries(entries);

            sSpellCategoryStore.clear();
            for (auto const& [spellId, spell] : spellEntries)
            {
                if (spell.SpellCategoriesId == 0)
                    continue;
                if (auto const* category = sSpellCategoriesStore.lookupEntry(spell.SpellCategoriesId); category != nullptr && category->Category != 0)
                    sSpellCategoryStore[category->Category].insert(spellId);
            }
        }

        sLogger.info("DB2: Spell/Skill stores ready (skills={}, skillAbilities={}, spells={}, effects={}).", sSkillLineStore.getNumRows(), sSkillLineAbilityStore.getNumRows(), sSpellStore.getNumRows(), sSpellEffectStore.getNumRows());
        return ok;
    }

    void buildPowerIndexByClass();

    bool loadForeverModernCharacterStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDB::WDC5File chrModel;
        WDB::WDC5File chrRaceXChrModel;
        if (!loadForeverWDC5Group({ { chrModel, ForeverFormat::ChrModel }, { chrRaceXChrModel, ForeverFormat::ChrRaceXChrModel } }, errors, dbcPath))
            return false;

        sChrModelStore.clear();
        for (uint32_t row = 0; row < chrModel.getRecordCount(); ++row)
        {
            WDB::Structures::ChrModelEntry entry;
            entry.id = chrModel.getRecordId(row);
            entry.sex = chrModel.getInt8(row, 3);
            entry.displayId = chrModel.getUInt32(row, 4);
            sChrModelStore[entry.id] = entry;
        }

        sChrRaceXChrModelStore.clear();
        for (uint32_t row = 0; row < chrRaceXChrModel.getRecordCount(); ++row)
        {
            WDB::Structures::ChrRaceXChrModelEntry entry;
            entry.id = chrRaceXChrModel.getRecordId(row);
            entry.chrRacesId = chrRaceXChrModel.getUInt8(row, 0);
            entry.chrModelId = chrRaceXChrModel.getInt32(row, 1);
            entry.sex = chrRaceXChrModel.getInt8(row, 2);
            entry.allowedTransmogSlots = chrRaceXChrModel.getInt32(row, 3);
            sChrRaceXChrModelStore[entry.id] = entry;
        }

        WDB::WDC5File chrClasses;
        WDB::WDC5File chrClassesXPowerTypes;
        WDB::WDC5File chrRaces;
        WDB::WDC5File faction;
        WDB::WDC5File factionTemplate;
        WDB::WDC5File chrSpecialization;

        if (!loadForeverWDC5Group({ { chrClasses, ForeverFormat::ChrClasses }, { chrClassesXPowerTypes, ForeverFormat::ChrClassesXPowerTypes }, { chrRaces, ForeverFormat::ChrRaces }, { faction, ForeverFormat::Faction }, { factionTemplate, ForeverFormat::FactionTemplate } }, errors, dbcPath))
            return false;

        if (!loadForeverGenericWDC5(chrSpecialization, "ChrSpecialization.db2", errors, dbcPath, {{12, 2}}))
            return false;

        if (chrSpecialization.getFieldCount() != 13)
        {
            errors.push_back("Forever DB2 ChrSpecialization.db2: expected 13 fields, got " + std::to_string(chrSpecialization.getFieldCount()));
            sLogger.failure("DB2: ChrSpecialization.db2 invalid (fields={}, expected=13).", chrSpecialization.getFieldCount());
            return false;
        }

        // Populate the existing AscEmu runtime stores. This deliberately keeps
        // Player and the rest of the core on the established WDBStore API while
        // replacing the legacy ChrClasses/ChrRaces DBC source for Forever.
        sChrClassesStore.clear();
        for (uint32_t row = 0; row < chrClasses.getRecordCount(); ++row)
        {
            WDB::Structures::ChrClassesEntry entry;
            entry.classId = chrClasses.getUInt8(row, 29);
            entry.powerType = static_cast<uint32_t>(static_cast<uint8_t>(chrClasses.getInt8(row, 32))); // DisplayPower
            entry.spellClassSet = chrClasses.getUInt8(row, 36);
            entry.cinematicSequenceId = chrClasses.getUInt32(row, 27);
            entry.apPerStr = chrClasses.getUInt8(row, 35);
            entry.apPerAgi = chrClasses.getUInt8(row, 34);
            entry.rapPerAgi = chrClasses.getUInt8(row, 33);
            sChrClassesStore[entry.classId] = std::move(entry);
        }

        sChrPowerTypesStore.clear();
        for (uint32_t row = 0; row < chrClassesXPowerTypes.getRecordCount(); ++row)
        {
            WDB::Structures::ChrPowerTypesEntry entry;
            entry.entry = chrClassesXPowerTypes.getRecordId(row);
            entry.classId = chrClassesXPowerTypes.getParentId(row);
            entry.power = chrClassesXPowerTypes.getUInt8(row, 0);
            sChrPowerTypesStore[entry.entry] = entry;
        }
        buildPowerIndexByClass();
        sLogger.info("DB2: ChrClassesXPowerTypes store ready (entries={}, warriorRageIndex={}, mageManaIndex={}, rogueComboIndex={}).", sChrPowerTypesStore.getNumRows(), powerIndexByClass[WARRIOR][POWER_TYPE_RAGE], powerIndexByClass[MAGE][POWER_TYPE_MANA], powerIndexByClass[ROGUE][POWER_TYPE_COMBO_POINTS]);

        std::memset(ClassSpecializationTabs, 0, sizeof(ClassSpecializationTabs));
        std::vector<std::pair<uint32_t, WDB::Structures::ChrSpecializationEntry>> specializationEntries;
        specializationEntries.reserve(chrSpecialization.getRecordCount());
        for (uint32_t row = 0; row < chrSpecialization.getRecordCount(); ++row)
        {
            WDB::Structures::ChrSpecializationEntry entry{};
            entry.Id = chrSpecialization.getRecordId(row);
            entry.classId = chrSpecialization.getUInt8(row, 4);
            entry.tabPage = static_cast<uint32_t>(static_cast<uint8_t>(chrSpecialization.getInt8(row, 5)));
            entry.petTabPage = static_cast<uint32_t>(static_cast<uint8_t>(chrSpecialization.getInt8(row, 6)));
            entry.masterySpellId = chrSpecialization.getUInt32(row, 12, 0);
            specializationEntries.emplace_back(entry.Id, entry);

            if (entry.classId < 12 && entry.tabPage < 4)
                ClassSpecializationTabs[entry.classId][entry.tabPage] = entry.Id;
        }
        sChrSpecializationStore.assignEntries(specializationEntries);
        sLogger.debugDbTables("DB2: ChrSpecialization store ready (entries={}).", sChrSpecializationStore.getNumRows());

        sChrRacesStore.clear();
        for (uint32_t row = 0; row < chrRaces.getRecordCount(); ++row)
        {
            WDB::Structures::ChrRacesEntry entry;
            entry.raceId = chrRaces.getRecordId(row);
            entry.flags = static_cast<uint32_t>(chrRaces.getInt32(row, 15));
            entry.factionId = static_cast<uint32_t>(chrRaces.getInt32(row, 16));
            entry.cinematicId = static_cast<uint32_t>(chrRaces.getInt32(row, 17));

            // Modern ChrRaces::Alliance: 0=Alliance, 1=Horde, 2=Neutral.
            // AscEmu's legacy runtime expects teamId 7 for Alliance and a
            // non-7 value for Horde. Preserve that established convention.
            int8_t const alliance = chrRaces.getInt8(row, 36);
            entry.teamId = alliance == 0 ? 7U : (alliance == 1 ? 1U : 0U);

            if (auto const* maleModel = getForeverChrModel(static_cast<uint8_t>(entry.raceId), 0))
                entry.modelMale = maleModel->displayId;
            if (auto const* femaleModel = getForeverChrModel(static_cast<uint8_t>(entry.raceId), 1))
                entry.modelFemale = femaleModel->displayId;

            sChrRacesStore[entry.raceId] = std::move(entry);
        }

        // Forever Faction.db2 keeps the four reputation relation sets in modern
        // array fields. Race masks are stored as two uint32 values per relation
        // (a 64-bit mask); AscEmu currently consumes the low 32 bits through its
        // established runtime race-mask API.
        sFactionStore.clear();
        for (uint32_t row = 0; row < faction.getRecordCount(); ++row)
        {
            WDB::Structures::FactionEntry entry;
            entry.id = faction.getRecordId(row);
            entry.reputationIndex = faction.getInt16(row, 2);
            entry.parentFactionId = faction.getUInt16(row, 3);
            entry.expansion = faction.getUInt8(row, 4);
            for (uint8_t i = 0; i < 4; ++i)
            {
                entry.reputationClassMask[i] = faction.getUInt16(row, 11, i);
                entry.reputationFlags[i] = faction.getUInt16(row, 12, i);
                entry.reputationBase[i] = faction.getInt32(row, 13, i);
                entry.reputationRaceMask[i] = faction.getUInt32(row, 17 + i, 0);
            }
            sFactionStore[entry.id] = std::move(entry);
        }

        // FactionTemplate.db2 keeps the currently verified Forever layout in this beta. The
        // modern client has eight explicit friend/enemy relations while the
        // existing AscEmu runtime structure stores four; copy the first four
        // and preserve the established core API.
        sFactionTemplateStore.clear();
        for (uint32_t row = 0; row < factionTemplate.getRecordCount(); ++row)
        {
            WDB::Structures::FactionTemplateEntry entry;
            entry.id = factionTemplate.getRecordId(row);
            entry.faction = factionTemplate.getUInt32(row, 0);
            entry.factionFlags = factionTemplate.getUInt32(row, 1);
            entry.ourMask = factionTemplate.getUInt8(row, 2);
            entry.friendlyMask = factionTemplate.getUInt8(row, 3);
            entry.hostileMask = factionTemplate.getUInt8(row, 4);

            for (std::size_t i = 0; i < WDB::Structures::maxFactionRelations; ++i)
            {
                entry.enemyFaction[i] = factionTemplate.getUInt32(row, 5, static_cast<uint32_t>(i));
                entry.friendFaction[i] = factionTemplate.getUInt32(row, 6, static_cast<uint32_t>(i));
            }

            sFactionTemplateStore[entry.id] = std::move(entry);
        }

        return true;
    }

    bool loadForeverModernCreatureStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDB::WDC5File creatureDisplayInfo;
        WDB::WDC5File creatureDisplayInfoExtra;
        WDB::WDC5File creatureModelData;

        bool const displayInfoLoaded = loadForeverWDC5(creatureDisplayInfo, ForeverFormat::CreatureDisplayInfo, errors, dbcPath);
        bool const displayInfoExtraLoaded = loadForeverWDC5(creatureDisplayInfoExtra, ForeverFormat::CreatureDisplayInfoExtra, errors, dbcPath);
        bool const modelDataLoaded = loadForeverWDC5(creatureModelData, ForeverFormat::CreatureModelData, errors, dbcPath);

        if (displayInfoLoaded)
        {
            sCreatureDisplayInfoStore.clear();
            for (uint32_t row = 0; row < creatureDisplayInfo.getRecordCount(); ++row)
            {
                WDB::Structures::CreatureDisplayInfoEntry entry;
                entry.id = creatureDisplayInfo.getRecordId(row);
                entry.modelId = creatureDisplayInfo.getUInt16(row, 1);
                entry.creatureModelScale = creatureDisplayInfo.getFloat(row, 4);
                entry.extendedDisplayInfoId = creatureDisplayInfo.getUInt32(row, 7);
                sCreatureDisplayInfoStore[entry.id] = std::move(entry);
            }
        }

        if (displayInfoExtraLoaded)
        {
            sCreatureDisplayInfoExtraStore.clear();
            for (uint32_t row = 0; row < creatureDisplayInfoExtra.getRecordCount(); ++row)
            {
                WDB::Structures::CreatureDisplayInfoExtraEntry entry;
                entry.displayExtraId = creatureDisplayInfoExtra.getRecordId(row);
                entry.race = creatureDisplayInfoExtra.getUInt8(row, 1);
                entry.displaySexId = creatureDisplayInfoExtra.getUInt8(row, 2);
                sCreatureDisplayInfoExtraStore[entry.displayExtraId] = std::move(entry);
            }
        }

        if (modelDataLoaded)
        {
            sCreatureModelDataStore.clear();
            for (uint32_t row = 0; row < creatureModelData.getRecordCount(); ++row)
            {
                WDB::Structures::CreatureModelDataEntry entry;
                entry.id = creatureModelData.getRecordId(row);
                entry.flags = creatureModelData.getUInt32(row, 1);
                entry.modelName.clear(); // Modern CreatureModelData.db2 no longer contains ModelName.
                entry.collisionHeight = creatureModelData.getFloat(row, 16);
                entry.modelScale = creatureModelData.getFloat(row, 21);
                entry.mountHeight = creatureModelData.getFloat(row, 25);
                sCreatureModelDataStore[entry.id] = std::move(entry);
            }
        }

        return displayInfoLoaded && displayInfoExtraLoaded && modelDataLoaded;
    }

    bool loadForeverModernCustomizationStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDB::WDC5File customization;
        WDB::WDC5File boneSet;
        WDB::WDC5File category;
        WDB::WDC5File choice;
        WDB::WDC5File condModel;
        WDB::WDC5File conversion;
        WDB::WDC5File displayInfo;
        WDB::WDC5File element;
        WDB::WDC5File geoset;
        WDB::WDC5File glyphPet;
        WDB::WDC5File material;
        WDB::WDC5File option;
        WDB::WDC5File req;
        WDB::WDC5File reqChoice;
        WDB::WDC5File skinnedModel;
        WDB::WDC5File visReq;
        WDB::WDC5File voice;

        // Only DB2 tables that actually populate runtime stores below are required.
        // Auxiliary Forever customization tables are loaded opportunistically until
        // their layouts are needed by the core. This prevents unsupported WDC5 metadata
        // variants in unused tables from blocking world startup.
        if (!loadForeverWDC5Group({
            { category, ForeverFormat::ChrCustomizationCategory }, { choice, ForeverFormat::ChrCustomizationChoice },
            { displayInfo, ForeverFormat::ChrCustomizationDisplayInfo }, { element, ForeverFormat::ChrCustomizationElement },
            { option, ForeverFormat::ChrCustomizationOption }, { req, ForeverFormat::ChrCustomizationReq },
            { reqChoice, ForeverFormat::ChrCustomizationReqChoice } }, errors, dbcPath))
            return false;

        loadForeverWDC5Optional(customization, ForeverFormat::ChrCustomization, dbcPath);
        loadForeverWDC5Optional(boneSet, ForeverFormat::ChrCustomizationBoneSet, dbcPath);
        loadForeverWDC5Optional(condModel, ForeverFormat::ChrCustomizationCondModel, dbcPath);
        loadForeverWDC5Optional(conversion, ForeverFormat::ChrCustomizationConversion, dbcPath);
        loadForeverWDC5Optional(geoset, ForeverFormat::ChrCustomizationGeoset, dbcPath);
        loadForeverWDC5Optional(glyphPet, ForeverFormat::ChrCustomizationGlyphPet, dbcPath);
        loadForeverWDC5Optional(material, ForeverFormat::ChrCustomizationMaterial, dbcPath);
        loadForeverWDC5Optional(skinnedModel, ForeverFormat::ChrCustomizationSkinnedModel, dbcPath);
        loadForeverWDC5Optional(visReq, ForeverFormat::ChrCustomizationVisReq, dbcPath);
        loadForeverWDC5Optional(voice, ForeverFormat::ChrCustomizationVoice, dbcPath);

        sChrCustomizationChoiceStore.clear();
        for (uint32_t row = 0; row < choice.getRecordCount(); ++row)
        {
            WDB::Structures::ChrCustomizationChoiceEntry entry;
            entry.id = choice.getRecordId(row);
            entry.optionId = choice.getUInt32(row, 2);
            entry.reqId = choice.getInt32(row, 3);
            entry.visReqId = choice.getInt32(row, 4);
            entry.sortOrder = choice.getUInt16(row, 5);
            entry.uiOrderIndex = choice.getUInt16(row, 6);
            entry.flags = choice.getInt32(row, 7);
            entry.addedInPatch = choice.getInt32(row, 8);
            entry.soundKitId = choice.getInt32(row, 9);
            entry.swatchColor[0] = choice.getInt32(row, 10, 0);
            entry.swatchColor[1] = choice.getInt32(row, 10, 1);
            sChrCustomizationChoiceStore[entry.id] = entry;
        }

        sChrCustomizationDisplayInfoStore.clear();
        for (uint32_t row = 0; row < displayInfo.getRecordCount(); ++row)
        {
            WDB::Structures::ChrCustomizationDisplayInfoEntry entry;
            entry.id = displayInfo.getRecordId(row);
            entry.shapeshiftFormId = displayInfo.getInt32(row, 0);
            entry.displayId = displayInfo.getInt32(row, 1);
            entry.barberShopMinCameraDistance = displayInfo.getFloat(row, 2);
            entry.barberShopHeightOffset = displayInfo.getFloat(row, 3);
            entry.barberShopCameraZoomOffset = displayInfo.getFloat(row, 4);
            sChrCustomizationDisplayInfoStore[entry.id] = entry;
        }

        sChrCustomizationElementStore.clear();
        for (uint32_t row = 0; row < element.getRecordCount(); ++row)
        {
            WDB::Structures::ChrCustomizationElementEntry entry;
            entry.id = element.getRecordId(row);
            entry.choiceId = element.getInt32(row, 0);
            entry.relatedChoiceId = element.getInt32(row, 1);
            entry.geosetId = element.getInt32(row, 2);
            entry.skinnedModelId = element.getInt32(row, 3);
            entry.materialId = element.getInt32(row, 4);
            entry.boneSetId = element.getInt32(row, 5);
            entry.condModelId = element.getInt32(row, 6);
            entry.displayInfoId = element.getInt32(row, 7);
            entry.itemGeoModifyId = element.getInt32(row, 8);
            entry.voiceId = element.getInt32(row, 9);
            entry.animKitId = element.getInt32(row, 10);
            entry.particleColorId = element.getInt32(row, 11);
            entry.geoComponentLinkId = element.getInt32(row, 12);
            sChrCustomizationElementStore[entry.id] = entry;
        }

        sChrCustomizationOptionStore.clear();
        for (uint32_t row = 0; row < option.getRecordCount(); ++row)
        {
            WDB::Structures::ChrCustomizationOptionEntry entry;
            entry.id = option.getRecordId(row);
            entry.secondaryId = option.getUInt16(row, 2);
            entry.flags = option.getInt32(row, 3);
            entry.chrModelId = option.getUInt32(row, 4);
            entry.sortIndex = option.getInt32(row, 5);
            entry.categoryId = option.getInt32(row, 6);
            entry.optionType = option.getInt32(row, 7);
            entry.barberShopCostModifier = option.getFloat(row, 8);
            entry.chrCustomizationId = option.getInt32(row, 9);
            entry.reqId = option.getInt32(row, 10);
            entry.uiOrderIndex = option.getInt32(row, 11);
            entry.addedInPatch = option.getInt32(row, 12);
            sChrCustomizationOptionStore[entry.id] = entry;
        }

        sChrCustomizationReqStore.clear();
        for (uint32_t row = 0; row < req.getRecordCount(); ++row)
        {
            WDB::Structures::ChrCustomizationReqEntry entry;
            entry.id = req.getRecordId(row);
            entry.flags = req.getInt32(row, 1);
            entry.classMask = req.getInt32(row, 2);
            entry.regionGroupMask = req.getInt32(row, 3);
            entry.achievementId = req.getInt32(row, 4);
            entry.questId = req.getInt32(row, 5);
            entry.overrideArchive = req.getInt32(row, 6);
            entry.itemModifiedAppearanceId = req.getInt32(row, 7);
            entry.raceMask[0] = req.getInt32(row, 8, 0);
            entry.raceMask[1] = req.getInt32(row, 8, 1);
            sChrCustomizationReqStore[entry.id] = entry;
        }

        sChrCustomizationReqChoiceStore.clear();
        for (uint32_t row = 0; row < reqChoice.getRecordCount(); ++row)
        {
            WDB::Structures::ChrCustomizationReqChoiceEntry entry;
            entry.id = reqChoice.getRecordId(row);
            entry.choiceId = reqChoice.getInt32(row, 0);
            entry.reqId = reqChoice.getParentId(row);
            sChrCustomizationReqChoiceStore[entry.id] = entry;
        }

        return true;
    }

    bool loadForeverModernTaxiStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDB::WDC5File taxiNodes;
        WDB::WDC5File taxiPath;
        WDB::WDC5File taxiPathNode;
        if (!loadForeverWDC5Group({ { taxiNodes, ForeverFormat::TaxiNodes }, { taxiPath, ForeverFormat::TaxiPath }, { taxiPathNode, ForeverFormat::TaxiPathNode } }, errors, dbcPath))
            return false;

        std::vector<std::pair<uint32_t, WDB::Structures::TaxiNodesEntry>> nodeEntries;
        nodeEntries.reserve(taxiNodes.getRecordCount());
        for (uint32_t row = 0; row < taxiNodes.getRecordCount(); ++row)
        {
            WDB::Structures::TaxiNodesEntry entry{};
            entry.id = taxiNodes.getRecordId(row);
            entry.mapid = taxiNodes.getUInt16(row, 5); // ContinentID
            entry.x = taxiNodes.getFloat(row, 1, 0);
            entry.y = taxiNodes.getFloat(row, 1, 1);
            entry.z = taxiNodes.getFloat(row, 1, 2);
            entry.mountCreatureID[0] = static_cast<uint32_t>(taxiNodes.getInt32(row, 14, 0));
            entry.mountCreatureID[1] = static_cast<uint32_t>(taxiNodes.getInt32(row, 14, 1));
#if VERSION_STRING >= Cata
            entry.flags = static_cast<uint32_t>(taxiNodes.getInt32(row, 8));
#endif
            nodeEntries.emplace_back(entry.id, entry);
        }
        sTaxiNodesStore.assignEntries(nodeEntries);

        std::vector<std::pair<uint32_t, WDB::Structures::TaxiPathEntry>> pathEntries;
        pathEntries.reserve(taxiPath.getRecordCount());
        for (uint32_t row = 0; row < taxiPath.getRecordCount(); ++row)
        {
            WDB::Structures::TaxiPathEntry entry{};
            entry.id = taxiPath.getRecordId(row);
            entry.from = taxiPath.getUInt16(row, 1);
            entry.to = taxiPath.getUInt16(row, 2);
            entry.price = taxiPath.getUInt32(row, 3);
            pathEntries.emplace_back(entry.id, entry);
        }
        sTaxiPathStore.assignEntries(pathEntries);

        std::vector<std::pair<uint32_t, WDB::Structures::TaxiPathNodeEntry>> pathNodeEntries;
        pathNodeEntries.reserve(taxiPathNode.getRecordCount());
        for (uint32_t row = 0; row < taxiPathNode.getRecordCount(); ++row)
        {
            WDB::Structures::TaxiPathNodeEntry entry{};
            entry.pathId = taxiPathNode.getUInt32(row, 2);
            entry.NodeIndex = static_cast<uint32_t>(taxiPathNode.getInt32(row, 3));
            entry.mapid = taxiPathNode.getUInt16(row, 4);
            entry.x = taxiPathNode.getFloat(row, 0, 0);
            entry.y = taxiPathNode.getFloat(row, 0, 1);
            entry.z = taxiPathNode.getFloat(row, 0, 2);
            entry.flags = static_cast<uint32_t>(taxiPathNode.getInt32(row, 5));
            entry.waittime = taxiPathNode.getUInt32(row, 6);
#if VERSION_STRING >= TBC
            entry.arivalEventID = static_cast<uint32_t>(taxiPathNode.getInt32(row, 7));
            entry.departureEventID = static_cast<uint32_t>(taxiPathNode.getInt32(row, 8));
#endif
            pathNodeEntries.emplace_back(taxiPathNode.getRecordId(row), entry);
        }
        sTaxiPathNodeStore.assignEntries(pathNodeEntries);

        return !nodeEntries.empty() && !pathEntries.empty() && !pathNodeEntries.empty();
    }


    bool loadForeverModernItemStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDB::WDC5File item;
        WDB::WDC5File itemSparse;
        WDB::WDC5File itemAppearance;
        WDB::WDC5File itemExtendedCost;
        WDB::WDC5File itemModifiedAppearance;
        WDB::WDC5File itemSet;
        WDB::WDC5File itemSetSpell;
        if (!loadForeverWDC5Group({
                { item, ForeverFormat::Item },
                { itemSparse, ForeverFormat::ItemSparse },
                { itemAppearance, ForeverFormat::ItemAppearance },
                { itemExtendedCost, ForeverFormat::ItemExtendedCost },
                { itemModifiedAppearance, ForeverFormat::ItemModifiedAppearance },
                { itemSet, ForeverFormat::ItemSet },
                { itemSetSpell, ForeverFormat::ItemSetSpell }
            }, errors, dbcPath))
            return false;

        std::unordered_map<uint32_t, uint32_t> displayInfoByAppearanceId;
        displayInfoByAppearanceId.reserve(itemAppearance.getRecordCount());

        for (uint32_t row = 0; row < itemAppearance.getRecordCount(); ++row)
        {
            uint32_t const appearanceId = itemAppearance.getRecordId(row);
            uint32_t const displayInfoId = itemAppearance.getUInt32(row, 1);
            displayInfoByAppearanceId.emplace(appearanceId, displayInfoId);
        }

        std::unordered_map<uint32_t, uint32_t> displayInfoByItemId;
        displayInfoByItemId.reserve(itemModifiedAppearance.getRecordCount());

        for (uint32_t row = 0; row < itemModifiedAppearance.getRecordCount(); ++row)
        {
            uint32_t const itemId = itemModifiedAppearance.getUInt32(row, 1);
            uint32_t const appearanceModifierId = itemModifiedAppearance.getUInt32(row, 2);
            uint32_t const appearanceId = itemModifiedAppearance.getUInt32(row, 3);

            // ItemEntry exposes a single legacy DisplayId. Prefer the base,
            // unmodified appearance for that compatibility value.
            if (appearanceModifierId != 0)
                continue;

            auto const appearanceItr = displayInfoByAppearanceId.find(appearanceId);
            if (appearanceItr == displayInfoByAppearanceId.end())
                continue;

            displayInfoByItemId.try_emplace(itemId, appearanceItr->second);
        }

        std::vector<std::pair<uint32_t, WDB::Structures::ItemEntry>> itemEntries;
        itemEntries.reserve(item.getRecordCount());

        for (uint32_t row = 0; row < item.getRecordCount(); ++row)
        {
            WDB::Structures::ItemEntry entry{};
            entry.ID = item.getRecordId(row);
            entry.Class = item.getUInt32(row, 0);
            entry.SubClass = item.getUInt8(row, 1);
            entry.Material = item.getUInt8(row, 2);
            entry.InventoryType = item.getUInt8(row, 3);
            entry.Sheath = item.getUInt8(row, 4);
            entry.SoundOverrideSubclass = item.getInt8(row, 6);

            auto const displayItr = displayInfoByItemId.find(entry.ID);
            entry.DisplayId = displayItr != displayInfoByItemId.end() ? displayItr->second : 0;

            itemEntries.emplace_back(entry.ID, entry);
        }

        sItemStore.assignEntries(itemEntries);
        sLogger.info(
            "DB2: Item store ready (items={}, defaultDisplayIds={}).",
            itemEntries.size(),
            displayInfoByItemId.size());

        std::vector<std::pair<uint32_t, WDB::Structures::ItemSparseEntry>> sparseEntries;
        sparseEntries.reserve(itemSparse.getRecordCount());
        for (uint32_t row = 0; row < itemSparse.getRecordCount(); ++row)
        {
            WDB::Structures::ItemSparseEntry entry{};
            entry.ID = itemSparse.getRecordId(row);
            entry.Description = std::string(itemSparse.getString(row, 0));
            entry.Name = std::string(itemSparse.getString(row, 4));
            entry.ExpansionID = itemSparse.getUInt32(row, 5);
            entry.DmgVariance = itemSparse.getFloat(row, 6);
            entry.LimitCategory = itemSparse.getUInt32(row, 7);
            entry.DurationInInventory = itemSparse.getUInt32(row, 8);
            entry.QualityModifier = itemSparse.getFloat(row, 9);
            entry.BagFamily = itemSparse.getUInt32(row, 10);
            entry.StartQuestID = itemSparse.getUInt32(row, 11);
            entry.LanguageID = itemSparse.getUInt32(row, 12);
            entry.ItemRange = itemSparse.getFloat(row, 13);

            for (uint32_t i = 0; i < 10; ++i)
            {
                entry.StatPercentageOfSocket[i] = itemSparse.getFloat(row, 14, i);
                entry.StatPercentEditor[i] = itemSparse.getInt32(row, 15, i);
                entry.StatModifierBonusStat[i] = itemSparse.getInt32(row, 16, i);
            }

            entry.Stackable = itemSparse.getInt32(row, 17);
            entry.MaxCount = itemSparse.getInt32(row, 18);
            entry.MinReputation = itemSparse.getInt32(row, 19);
            entry.RequiredAbility = itemSparse.getUInt32(row, 20);
            for (uint32_t i = 0; i < 2; ++i)
                entry.AllowableRace[i] = itemSparse.getUInt32(row, 21, i);
            entry.SellPrice = itemSparse.getUInt32(row, 22);
            entry.BuyPrice = itemSparse.getUInt32(row, 23);
            entry.VendorStackCount = itemSparse.getUInt32(row, 24);
            entry.PriceVariance = itemSparse.getFloat(row, 25);
            entry.PriceRandomValue = itemSparse.getFloat(row, 26);
            for (uint32_t i = 0; i < 5; ++i)
                entry.Flags[i] = itemSparse.getUInt32(row, 27, i);
            entry.OppositeFactionItemID = itemSparse.getUInt32(row, 28);
            entry.ModifiedCraftingReagentItemID = itemSparse.getUInt32(row, 29);
            entry.ContentTuningID = itemSparse.getUInt32(row, 30);
            entry.PlayerLevelToItemLevelCurveID = itemSparse.getUInt32(row, 31);
            entry.ItemLevelOffsetCurveID = itemSparse.getUInt32(row, 32);
            entry.ItemLevelOffsetItemLevel = itemSparse.getInt32(row, 33);
            entry.ItemSquishEraID = itemSparse.getUInt32(row, 34);
            entry.ItemNameDescriptionID = itemSparse.getUInt16(row, 35);
            entry.RequiredTransmogHoliday = itemSparse.getUInt16(row, 36);
            entry.RequiredHoliday = itemSparse.getUInt16(row, 37);
            entry.GemProperties = itemSparse.getUInt16(row, 38);
            entry.SocketMatchEnchantmentID = itemSparse.getUInt16(row, 39);
            entry.TotemCategoryID = itemSparse.getUInt16(row, 40);
            entry.InstanceBound = itemSparse.getUInt16(row, 41);
            for (uint32_t i = 0; i < 2; ++i)
                entry.ZoneBound[i] = itemSparse.getUInt16(row, 42, i);
            entry.ItemSet = itemSparse.getUInt16(row, 43);
            entry.LockID = itemSparse.getUInt16(row, 44);
            entry.PageID = itemSparse.getUInt16(row, 45);
            entry.ItemDelay = itemSparse.getUInt16(row, 46);
            entry.MinFactionID = itemSparse.getUInt16(row, 47);
            entry.RequiredSkillRank = itemSparse.getUInt16(row, 48);
            entry.RequiredSkill = itemSparse.getUInt16(row, 49);
            entry.ItemLevel = itemSparse.getUInt16(row, 50);
            entry.AllowableClass = itemSparse.getInt16(row, 51);
            entry.ArtifactID = itemSparse.getUInt8(row, 52);
            entry.SpellWeight = itemSparse.getUInt8(row, 53);
            entry.SpellWeightCategory = itemSparse.getUInt8(row, 54);
            for (uint32_t i = 0; i < 3; ++i)
                entry.SocketType[i] = itemSparse.getUInt8(row, 55, i);
            entry.SheatheType = itemSparse.getUInt8(row, 56);
            entry.Material = itemSparse.getUInt8(row, 57);
            entry.PageMaterialID = itemSparse.getUInt8(row, 58);
            entry.Bonding = itemSparse.getUInt8(row, 59);
            entry.DamageType = itemSparse.getUInt8(row, 60);
            entry.ContainerSlots = itemSparse.getUInt8(row, 61);
            entry.RequiredPVPMedal = itemSparse.getUInt8(row, 62);
            entry.RequiredPVPRank = itemSparse.getInt8(row, 63);
            entry.RequiredLevel = itemSparse.getInt8(row, 64);
            entry.InventoryType = itemSparse.getInt8(row, 65);
            entry.OverallQualityID = itemSparse.getInt8(row, 66);
            entry.AmmunitionType = itemSparse.getUInt8(row, 67);

            sparseEntries.emplace_back(entry.ID, std::move(entry));
        }
        sItemSparseStore.assignEntries(sparseEntries);
        sLogger.info("DB2: ItemSparse store ready (entries={}).", sparseEntries.size());

        WDB::WDC5File itemBonus;
        if (loadForeverWDC5Optional(itemBonus, ForeverFormat::ItemBonus, dbcPath))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::ItemBonusEntry>> entries;
            entries.reserve(itemBonus.getRecordCount());
            for (uint32_t row = 0; row < itemBonus.getRecordCount(); ++row)
            {
                WDB::Structures::ItemBonusEntry entry{};
                entry.ID = itemBonus.getRecordId(row);
                for (uint32_t i = 0; i < 4; ++i)
                    entry.Value[i] = itemBonus.getInt32(row, 0, i);
                entry.ParentItemBonusListID = itemBonus.getUInt16(row, 1);
                entry.Type = itemBonus.getUInt8(row, 2);
                entry.OrderIndex = itemBonus.getUInt8(row, 3);
                entries.emplace_back(entry.ID, entry);
            }
            sItemBonusStore.assignEntries(entries);
            sLogger.info("DB2: ItemBonus store ready (entries={}).", entries.size());
        }

        WDB::WDC5File durabilityCosts;
        if (loadForeverWDC5Optional(durabilityCosts, ForeverFormat::DurabilityCosts, dbcPath))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::DurabilityCostsEntry>> entries;
            entries.reserve(durabilityCosts.getRecordCount());
            for (uint32_t row = 0; row < durabilityCosts.getRecordCount(); ++row)
            {
                WDB::Structures::DurabilityCostsEntry entry{};
                entry.itemLevel = durabilityCosts.getRecordId(row);
                for (uint32_t i = 0; i < 21; ++i)
                    entry.modifier[i] = durabilityCosts.getUInt32(row, 0, i);
                for (uint32_t i = 0; i < 8; ++i)
                    entry.modifier[21 + i] = durabilityCosts.getUInt32(row, 1, i);
                entries.emplace_back(entry.itemLevel, entry);
            }
            sDurabilityCostsStore.assignEntries(entries);
            sLogger.info("DB2: DurabilityCosts store ready (itemLevels={}).", entries.size());
        }

        WDB::WDC5File durabilityQuality;
        if (loadForeverWDC5Optional(durabilityQuality, ForeverFormat::DurabilityQuality, dbcPath))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::DurabilityQualityEntry>> entries;
            entries.reserve(durabilityQuality.getRecordCount());
            for (uint32_t row = 0; row < durabilityQuality.getRecordCount(); ++row)
            {
                WDB::Structures::DurabilityQualityEntry entry{};
                entry.id = durabilityQuality.getRecordId(row);
                entry.qualityModifier = durabilityQuality.getFloat(row, 0);
                entries.emplace_back(entry.id, entry);
            }
            sDurabilityQualityStore.assignEntries(entries);
            sLogger.info("DB2: DurabilityQuality store ready (entries={}).", entries.size());
        }

        WDB::WDC5File randPropPoints;
        if (loadForeverWDC5Optional(randPropPoints, ForeverFormat::RandPropPoints, dbcPath))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::RandPropPointsEntry>> randPropEntries;
            randPropEntries.reserve(randPropPoints.getRecordCount());
            for (uint32_t row = 0; row < randPropPoints.getRecordCount(); ++row)
            {
                WDB::Structures::RandPropPointsEntry entry{};
                entry.ID = randPropPoints.getRecordId(row);
                entry.DamageReplaceStatF = randPropPoints.getFloat(row, 0);
                entry.DamageSecondaryF = randPropPoints.getFloat(row, 1);
                entry.DamageReplaceStat = randPropPoints.getInt32(row, 2);
                entry.DamageSecondary = randPropPoints.getInt32(row, 3);
                for (uint32_t i = 0; i < 5; ++i)
                {
                    entry.EpicF[i] = randPropPoints.getFloat(row, 4, i);
                    entry.SuperiorF[i] = randPropPoints.getFloat(row, 5, i);
                    entry.GoodF[i] = randPropPoints.getFloat(row, 6, i);
                    entry.Epic[i] = randPropPoints.getUInt32(row, 7, i);
                    entry.Superior[i] = randPropPoints.getUInt32(row, 8, i);
                    entry.Good[i] = randPropPoints.getUInt32(row, 9, i);
                }
                randPropEntries.emplace_back(entry.ID, entry);
            }
            sRandPropPointsStore.assignEntries(randPropEntries);
            sLogger.info("DB2: RandPropPoints store ready (entries={}).", randPropEntries.size());
        }

        WDB::WDC5File armorLocation;
        if (loadForeverWDC5Optional(armorLocation, ForeverFormat::ArmorLocation, dbcPath))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::ArmorLocationEntry>> entries;
            entries.reserve(armorLocation.getRecordCount());
            for (uint32_t row = 0; row < armorLocation.getRecordCount(); ++row)
            {
                WDB::Structures::ArmorLocationEntry entry{};
                entry.ID = armorLocation.getRecordId(row);
                for (uint32_t i = 0; i < 4; ++i)
                    entry.ArmorModifier[i] = armorLocation.getFloat(row, i);
                entry.Modifier = armorLocation.getFloat(row, 4);
                entries.emplace_back(entry.ID, entry);
            }
            sArmorLocationForeverStore.assignEntries(entries);
            sLogger.info("DB2: ArmorLocation store ready (entries={}).", entries.size());
        }

        WDB::WDC5File itemArmorQuality;
        if (loadForeverWDC5Optional(itemArmorQuality, ForeverFormat::ItemArmorQuality, dbcPath))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::ItemArmorQualityEntry>> entries;
            entries.reserve(itemArmorQuality.getRecordCount());
            for (uint32_t row = 0; row < itemArmorQuality.getRecordCount(); ++row)
            {
                WDB::Structures::ItemArmorQualityEntry entry{};
                entry.ID = itemArmorQuality.getRecordId(row);
                for (uint32_t i = 0; i < 7; ++i)
                    entry.Quality[i] = itemArmorQuality.getFloat(row, 0, i);
                entries.emplace_back(entry.ID, entry);
            }
            sItemArmorQualityForeverStore.assignEntries(entries);
            sLogger.info("DB2: ItemArmorQuality store ready (entries={}).", entries.size());
        }

        WDB::WDC5File itemArmorShield;
        if (loadForeverWDC5Optional(itemArmorShield, ForeverFormat::ItemArmorShield, dbcPath))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::ItemArmorShieldEntry>> entries;
            entries.reserve(itemArmorShield.getRecordCount());
            for (uint32_t row = 0; row < itemArmorShield.getRecordCount(); ++row)
            {
                WDB::Structures::ItemArmorShieldEntry entry{};
                entry.ID = itemArmorShield.getRecordId(row);
                for (uint32_t i = 0; i < 7; ++i)
                    entry.Quality[i] = itemArmorShield.getFloat(row, 0, i);
                entry.ItemLevel = itemArmorShield.getUInt32(row, 1);
                entries.emplace_back(entry.ID, entry);
            }
            sItemArmorShieldForeverStore.assignEntries(entries);
            sLogger.info("DB2: ItemArmorShield store ready (entries={}).", entries.size());
        }

        WDB::WDC5File itemArmorTotal;
        if (loadForeverWDC5Optional(itemArmorTotal, ForeverFormat::ItemArmorTotal, dbcPath))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::ItemArmorTotalEntry>> entries;
            entries.reserve(itemArmorTotal.getRecordCount());
            for (uint32_t row = 0; row < itemArmorTotal.getRecordCount(); ++row)
            {
                WDB::Structures::ItemArmorTotalEntry entry{};
                entry.ID = itemArmorTotal.getRecordId(row);
                entry.ItemLevel = itemArmorTotal.getUInt32(row, 0);
                for (uint32_t i = 0; i < 4; ++i)
                    entry.Armor[i] = itemArmorTotal.getFloat(row, i + 1);
                entries.emplace_back(entry.ID, entry);
            }
            sItemArmorTotalForeverStore.assignEntries(entries);
            sLogger.info("DB2: ItemArmorTotal store ready (entries={}).", entries.size());
        }

        WDB::WDC5File itemDamageThrown;
        if (loadForeverWDC5Optional(itemDamageThrown, ForeverFormat::ItemDamageThrown, dbcPath))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::ItemDamageEntry>> entries;
            entries.reserve(itemDamageThrown.getRecordCount());
            for (uint32_t row = 0; row < itemDamageThrown.getRecordCount(); ++row)
            {
                WDB::Structures::ItemDamageEntry entry{};
                entry.ID = itemDamageThrown.getRecordId(row);
                entry.ItemLevel = itemDamageThrown.getUInt32(row, 0);
                for (uint32_t i = 0; i < 7; ++i)
                    entry.Quality[i] = itemDamageThrown.getFloat(row, 1, i);
                entries.emplace_back(entry.ID, entry);
            }
            sItemDamageThrownForeverStore.assignEntries(entries);
            sLogger.info("DB2: ItemDamageThrown store ready (entries={}).", entries.size());
        }

        WDB::WDC5File itemEffect;
        if (loadForeverWDC5Optional(itemEffect, ForeverFormat::ItemEffect, dbcPath))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::ItemEffectEntry>> entries;
            entries.reserve(itemEffect.getRecordCount());
            for (uint32_t row = 0; row < itemEffect.getRecordCount(); ++row)
            {
                WDB::Structures::ItemEffectEntry entry{};
                entry.ID = itemEffect.getRecordId(row);
                entry.LegacySlotIndex = itemEffect.getInt32(row, 0);
                entry.TriggerType = itemEffect.getInt32(row, 1);
                entry.Charges = static_cast<int32_t>(itemEffect.getInt16(row, 2));
                entry.Cooldown = itemEffect.getInt32(row, 3);
                entry.CategoryCooldown = itemEffect.getInt32(row, 4);
                entry.Category = itemEffect.getUInt32(row, 5);
                entry.SpellID = itemEffect.getUInt32(row, 6);
                entry.ChrSpecializationID = itemEffect.getUInt32(row, 7);
                entry.PlayerConditionID = itemEffect.getUInt32(row, 8);
                entries.emplace_back(entry.ID, entry);
            }
            sItemEffectForeverStore.assignEntries(entries);
            sLogger.info("DB2: ItemEffect store ready (entries={}).", entries.size());
        }

        WDB::WDC5File itemXItemEffect;
        if (loadForeverWDC5Optional(itemXItemEffect, ForeverFormat::ItemXItemEffect, dbcPath))
        {
            std::vector<std::pair<uint32_t, WDB::Structures::ItemXItemEffectEntry>> entries;
            entries.reserve(itemXItemEffect.getRecordCount());
            for (uint32_t row = 0; row < itemXItemEffect.getRecordCount(); ++row)
            {
                WDB::Structures::ItemXItemEffectEntry entry{};
                entry.ID = itemXItemEffect.getRecordId(row);
                entry.ItemEffectID = itemXItemEffect.getUInt32(row, 0);
                entry.ItemID = itemXItemEffect.getParentId(row);
                entries.emplace_back(entry.ID, entry);
            }
            sItemXItemEffectForeverStore.assignEntries(entries);
            sLogger.info("DB2: ItemXItemEffect store ready (entries={}).", entries.size());
        }

        std::vector<std::pair<uint32_t, WDB::Structures::ItemExtendedCostEntry>> extendedCostEntries;
        extendedCostEntries.reserve(itemExtendedCost.getRecordCount());

        for (uint32_t row = 0; row < itemExtendedCost.getRecordCount(); ++row)
        {
            WDB::Structures::ItemExtendedCostEntry entry{};
            entry.costid = itemExtendedCost.getRecordId(row);
            entry.honor_points = 0;
            entry.arena_points = 0;
            entry.personalrating = itemExtendedCost.getUInt16(row, 1);
            entry.arena_slot = itemExtendedCost.getUInt8(row, 2);

            for (uint32_t i = 0; i < 5; ++i)
            {
                entry.item[i] = itemExtendedCost.getUInt32(row, 7, i);
                entry.count[i] = itemExtendedCost.getUInt16(row, 8, i);
                entry.reqcur[i] = itemExtendedCost.getUInt16(row, 9, i);
                entry.reqcurrcount[i] = itemExtendedCost.getUInt32(row, 10, i);
            }

            extendedCostEntries.emplace_back(entry.costid, entry);
        }

        sItemExtendedCostStore.assignEntries(extendedCostEntries);
        sLogger.info("DB2: ItemExtendedCost store ready (entries={}).", extendedCostEntries.size());

        std::vector<std::pair<uint32_t, WDB::Structures::ItemSetEntry>> entries;
        entries.reserve(itemSet.getRecordCount());
        std::map<uint32_t, size_t> entryIndexById;

        for (uint32_t row = 0; row < itemSet.getRecordCount(); ++row)
        {
            WDB::Structures::ItemSetEntry entry{};
            entry.id = itemSet.getRecordId(row);
            entry.RequiredSkillID = itemSet.getUInt32(row, 2);
            entry.RequiredSkillAmt = itemSet.getUInt16(row, 3);

            // Modern ItemSet has 17 item IDs while the legacy AscEmu view has 10.
            // Preserve the first ten IDs used by the existing core API.
            for (uint32_t i = 0; i < 10; ++i)
                entry.itemid[i] = itemSet.getUInt32(row, 4, i);

            entryIndexById.emplace(entry.id, entries.size());
            entries.emplace_back(entry.id, entry);
        }

        std::map<uint32_t, uint32_t> bonusCountBySet;

        for (uint32_t row = 0; row < itemSetSpell.getRecordCount(); ++row)
        {
            uint32_t const itemSetId = itemSetSpell.getParentId(row);
            auto const indexItr = entryIndexById.find(itemSetId);
            if (indexItr == entryIndexById.end())
                continue;

            uint16_t const chrSpecId = itemSetSpell.getUInt16(row, 0);
            uint16_t const traitSubTreeId = itemSetSpell.getUInt16(row, 2);
            if (chrSpecId != 0 || traitSubTreeId != 0)
            {
                continue;
            }

            uint32_t& bonusIndex = bonusCountBySet[itemSetId];
            if (bonusIndex >= 8)
                continue;

            auto& entry = entries[indexItr->second].second;
            entry.SpellID[bonusIndex] = itemSetSpell.getUInt32(row, 1);
            entry.itemscount[bonusIndex] = itemSetSpell.getUInt8(row, 3);
            ++bonusIndex;
        }

        sItemSetStore.assignEntries(entries);

        // These two client tables no longer exist in the Forever beta / modern
        // Forever DB2 set. Keep the legacy stores available to old AscEmu code,
        // but intentionally empty instead of reporting missing .dbc files.
        sItemRandomPropertiesStore.clear();
        sItemRandomSuffixStore.clear();

        return true;
    }

    bool loadForeverAchievementStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDB::WDC5File achievement, criteria, criteriaTree, modifierTree;
        if (!loadForeverGenericWDC5(achievement, "Achievement.db2", errors, dbcPath)
            || !loadForeverGenericWDC5(criteria, "Criteria.db2", errors, dbcPath)
            || !loadForeverGenericWDC5(criteriaTree, "CriteriaTree.db2", errors, dbcPath)
            || !loadForeverGenericWDC5(modifierTree, "ModifierTree.db2", errors, dbcPath))
            return false;

        auto verify = [&](WDB::WDC5File const& file, char const* name, uint32_t logicalFields)
        {
            bool const externalId = file.hasExternalRecordIds() || file.getIndexField() < 0;
            uint32_t const expected = externalId ? logicalFields - 1 : logicalFields;
            if (file.getFieldCount() == expected)
                return true;
            errors.push_back(std::string("Forever DB2 ") + name + ": unexpected field count " + std::to_string(file.getFieldCount()) + " (expected " + std::to_string(expected) + ")");
            sLogger.failure("DB2: {} invalid (fields={}, expected={}).", name, file.getFieldCount(), expected);
            return false;
        };

        if (!verify(achievement, "Achievement.db2", 19) || !verify(criteria, "Criteria.db2", 12)
            || !verify(criteriaTree, "CriteriaTree.db2", 8) || !verify(modifierTree, "ModifierTree.db2", 8))
            return false;

        auto field = [](WDB::WDC5File const& file, uint32_t logical, uint32_t idLogical)
        {
            bool const externalId = file.hasExternalRecordIds() || file.getIndexField() < 0;
            if (!externalId || logical < idLogical)
                return logical;
            return logical > idLogical ? logical - 1 : logical;
        };

        sForeverAchievementStore.clear();
        for (uint32_t row = 0; row < achievement.getRecordCount(); ++row)
        {
            WDB::Structures::ForeverAchievementEntry e{};
            e.id = achievement.getRecordId(row);
            e.faction = achievement.getInt8(row, field(achievement, 5, 3));
            e.flags = achievement.getInt32(row, field(achievement, 10, 3));
            e.criteriaTreeId = achievement.getUInt32(row, field(achievement, 14, 3));
            sForeverAchievementStore[e.id] = e;
        }

        sForeverCriteriaStore.clear();
        for (uint32_t row = 0; row < criteria.getRecordCount(); ++row)
        {
            WDB::Structures::ForeverCriteriaEntry e{};
            e.id = criteria.getRecordId(row);
            e.type = criteria.getInt16(row, field(criteria, 1, 0));
            e.asset = criteria.getInt32(row, field(criteria, 2, 0));
            e.modifierTreeId = criteria.getInt32(row, field(criteria, 3, 0));
            sForeverCriteriaStore[e.id] = e;
        }

        sForeverCriteriaTreeStore.clear();
        for (uint32_t row = 0; row < criteriaTree.getRecordCount(); ++row)
        {
            WDB::Structures::ForeverCriteriaTreeEntry e{};
            e.id = criteriaTree.getRecordId(row);
            e.parent = criteriaTree.getUInt32(row, field(criteriaTree, 2, 0));
            e.amount = criteriaTree.getUInt32(row, field(criteriaTree, 3, 0));
            e.op = criteriaTree.getInt32(row, field(criteriaTree, 4, 0));
            e.criteriaId = criteriaTree.getUInt32(row, field(criteriaTree, 5, 0));
            sForeverCriteriaTreeStore[e.id] = e;
        }

        sForeverModifierTreeStore.clear();
        for (uint32_t row = 0; row < modifierTree.getRecordCount(); ++row)
        {
            WDB::Structures::ForeverModifierTreeEntry e{};
            e.id = modifierTree.getRecordId(row);
            e.parent = modifierTree.getUInt32(row, field(modifierTree, 1, 0));
            e.op = modifierTree.getInt8(row, field(modifierTree, 2, 0));
            e.amount = modifierTree.getInt8(row, field(modifierTree, 3, 0));
            e.type = modifierTree.getInt32(row, field(modifierTree, 4, 0));
            e.asset = modifierTree.getInt32(row, field(modifierTree, 5, 0));
            e.secondaryAsset = modifierTree.getInt32(row, field(modifierTree, 6, 0));
            e.tertiaryAsset = modifierTree.getInt32(row, field(modifierTree, 7, 0));
            sForeverModifierTreeStore[e.id] = e;
        }

        sLogger.debugDbTables("DB2: Achievement stores ready (achievements={}, criteria={}, trees={}, modifiers={}).", sForeverAchievementStore.size(), sForeverCriteriaStore.size(), sForeverCriteriaTreeStore.size(), sForeverModifierTreeStore.size());
        return true;
    }

    bool loadForeverRulesetStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDB::WDC5File relation;
        if (!loadForeverGenericWDC5(relation, "SuperDistrictSetXAvailableSD.db2", errors, dbcPath))
            return false;

        // Classic 1.60 stores AvailableSuperDistrictID as the only physical field.
        // SuperDistrictSetID is the WDC5 relationship parent and is exposed by getParentId().
        if (relation.getFieldCount() != 1u)
        {
            errors.push_back("Forever DB2 SuperDistrictSetXAvailableSD.db2: unexpected field count " + std::to_string(relation.getFieldCount()) + " (expected 1 plus relationship parent)");
            sLogger.failure("DB2: SuperDistrictSetXAvailableSD.db2 invalid (fields={}, expected=1 plus relationship parent).", relation.getFieldCount());
            return false;
        }

        sSuperDistrictSetXAvailableSDStore.clear();
        for (uint32_t row = 0; row < relation.getRecordCount(); ++row)
        {
            WDB::Structures::SuperDistrictSetXAvailableSDEntry entry{};
            entry.id = relation.getRecordId(row);
            entry.superDistrictSetId = relation.getParentId(row);
            entry.availableSuperDistrictId = relation.getUInt32(row, 0);

            if (entry.superDistrictSetId == 0u)
            {
                sLogger.debugDbTables("Forever SuperDistrictSetXAvailableSD row id={} has no relationship parent; skipping.", entry.id);
                continue;
            }

            sSuperDistrictSetXAvailableSDStore[entry.id] = entry;
        }

        sLogger.debugDbTables("DB2: SuperDistrictSetXAvailableSD store ready (relations={}).", sSuperDistrictSetXAvailableSDStore.size());
        return true;
    }

    bool loadForeverCurrencyStore(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDB::WDC5File currencyTypes;
        if (!loadForeverGenericWDC5(currencyTypes, "CurrencyTypes.db2", errors, dbcPath, {{22, 2}}))
            return false;

        // FOREVER-VERIFIED: CurrencyTypes.db2 uses 23 physical fields in 1.60.1.
        // The record ID is external; field 22 is Flags[2]. Only the fields used by
        // AscEmu's common currency runtime are bridged here.
        if (currencyTypes.getFieldCount() != 23)
        {
            errors.push_back("Forever DB2 CurrencyTypes.db2: unexpected field count " + std::to_string(currencyTypes.getFieldCount()) + " (expected 23)");
            sLogger.failure("DB2: CurrencyTypes.db2 invalid (fields={}, expected=23).", currencyTypes.getFieldCount());
            return false;
        }

        std::vector<std::pair<uint32_t, WDB::Structures::CurrencyTypesEntry>> entries;
        entries.reserve(currencyTypes.getRecordCount());
        for (uint32_t row = 0; row < currencyTypes.getRecordCount(); ++row)
        {
            WDB::Structures::CurrencyTypesEntry entry{};
            const uint32_t id = currencyTypes.getRecordId(row);
            entry.Category = static_cast<uint32_t>(currencyTypes.getInt32(row, 2));
            entry.name = nullptr; // Localized DB2 name is not required by the runtime currency logic.
            entry.TotalCap = currencyTypes.getUInt32(row, 6);
            entry.WeekCap = currencyTypes.getUInt32(row, 7);
            entry.Flags = currencyTypes.getUInt32(row, 22, 0);
            entries.emplace_back(id, entry);
        }

        sCurrencyTypesStore.assignEntries(entries);
        sLogger.debugDbTables("DB2: CurrencyTypes store ready (entries={}).", entries.size());
        return true;
    }

    bool loadForeverTraitStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDB::WDC5File traitSystem, traitTree, traitNode, traitNodeEntry, traitNodeXEntry, traitDefinition, traitSubTree;
        WDB::WDC5File traitCost, traitCurrency, traitCurrencySource, traitNodeEntryXCost, traitTreeXCurrency, traitTreeLoadout, skillLineXTraitTree;

        auto load = [&](WDB::WDC5File& file, char const* name)
        {
            return loadForeverGenericWDC5(file, name, errors, dbcPath);
        };

        if (!load(traitSystem, "TraitSystem.db2") || !load(traitTree, "TraitTree.db2") || !load(traitNode, "TraitNode.db2")
            || !load(traitNodeEntry, "TraitNodeEntry.db2") || !load(traitNodeXEntry, "TraitNodeXTraitNodeEntry.db2")
            || !load(traitDefinition, "TraitDefinition.db2") || !load(traitSubTree, "TraitSubTree.db2")
            || !load(traitCost, "TraitCost.db2") || !load(traitCurrency, "TraitCurrency.db2")
            || !load(traitCurrencySource, "TraitCurrencySource.db2") || !load(traitNodeEntryXCost, "TraitNodeEntryXTraitCost.db2")
            || !load(traitTreeXCurrency, "TraitTreeXTraitCurrency.db2") || !load(traitTreeLoadout, "TraitTreeLoadout.db2")
            || !load(skillLineXTraitTree, "SkillLineXTraitTree.db2"))
            return false;

        auto verify = [&](WDB::WDC5File const& file, char const* name, uint32_t logicalFields)
        {
            uint32_t const fields = file.getFieldCount();
            bool const externalId = file.hasExternalRecordIds() || file.getIndexField() < 0;
            uint32_t const expected = externalId ? logicalFields - 1 : logicalFields;
            if (fields == expected)
                return true;

            errors.push_back(std::string("Forever DB2 ") + name + ": unexpected field count " + std::to_string(fields)
                + " (expected " + std::to_string(expected) + (externalId ? " with external ID)" : " with embedded ID)"));
            sLogger.failure("DB2: {} invalid (fields={}, expected={}, indexField={}).", name, fields, expected, file.getIndexField());
            return false;
        };

        if (!verify(traitSystem, "TraitSystem.db2", 6) || !verify(traitTree, "TraitTree.db2", 10)
            || !verify(traitNode, "TraitNode.db2", 7) || !verify(traitNodeEntry, "TraitNodeEntry.db2", 5)
            || !verify(traitNodeXEntry, "TraitNodeXTraitNodeEntry.db2", 4) || !verify(traitDefinition, "TraitDefinition.db2", 8)
            || !verify(traitSubTree, "TraitSubTree.db2", 5) || !verify(traitCost, "TraitCost.db2", 5)
            || !verify(traitCurrency, "TraitCurrency.db2", 8)
            || !verify(traitNodeEntryXCost, "TraitNodeEntryXTraitCost.db2", 3) || !verify(traitTreeXCurrency, "TraitTreeXTraitCurrency.db2", 4)
            || !verify(traitTreeLoadout, "TraitTreeLoadout.db2", 3) || !verify(skillLineXTraitTree, "SkillLineXTraitTree.db2", 4))
            return false;

        bool const currencySourceHasSuperDistrict = traitCurrencySource.getFieldCount() == static_cast<uint32_t>(((traitCurrencySource.hasExternalRecordIds() || traitCurrencySource.getIndexField() < 0) ? 10 : 11) - 1);
        uint32_t const currencySourceLogicalFields = currencySourceHasSuperDistrict ? 10u : 9u;
        if (!verify(traitCurrencySource, "TraitCurrencySource.db2", currencySourceLogicalFields))
            return false;

        auto field = [](WDB::WDC5File const& file, uint32_t logicalField, uint32_t idLogicalField)
        {
            if (file.hasExternalRecordIds() || file.getIndexField() < 0)
                return logicalField < idLogicalField ? logicalField : logicalField - 1;
            return logicalField;
        };

        sTraitSystemStore.clear();
        for (uint32_t row = 0; row < traitSystem.getRecordCount(); ++row)
        {
            WDB::Structures::TraitSystemEntry e{};
            e.id = traitSystem.getRecordId(row);
            e.flags = traitSystem.getInt32(row, field(traitSystem, 1, 0));
            e.widgetSetId = traitSystem.getInt32(row, field(traitSystem, 2, 0));
            e.traitChangeSpell = traitSystem.getInt32(row, field(traitSystem, 3, 0));
            e.itemId = traitSystem.getInt32(row, field(traitSystem, 4, 0));
            e.variationType = traitSystem.getInt32(row, field(traitSystem, 5, 0));
            sTraitSystemStore[e.id] = e;
        }

        sTraitTreeStore.clear();
        for (uint32_t row = 0; row < traitTree.getRecordCount(); ++row)
        {
            WDB::Structures::TraitTreeEntry e{};
            e.id = traitTree.getRecordId(row);
            e.traitSystemId = traitTree.getUInt32(row, field(traitTree, 2, 1));
            e.baseNodeGroup = traitTree.getInt32(row, field(traitTree, 3, 1));
            e.firstTraitNodeId = traitTree.getInt32(row, field(traitTree, 4, 1));
            e.playerConditionId = traitTree.getInt32(row, field(traitTree, 5, 1));
            e.flags = traitTree.getInt32(row, field(traitTree, 6, 1));
            e.minZoom = traitTree.getFloat(row, field(traitTree, 7, 1));
            e.maxZoom = traitTree.getFloat(row, field(traitTree, 8, 1));
            e.uiTextureKitId = traitTree.getInt32(row, field(traitTree, 9, 1));
            sTraitTreeStore[e.id] = e;
        }

        sTraitNodeStore.clear();
        for (uint32_t row = 0; row < traitNode.getRecordCount(); ++row)
        {
            WDB::Structures::TraitNodeEntry e{};
            e.id = traitNode.getRecordId(row);
            e.traitTreeId = traitNode.getUInt32(row, field(traitNode, 1, 0));
            e.posX = traitNode.getInt32(row, field(traitNode, 2, 0));
            e.posY = traitNode.getInt32(row, field(traitNode, 3, 0));
            e.type = traitNode.getUInt8(row, field(traitNode, 4, 0));
            e.flags = traitNode.getInt32(row, field(traitNode, 5, 0));
            e.traitSubTreeId = traitNode.getInt32(row, field(traitNode, 6, 0));
            sTraitNodeStore[e.id] = e;
        }

        sTraitNodeEntryStore.clear();
        for (uint32_t row = 0; row < traitNodeEntry.getRecordCount(); ++row)
        {
            WDB::Structures::TraitNodeEntryEntry e{};
            e.id = traitNodeEntry.getRecordId(row);
            e.traitDefinitionId = traitNodeEntry.getInt32(row, field(traitNodeEntry, 1, 0));
            e.maxRanks = traitNodeEntry.getInt32(row, field(traitNodeEntry, 2, 0));
            e.nodeEntryType = traitNodeEntry.getUInt8(row, field(traitNodeEntry, 3, 0));
            e.traitSubTreeId = traitNodeEntry.getInt32(row, field(traitNodeEntry, 4, 0));
            sTraitNodeEntryStore[e.id] = e;
        }

        sTraitNodeXTraitNodeEntryStore.clear();
        for (uint32_t row = 0; row < traitNodeXEntry.getRecordCount(); ++row)
        {
            WDB::Structures::TraitNodeXTraitNodeEntryEntry e{};
            e.id = traitNodeXEntry.getRecordId(row);
            e.traitNodeId = traitNodeXEntry.getUInt32(row, field(traitNodeXEntry, 1, 0));
            e.traitNodeEntryId = traitNodeXEntry.getInt32(row, field(traitNodeXEntry, 2, 0));
            e.index = traitNodeXEntry.getInt32(row, field(traitNodeXEntry, 3, 0));
            sTraitNodeXTraitNodeEntryStore[e.id] = e;
        }

        sTraitDefinitionStore.clear();
        for (uint32_t row = 0; row < traitDefinition.getRecordCount(); ++row)
        {
            WDB::Structures::TraitDefinitionEntry e{};
            e.id = traitDefinition.getRecordId(row);
            e.spellId = traitDefinition.getInt32(row, field(traitDefinition, 4, 3));
            e.overrideIcon = traitDefinition.getInt32(row, field(traitDefinition, 5, 3));
            e.overridesSpellId = traitDefinition.getInt32(row, field(traitDefinition, 6, 3));
            e.visibleSpellId = traitDefinition.getInt32(row, field(traitDefinition, 7, 3));
            sTraitDefinitionStore[e.id] = e;
        }

        sTraitSubTreeStore.clear();
        for (uint32_t row = 0; row < traitSubTree.getRecordCount(); ++row)
        {
            WDB::Structures::TraitSubTreeEntry e{};
            e.id = traitSubTree.getRecordId(row);
            e.uiTextureAtlasElementId = traitSubTree.getInt32(row, field(traitSubTree, 3, 2));
            e.traitTreeId = traitSubTree.getUInt32(row, field(traitSubTree, 4, 2));
            sTraitSubTreeStore[e.id] = e;
        }

        sTraitCostStore.clear();
        for (uint32_t row = 0; row < traitCost.getRecordCount(); ++row)
        {
            WDB::Structures::TraitCostEntry e{};
            e.id = traitCost.getRecordId(row);
            e.amount = traitCost.getInt32(row, field(traitCost, 2, 1));
            e.traitCurrencyId = traitCost.getInt32(row, field(traitCost, 3, 1));
            e.curveId = traitCost.getInt32(row, field(traitCost, 4, 1));
            sTraitCostStore[e.id] = e;
        }

        sTraitCurrencyStore.clear();
        for (uint32_t row = 0; row < traitCurrency.getRecordCount(); ++row)
        {
            WDB::Structures::TraitCurrencyEntry e{};
            e.id = traitCurrency.getRecordId(row);
            e.type = traitCurrency.getInt32(row, field(traitCurrency, 1, 0));
            e.currencyTypesId = traitCurrency.getInt32(row, field(traitCurrency, 2, 0));
            e.flags = traitCurrency.getInt32(row, field(traitCurrency, 3, 0));
            e.icon = traitCurrency.getInt32(row, field(traitCurrency, 4, 0));
            e.playerDataElementAccountId = traitCurrency.getInt32(row, field(traitCurrency, 5, 0));
            e.playerDataElementCharacterId = traitCurrency.getInt32(row, field(traitCurrency, 6, 0));
            e.sourcedMax = traitCurrency.getInt32(row, field(traitCurrency, 7, 0));
            sTraitCurrencyStore[e.id] = e;
        }

        sTraitCurrencySourceStore.clear();
        for (uint32_t row = 0; row < traitCurrencySource.getRecordCount(); ++row)
        {
            WDB::Structures::TraitCurrencySourceEntry e{};
            e.id = traitCurrencySource.getRecordId(row);
            e.traitCurrencyId = traitCurrencySource.getUInt32(row, field(traitCurrencySource, 2, 1));
            e.amount = traitCurrencySource.getInt32(row, field(traitCurrencySource, 3, 1));
            e.questId = traitCurrencySource.getInt32(row, field(traitCurrencySource, 4, 1));
            e.achievementId = traitCurrencySource.getInt32(row, field(traitCurrencySource, 5, 1));
            e.playerLevel = traitCurrencySource.getInt32(row, field(traitCurrencySource, 6, 1));
            e.traitNodeEntryId = traitCurrencySource.getInt32(row, field(traitCurrencySource, 7, 1));
            if (currencySourceHasSuperDistrict)
                e.superDistrictSetId = traitCurrencySource.getInt32(row, field(traitCurrencySource, 8, 1));
            e.orderIndex = traitCurrencySource.getInt32(row, field(traitCurrencySource, currencySourceHasSuperDistrict ? 9 : 8, 1));
            sTraitCurrencySourceStore[e.id] = e;
        }

        sTraitNodeEntryXTraitCostStore.clear();
        for (uint32_t row = 0; row < traitNodeEntryXCost.getRecordCount(); ++row)
        {
            WDB::Structures::TraitNodeEntryXTraitCostEntry e{};
            e.id = traitNodeEntryXCost.getRecordId(row);
            e.traitNodeEntryId = traitNodeEntryXCost.getUInt32(row, field(traitNodeEntryXCost, 1, 0));
            e.traitCostId = traitNodeEntryXCost.getInt32(row, field(traitNodeEntryXCost, 2, 0));
            sTraitNodeEntryXTraitCostStore[e.id] = e;
        }

        sTraitTreeXTraitCurrencyStore.clear();
        for (uint32_t row = 0; row < traitTreeXCurrency.getRecordCount(); ++row)
        {
            WDB::Structures::TraitTreeXTraitCurrencyEntry e{};
            e.id = traitTreeXCurrency.getRecordId(row);
            e.index = traitTreeXCurrency.getInt32(row, field(traitTreeXCurrency, 1, 0));
            e.traitTreeId = traitTreeXCurrency.getUInt32(row, field(traitTreeXCurrency, 2, 0));
            e.traitCurrencyId = traitTreeXCurrency.getInt32(row, field(traitTreeXCurrency, 3, 0));
            sTraitTreeXTraitCurrencyStore[e.id] = e;
        }

        sTraitTreeLoadoutStore.clear();
        for (uint32_t row = 0; row < traitTreeLoadout.getRecordCount(); ++row)
        {
            WDB::Structures::TraitTreeLoadoutEntry e{};
            e.id = traitTreeLoadout.getRecordId(row);
            e.traitTreeId = traitTreeLoadout.getUInt32(row, field(traitTreeLoadout, 1, 0));
            e.chrSpecializationId = traitTreeLoadout.getInt32(row, field(traitTreeLoadout, 2, 0));
            sTraitTreeLoadoutStore[e.id] = e;
        }

        sSkillLineXTraitTreeStore.clear();
        for (uint32_t row = 0; row < skillLineXTraitTree.getRecordCount(); ++row)
        {
            WDB::Structures::SkillLineXTraitTreeEntry e{};
            e.id = skillLineXTraitTree.getRecordId(row);
            e.skillLineId = skillLineXTraitTree.getUInt32(row, field(skillLineXTraitTree, 1, 0));
            e.traitTreeId = skillLineXTraitTree.getInt32(row, field(skillLineXTraitTree, 2, 0));
            e.orderIndex = skillLineXTraitTree.getInt32(row, field(skillLineXTraitTree, 3, 0));
            sSkillLineXTraitTreeStore[e.id] = e;
        }

        sLogger.debugDbTables("DB2: Trait stores ready (systems={}, trees={}, nodes={}, entries={}, definitions={}, subtrees={}, currencies={}, sources={}, skillLineTrees={}).",
            sTraitSystemStore.size(), sTraitTreeStore.size(), sTraitNodeStore.size(), sTraitNodeEntryStore.size(), sTraitDefinitionStore.size(),
            sTraitSubTreeStore.size(), sTraitCurrencyStore.size(), sTraitCurrencySourceStore.size(), sSkillLineXTraitTreeStore.size());
        return true;
    }

    bool loadForeverModernMapStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDB::WDC5File map;
        WDB::WDC5File mapDifficulty;
        WDB::WDC5File uiMapAssignment;
        WDB::WDC5File worldMapOverlay;
        if (!loadForeverWDC5Group({ { map, ForeverFormat::Map }, { mapDifficulty, ForeverFormat::MapDifficulty }, { uiMapAssignment, ForeverFormat::UiMapAssignment }, { worldMapOverlay, ForeverFormat::WorldMapOverlay } }, errors, dbcPath))
            return false;

        sMapStore.clear();
        for (uint32_t row = 0; row < map.getRecordCount(); ++row)
        {
            WDB::Structures::MapEntry entry;
            entry.id = map.getRecordId(row);

            // AscEmu's legacy mapType uses the same 0..4 semantics as the
            // modern Map::InstanceType field (world/party/raid/bg/arena).
            entry.mapType = static_cast<uint32_t>(static_cast<uint8_t>(map.getInt8(row, 8)));
            entry.linkedZone = map.getUInt16(row, 10);       // AreaTableID
            entry.multimapId = static_cast<uint32_t>(static_cast<uint16_t>(map.getInt16(row, 14))); // CosmeticParentMapID
            entry.parentMap = map.getInt16(row, 17);        // CorpseMapID
            entry.startX = map.getFloat(row, 6, 0);         // Corpse X
            entry.startY = map.getFloat(row, 6, 1);         // Corpse Y
            entry.addon = map.getUInt8(row, 9);             // ExpansionID
            entry.maxPlayers = map.getUInt8(row, 18);
            sMapStore[entry.id] = std::move(entry);
        }

        sMapDifficultyStore.clear();
        for (uint32_t row = 0; row < mapDifficulty.getRecordCount(); ++row)
        {
            WDB::Structures::MapDifficultyEntry entry;
            entry.id = mapDifficulty.getRecordId(row);
            entry.mapId = mapDifficulty.getParentId(row);
            entry.difficulty = static_cast<uint32_t>(static_cast<uint16_t>(mapDifficulty.getInt16(row, 2)));
            uint8_t const resetInterval = mapDifficulty.getUInt8(row, 4);
            entry.raidDuration = resetInterval == 1 ? 86400U : (resetInterval == 2 ? 604800U : 0U);
            int32_t const maxPlayers = mapDifficulty.getInt32(row, 5);
            entry.maxPlayers = maxPlayers > 0 ? static_cast<uint32_t>(maxPlayers) : 0U;
            sMapDifficultyStore[entry.id] = std::move(entry);
        }

        // WorldMapArea.dbc no longer exists in modern clients. UiMapAssignment
        // is the modern source that directly relates AreaID -> MapID. Populate
        // the legacy compatibility store from that relation so old AscEmu code
        // (notably flying-map resolution) can keep using the established API.
        sWorldMapAreaStore.clear();
        for (uint32_t row = 0; row < uiMapAssignment.getRecordCount(); ++row)
        {
            int32_t const mapId = uiMapAssignment.getInt32(row, 6);
            int32_t const areaId = uiMapAssignment.getInt32(row, 7);
            if (mapId < 0 || areaId <= 0)
                continue;

            WDB::Structures::WorldMapAreaEntry entry;
            entry.mapId = static_cast<uint32_t>(mapId);
            entry.zoneId = static_cast<uint32_t>(areaId);
            entry.continentMapId = mapId;
            sWorldMapAreaStore[entry.zoneId] = entry;
        }

        sWorldMapOverlayStore.clear();
        for (uint32_t row = 0; row < worldMapOverlay.getRecordCount(); ++row)
        {
            WDB::Structures::WorldMapOverlayEntry entry{};
            entry.ID = worldMapOverlay.getRecordId(row);
            entry.areaID = worldMapOverlay.getUInt32(row, 12, 0);
            entry.areaID_2 = worldMapOverlay.getUInt32(row, 12, 1);
            entry.areaID_3 = worldMapOverlay.getUInt32(row, 12, 2);
            entry.areaID_4 = worldMapOverlay.getUInt32(row, 12, 3);
            sWorldMapOverlayStore[entry.ID] = entry;
        }

        auto const* map0 = sMapStore.lookupEntry(0);

        return map0 != nullptr;
    }

    bool loadForeverModernTerrainStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDB::WDC5File areaTable;
        WDB::WDC5File liquidType;
        WDB::WDC5File wmoAreaTable;

        if (!loadForeverWDC5Group({ { areaTable, ForeverFormat::AreaTable }, { liquidType, ForeverFormat::LiquidType }, { wmoAreaTable, ForeverFormat::WMOAreaTable } }, errors, dbcPath))
            return false;

        sAreaStore.clear();
        for (uint32_t row = 0; row < areaTable.getRecordCount(); ++row)
        {
            WDB::Structures::AreaTableEntry entry{};
            entry.id = areaTable.getRecordId(row);
            entry.map_id = areaTable.getUInt16(row, 2);              // ContinentID
            entry.zone = areaTable.getUInt16(row, 3);                // ParentAreaID
            entry.explore_flag = static_cast<uint32_t>(areaTable.getInt16(row, 4)); // AreaBit
            entry.area_level = static_cast<int32_t>(areaTable.getInt8(row, 11));    // ExplorationLevel
            entry.team = areaTable.getUInt8(row, 14);                // FactionGroupMask
            entry.flags = areaTable.getUInt32(row, 22, 0);           // Flags[0]
            for (uint32_t i = 0; i < 4; ++i)
                entry.liquid_type_override[i] = areaTable.getUInt16(row, 23, i);

            // Modern AreaTable no longer carries the legacy elevation field.
            // Terrain height continues to come from maps/vmaps.
            entry.elevation = 0.0f;
            sAreaStore[entry.id] = std::move(entry);
        }

        std::vector<std::pair<uint32_t, WDB::Structures::LiquidTypeEntry>> liquidEntries;
        liquidEntries.reserve(liquidType.getRecordCount());
        for (uint32_t row = 0; row < liquidType.getRecordCount(); ++row)
        {
            WDB::Structures::LiquidTypeEntry entry{};
            entry.Id = liquidType.getRecordId(row);
            // AscEmu's legacy Type is the liquid category used to build
            // MAP_LIQUID_TYPE_* masks. Modern clients expose that as SoundBank.
            entry.Type = liquidType.getUInt8(row, 3);                // SoundBank
            entry.SpellId = liquidType.getUInt32(row, 5);           // SpellID
            liquidEntries.emplace_back(entry.Id, entry);
        }
        sLiquidTypeStore.assignEntries(liquidEntries);

        std::vector<std::pair<uint32_t, WDB::Structures::WMOAreaTableEntry>> wmoEntries;
        wmoEntries.reserve(wmoAreaTable.getRecordCount());
        for (uint32_t row = 0; row < wmoAreaTable.getRecordCount(); ++row)
        {
            WDB::Structures::WMOAreaTableEntry entry{};
            entry.id = wmoAreaTable.getRecordId(row);
            entry.rootId = static_cast<int32_t>(wmoAreaTable.getUInt16(row, 2)); // WMOID
            entry.adtId = static_cast<int32_t>(wmoAreaTable.getUInt8(row, 3));   // NameSetID
            entry.groupId = wmoAreaTable.getInt32(row, 4);                      // WMOGroupID
            entry.areaId = wmoAreaTable.getUInt16(row, 13);                     // AreaTableID
            entry.flags = wmoAreaTable.getUInt32(row, 14);                      // Flags
            wmoEntries.emplace_back(entry.id, entry);
        }
        sWMOAreaTableStore.assignEntries(wmoEntries);

        sLogger.info("DB2: Terrain stores ready (AreaTable={}, LiquidType={}, WMOAreaTable={}).",
            sAreaStore.size(), liquidEntries.size(), wmoEntries.size());
        return !sAreaStore.empty() && !liquidEntries.empty();
    }
#endif

    void buildAreaMapCollection()
    {
        auto* areaMapCollection = MapManagement::AreaManagement::AreaStorage::getMapCollection();
        if (!areaMapCollection)
            return;

        for (auto const& mapObject : sMapStore | std::views::values)
        {
            areaMapCollection->insert({mapObject.id, mapObject.linkedZone});
        }
    }

    void buildMapDifficultyMap()
    {
        sMapDifficultyMap.clear();

        // Fill the map difficulty map with data from MapDifficultyStore if available Cata / MoP
        for (auto const& entry : sMapDifficultyStore | std::views::values)
        {
            uint32_t const key = Util::MAKE_PAIR32(static_cast<uint16_t>(entry.mapId), static_cast<uint16_t>(entry.difficulty));
            sMapDifficultyMap[key] = WDB::Structures::MapDifficulty(
                entry.raidDuration,
                entry.maxPlayers,
                !entry.message.empty()
            );
        }

        // Fallback classic, tbc and wotlk, where MapDifficultyStore is not available
        if (sMapDifficultyStore.empty())
        {
            for (auto const& entry : sMapStore | std::views::values)
            {
                uint32_t const maxPlayers = (entry.getAddon() < 1)
                                                ? (entry.isRaid() ? 40 : 5)
                                                : (entry.isRaid() ? 25 : 5);

                if (!entry.getResetTimeHeroic())
                {
                    sMapDifficultyMap[Util::MAKE_PAIR32(static_cast<uint16_t>(entry.id), InstanceDifficulty::Difficulties::DUNGEON_NORMAL)] =
                        WDB::Structures::MapDifficulty(entry.getResetTimeNormal(), maxPlayers, false);
                }
                else
                {
                    sMapDifficultyMap[Util::MAKE_PAIR32(static_cast<uint16_t>(entry.id), InstanceDifficulty::Difficulties::DUNGEON_HEROIC)] =
                        WDB::Structures::MapDifficulty(entry.getResetTimeHeroic(), maxPlayers, false);
                }
            }
        }
    }

    void buildPowerIndexByClass()
    {
#if defined(AE_FOREVER)
        constexpr uint8_t invalidPowerIndex = 0;
#else
        constexpr uint8_t invalidPowerIndex = TOTAL_PLAYER_POWER_TYPES;
#endif

        for (auto& classPowers : powerIndexByClass)
            classPowers.fill(invalidPowerIndex);

#if defined(AE_FOREVER)
        // ChrClassesXPowerTypes defines the per-class wire order. Sort by ClassID and
        // PowerType before assigning array indices; store iteration order is not stable
        // and can place powers in the wrong slot for classes with multiple power types.
        std::vector<WDB::Structures::ChrPowerTypesEntry> powers;
        powers.reserve(sChrPowerTypesStore.getNumRows());

        for (auto const& powerEntry : sChrPowerTypesStore | std::views::values)
        {
            if (powerEntry.classId >= MAX_PLAYER_CLASSES || powerEntry.power >= TOTAL_PLAYER_POWER_TYPES)
                continue;

            powers.push_back(powerEntry);
        }

        std::ranges::sort(powers, [](auto const& left, auto const& right)
        {
            if (left.classId != right.classId)
                return left.classId < right.classId;

            return left.power < right.power;
        });

        std::array<uint8_t, MAX_PLAYER_CLASSES> nextIndex{};
        nextIndex.fill(POWER_FIELD_INDEX_1);

        for (auto const& powerEntry : powers)
            powerIndexByClass[powerEntry.classId][powerEntry.power] = nextIndex[powerEntry.classId]++;
#else
        for (auto const& powerEntry : sChrPowerTypesStore | std::views::values)
        {
            // Boundary Checks against Out-of-Bounds access
            if (powerEntry.classId >= MAX_PLAYER_CLASSES || powerEntry.power >= TOTAL_PLAYER_POWER_TYPES)
                continue;

            uint8_t index = 1;
            for (uint8_t power = POWER_TYPE_MANA; power < TOTAL_PLAYER_POWER_TYPES; ++power)
            {
                if (powerIndexByClass[powerEntry.classId][power] != invalidPowerIndex)
                    ++index;
            }

            powerIndexByClass[powerEntry.classId][powerEntry.power] = index;
        }
#endif
    }
}

bool loadDBCs()
{
    /////////////////////////////////////////////////////////////////////////////////////////
    // Load basic dbcs available in all versions
    uint32_t available_dbc_locales = 0xFFFFFFFF;
    WDB::StoreProblemList bad_dbc_files;
    std::string dbc_path = sWorld.settings.server.dataDir + "dbc/";

#if defined(AE_FOREVER)
    // Forever has no legacy DBC data path. All client data must come from DB2/WDC5.
    // Keep populating the established AscEmu runtime stores so the rest of the core
    // can remain format-agnostic.
    loadForeverModernCharacterStores(bad_dbc_files, dbc_path);
    loadForeverModernCreatureStores(bad_dbc_files, dbc_path);
    loadForeverModernCustomizationStores(bad_dbc_files, dbc_path);
    loadForeverModernTaxiStores(bad_dbc_files, dbc_path);
    loadForeverModernItemStores(bad_dbc_files, dbc_path);
    loadForeverAchievementStores(bad_dbc_files, dbc_path);
    loadForeverCurrencyStore(bad_dbc_files, dbc_path);
    loadForeverRulesetStores(bad_dbc_files, dbc_path);
    loadForeverTraitStores(bad_dbc_files, dbc_path);
    loadForeverModernMapStores(bad_dbc_files, dbc_path);
    loadForeverModernTerrainStores(bad_dbc_files, dbc_path);
    loadForeverModernQuestStores(bad_dbc_files, dbc_path);
    loadForeverModernSpellSkillStores(bad_dbc_files, dbc_path);
    loadForeverModernEmoteStores(bad_dbc_files, dbc_path);

    buildMapDifficultyMap();
    buildAreaMapCollection();

    if (!bad_dbc_files.empty())
    {
        for (std::string const& problem : bad_dbc_files)
            sLogger.failure("{}", problem);
        return false;
    }

    return true;
#else // !AE_FOREVER - legacy DBC/DB2 path

    // Load ChrClasses.dbc first to ensure the dbcLocaleId is set correctly before loading other DBC files that may depend on it
    WDB::loadUnifiedWDBStore<WDB::Structures::ChrClassesEntry>(
        bad_dbc_files, sChrClassesStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::ChrClassesEntry& entry) {
            entry.classId = raw.id;
            entry.powerType = raw.powerType;
            entry.spellClassSet = raw.spellClassSet;

            if constexpr (requires { raw.cinematicSequenceId; }) {
                entry.cinematicSequenceId = raw.cinematicSequenceId;
            }

            if constexpr (requires { raw.requiredExpansion; }) {
                entry.requiredExpansion = raw.requiredExpansion;
            }

            if constexpr (requires { raw.apPerStr; }) {
                entry.apPerStr = raw.apPerStr;
                entry.apPerAgi = raw.apPerAgi;
                entry.rapPerAgi = raw.rapPerAgi;
            }

            // Pre-Cata: Array of localized strings
            if constexpr (requires { { raw.name[0] } -> std::convertible_to<const char*>; }) {
                constexpr size_t arraySize = std::extent_v<decltype(raw.name)>;

                // Auto-detect DBC locale ID on Warrior entry (ID 1)
                if (raw.id == 1) {
                    for (size_t i = 0; i < arraySize; ++i) {
                        if (raw.name[i] && raw.name[i][0] != '\0') {
                            uint8_t const detectedLocale = static_cast<uint8_t>(i);
                            sWorld.setDbcLocaleLanguageId(detectedLocale);
                            sLogger.info("DBC: Auto-detected locale ID {} ({}) from ChrClasses.dbc", detectedLocale, Util::getLanguagesStringFromId(detectedLocale));
                            break;
                        }
                    }
                }

                uint8_t const localeId = sWorld.getDbcLocaleLanguageId();
                entry.name = (localeId < arraySize && raw.name[localeId]) ? raw.name[localeId] : (raw.name[0] ? raw.name[0] : "");
            }
            // Cata / MoP: Single string
            else if constexpr (requires { raw.name; }) {
                entry.name = raw.name ? raw.name : "";
            }
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::AreaTableEntry>(
        bad_dbc_files, sAreaStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::AreaTableEntry& entry) {
            entry.id = raw.id;
            entry.map_id = raw.map_id;
            entry.zone = raw.zone;
            entry.explore_flag = raw.explore_flag;
            entry.flags = raw.flags;
            entry.area_level = raw.area_level;
            entry.team = raw.team;

            if constexpr (requires { { raw.area_name[0] } -> std::convertible_to<const char*>; })
            {
                uint8_t localeId = sWorld.getDbcLocaleLanguageId();
                entry.area_name = raw.area_name[localeId] ? raw.area_name[localeId] : ""; // Classic, TBC, WotLK
            }
            else
            {
                entry.area_name = raw.area_name ? raw.area_name : ""; // Cata, MoP
            }

            if constexpr (requires { raw.liquid_type_override[0]; })
            {
                std::copy(std::begin(raw.liquid_type_override), std::end(raw.liquid_type_override), std::begin(entry.liquid_type_override));
            }
            else
            {
                entry.liquid_type_override[0] = raw.liquid_type_override; // Classic
            }

            if constexpr (requires { raw.elevation; })
            {
                entry.elevation = raw.elevation; // MoP
            }
            else if constexpr (requires { raw.min_elevation; })
            {
                entry.elevation = raw.min_elevation; // Cata
            }
        }
    );


    MapManagement::AreaManagement::AreaStorage::initialise(&sAreaStore);

    WDB::loadUnifiedWDBStore<WDB::Structures::AreaTriggerEntry>(
        bad_dbc_files, sAreaTriggerStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::AreaTriggerEntry& entry)
        {
            entry.id = raw.id;
            entry.mapId = raw.mapId;
            entry.x = raw.x;
            entry.y = raw.y;
            entry.z = raw.z;
            entry.boxRadius = raw.boxRadius;
            entry.boxX = raw.boxX;
            entry.boxY = raw.boxY;
            entry.boxZ = raw.boxZ;
            entry.boxOrientation = raw.boxOrientation;
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::AuctionHouseEntry>(
        bad_dbc_files, sAuctionHouseStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::AuctionHouseEntry& entry)
        {
            entry.id = raw.id;
            entry.faction = raw.faction;
            entry.fee = raw.fee;
            entry.tax = raw.tax;
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::BankBagSlotPricesEntry>(
        bad_dbc_files, sBankBagSlotPricesStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::BankBagSlotPricesEntry& entry)
        {
            entry.id = raw.id;
            entry.price = raw.price;
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::ChatChannelsEntry>(
        bad_dbc_files, sChatChannelsStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::ChatChannelsEntry& entry)
        {
            entry.id = raw.id;
            entry.flags = raw.flags;

            if constexpr (std::is_array_v<decltype(raw.namePattern)>)
            {
                uint8_t const localeId = sWorld.getDbcLocaleLanguageId();
                constexpr size_t arraySize = sizeof(raw.namePattern) / sizeof(raw.namePattern[0]);

                uint8_t const validIndex = (localeId < arraySize) ? localeId : 0;
                entry.namePattern = raw.namePattern[validIndex] ? raw.namePattern[validIndex] : "";
            }
            else
            {
                entry.namePattern = raw.namePattern ? raw.namePattern : "";
            }
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::CharStartOutfitEntry>(
        bad_dbc_files, sCharStartOutfitStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::CharStartOutfitEntry& entry)
        {
            entry.race = raw.race;
            entry.classId = raw.classId;
            entry.gender = raw.gender;

            constexpr size_t rawItemCount = sizeof(raw.itemId) / sizeof(raw.itemId[0]);
            for (size_t i = 0; i < rawItemCount; ++i)
            {
                entry.itemId[i] = raw.itemId[i];
            }

            if constexpr (requires { raw.petDisplayId; })
            {
                entry.petDisplayId = raw.petDisplayId;
                entry.petFamilyEntry = raw.petFamilyEntry;
            }
        });

    for (auto const& [id, outfit] : sCharStartOutfitStore)
    {
        sCharStartOutfitMap[outfit.makeKey()] = &outfit;
    }

    WDB::loadUnifiedWDBStore<WDB::Structures::ChrRacesEntry>(
        bad_dbc_files, sChrRacesStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::ChrRacesEntry& entry) {
            entry.raceId = raw.id;
            entry.flags = raw.flags;
            entry.factionId = raw.faction_id;
            entry.modelMale = raw.model_male;
            entry.modelFemale = raw.model_female;
            entry.cinematicId = raw.cinematic_id;

            if constexpr (requires { raw.team_id; })
            {
                entry.teamId = raw.team_id;
            }
            else
            {
                entry.teamId = 0;
            }

            if constexpr (requires { { raw.name[0] } -> std::convertible_to<const char*>; })
            {
                uint8_t localeId = sWorld.getDbcLocaleLanguageId();
                entry.name = raw.name[localeId] ? raw.name[localeId] : "";
            }
            else
            {
                entry.name = raw.name ? raw.name : "";
            }

            if constexpr (requires { raw.start_taxi_mask; }) // Classic
            {
                entry.startTaxiMask = raw.start_taxi_mask;
            }
            else
            {
                entry.startTaxiMask = 0;
            }

            if constexpr (requires { raw.expansion; }) // >= TBC
            {
                entry.expansion = raw.expansion;
            }
            else
            {
                entry.expansion = 0; // Classic
            }
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::CreatureDisplayInfoEntry>(
        bad_dbc_files, sCreatureDisplayInfoStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::CreatureDisplayInfoEntry& entry)
        {
            entry.id = raw.id;
            entry.modelId = raw.modelId;
            entry.extendedDisplayInfoId = raw.extendedDisplayInfoId;
            entry.creatureModelScale = raw.creatureModelScale;
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::CreatureDisplayInfoExtraEntry>(
        bad_dbc_files, sCreatureDisplayInfoExtraStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::CreatureDisplayInfoExtraEntry& entry)
        {
            entry.displayExtraId = raw.displayExtraId;
            entry.race = raw.race;
            entry.displaySexId = raw.displaySexId;
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::CreatureModelDataEntry>(
        bad_dbc_files, sCreatureModelDataStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::CreatureModelDataEntry& entry)
        {
            entry.id = raw.id;
            entry.flags = raw.flags;
            entry.modelName = raw.modelName ? raw.modelName : "";
            entry.collisionHeight = raw.collisionHeight;

            if constexpr (requires { raw.modelScale; })
            {
                entry.modelScale = raw.modelScale;
            }

            if constexpr (requires { raw.mountHeight; })
            {
                entry.mountHeight = raw.mountHeight;
            }
        });


    WDB::loadUnifiedWDBStore<WDB::Structures::CreatureSpellDataEntry>(
        bad_dbc_files, sCreatureSpellDataStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::CreatureSpellDataEntry& entry)
        {
            entry.id = raw.id;
            for (size_t i = 0; i < entry.spells.size(); ++i)
            {
                entry.spells[i] = raw.spells[i];
                entry.cooldowns[i] = raw.cooldowns[i];
            }
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::CreatureFamilyEntry>(
        bad_dbc_files, sCreatureFamilyStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::CreatureFamilyEntry& entry)
        {
            entry.id = raw.id;
            entry.minSize = raw.minSize;
            entry.minLevel = raw.minLevel;
            entry.maxSize = raw.maxSize;
            entry.maxLevel = raw.maxLevel;
            entry.skillLine = raw.skillLine;
            entry.tameable = raw.tameable;
            entry.petDietFlags = raw.petDietFlags;

            if constexpr (requires { raw.talentTree; })
            {
                entry.talentTree = raw.talentTree;
            }

            if constexpr (std::is_array_v<decltype(raw.name)>)
            {
                uint8_t const localeId = sWorld.getDbcLocaleLanguageId();
                constexpr size_t arraySize = sizeof(raw.name) / sizeof(raw.name[0]);
                uint8_t const validIndex = (localeId < arraySize) ? localeId : 0;
                entry.name = raw.name[validIndex] ? raw.name[validIndex] : "";
            }
            else
            {
                entry.name = raw.name ? raw.name : "";
            }
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::DurabilityCostsEntry>(
        bad_dbc_files, sDurabilityCostsStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::DurabilityCostsEntry& entry)
        {
            entry.itemLevel = raw.itemLevel;
            for (size_t i = 0; i < entry.modifier.size(); ++i)
            {
                entry.modifier[i] = raw.modifier[i];
            }
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::DurabilityQualityEntry>(
        bad_dbc_files, sDurabilityQualityStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::DurabilityQualityEntry& entry)
        {
            entry.id = raw.id;
            entry.qualityModifier = raw.qualityModifier;
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::EmotesTextEntry>(
        bad_dbc_files, sEmotesTextStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::EmotesTextEntry& entry)
        {
            entry.id = raw.id;
            for (size_t i = 0; i < entry.textId.size(); ++i)
            {
                entry.textId[i] = raw.textId[i];
            }
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::FactionEntry>(
        bad_dbc_files, sFactionStore, dbc_path,
        [](const auto& raw, WDB::Structures::FactionEntry& entry) {
            entry.id = raw.id;
            entry.reputationIndex = raw.reputationIndex;
            entry.parentFactionId = raw.parentFactionId;

            for (std::size_t i = 0; i < 4; ++i)
            {
                entry.reputationRaceMask[i] = raw.reputationRaceMask[i];
                entry.reputationClassMask[i] = raw.reputationClassMask[i];
                entry.reputationBase[i] = raw.reputationBase[i];
                entry.reputationFlags[i] = raw.reputationFlags[i];
            }

            // Spillover (since WotLK/TBC)
            if constexpr (requires { raw.spilloverRateIn; })
            {
                entry.spilloverRateIn = raw.spilloverRateIn;
                entry.spilloverRateOut = raw.spilloverRateOut;
                entry.spilloverMaxIn = raw.spilloverMaxIn;
            }

            // Expansion (since Cata/MoP)
            if constexpr (requires { raw.expansion; })
            {
                entry.expansion = raw.expansion;
            }

            // name localization
            if constexpr (requires { { raw.name[0] } -> std::convertible_to<const char*>; })
            {
                uint8_t localeId = sWorld.getDbcLocaleLanguageId();
                entry.name = raw.name[localeId] ? raw.name[localeId] : "";
            }
            else if constexpr (requires { { raw.name } -> std::convertible_to<const char*>; })
            {
                entry.name = raw.name ? raw.name : "";
            }
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::FactionTemplateEntry>(
        bad_dbc_files, sFactionTemplateStore, dbc_path,
        [](const auto& raw, WDB::Structures::FactionTemplateEntry& entry) {
            entry.id = raw.id;
            entry.faction = raw.faction;
            entry.factionFlags = raw.factionFlags;
            entry.ourMask = raw.ourMask;
            entry.friendlyMask = raw.friendlyMask;
            entry.hostileMask = raw.hostileMask;

            for (std::size_t i = 0; i < WDB::Structures::maxFactionRelations; ++i)
            {
                entry.enemyFaction[i] = raw.enemyFaction[i];
                entry.friendFaction[i] = raw.friendFaction[i];
            }
        }
    );


    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGameObjectDisplayInfoStore, dbc_path, "GameObjectDisplayInfo.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemSetStore, dbc_path, "ItemSet.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemRandomPropertiesStore, dbc_path, "ItemRandomProperties.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sLFGDungeonStore, dbc_path, "LFGDungeons.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sLiquidTypeStore, dbc_path, "LiquidType.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sLockStore, dbc_path, "Lock.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sMailTemplateStore, dbc_path, "MailTemplate.dbc");

    WDB::loadUnifiedWDBStore<WDB::Structures::MapDifficultyEntry>(
        bad_dbc_files, sMapDifficultyStore, dbc_path,
        [](const auto& raw, WDB::Structures::MapDifficultyEntry& entry) {
            entry.mapId = raw.mapId;
            entry.difficulty = raw.difficulty;
            entry.message = raw.message ? raw.message : "";
            entry.raidDuration = raw.raidDuration;
            entry.maxPlayers = raw.maxPlayers;
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::MapEntry>(
        bad_dbc_files, sMapStore, dbc_path,
        [](const auto& raw, WDB::Structures::MapEntry& entry) {
            entry.id = raw.id;
            entry.mapType = raw.mapType;
            entry.linkedZone = raw.linkedZone;
            entry.multimapId = raw.multimapId;

            if constexpr (requires { { raw.mapName[0] } -> std::convertible_to<const char*>; })
            {
                uint8_t localeId = sWorld.getDbcLocaleLanguageId();
                entry.mapName = raw.mapName[localeId] ? raw.mapName[localeId] : "";
            }
            else if constexpr (requires { { raw.mapName } -> std::convertible_to<const char*>; })
            {
                entry.mapName = raw.mapName ? raw.mapName : "";
            }

            if constexpr (requires { raw.parentMap; })
            {
                entry.parentMap = raw.parentMap;
                entry.startX = raw.startX;
                entry.startY = raw.startY;
                entry.addon = raw.addon;
            }
            else
            {
                // Classic fallback
                entry.parentMap = -1;
                entry.startX = 0.0f;
                entry.startY = 0.0f;
                entry.addon = 0;
            }

            if constexpr (requires { raw.resetRaidTime; })
            {
                entry.resetRaidTime = raw.resetRaidTime;
                entry.resetHeroicTime = raw.resetHeroicTime;
            }
            else
            {
                entry.resetRaidTime = 604800; // Classic default: 7 days
                entry.resetHeroicTime = 0;
            }

            if constexpr (requires { raw.maxPlayers; })
            {
                entry.unkTime = raw.unkTime;
                entry.maxPlayers = raw.maxPlayers;
            }

            if constexpr (requires { raw.nextPhaseMap; })
            {
                entry.nextPhaseMap = raw.nextPhaseMap;
            }
        }
    );


    for (auto const& entry : sMapDifficultyStore | std::views::values)
    {
        uint32_t key = Util::MAKE_PAIR32(static_cast<uint16_t>(entry.mapId), static_cast<uint16_t>(entry.difficulty));
        sMapDifficultyMap[key] = WDB::Structures::MapDifficulty(
            entry.raidDuration,
            entry.maxPlayers,
            !entry.message.empty()
        );
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

    {
        WDB::WDBContainer<WDB::Structures::LegacySkillLineAbilityEntry> legacySkillLineAbilityStore;
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, legacySkillLineAbilityStore, dbc_path, "SkillLineAbility.dbc");

        std::vector<std::pair<uint32_t, WDB::Structures::SkillLineAbilityEntry>> entries;
        entries.reserve(legacySkillLineAbilityStore.getNumRows());
        for (uint32_t id = 0; id < legacySkillLineAbilityStore.getNumRows(); ++id)
        {
            auto const* raw = legacySkillLineAbilityStore.lookupEntry(id);
            if (raw == nullptr)
                continue;

            WDB::Structures::SkillLineAbilityEntry entry{};
            entry.Id = raw->Id;
            entry.skilline = raw->skilline;
            entry.spell = raw->spell;
            entry.races.addLegacyMask(raw->raceMask);
            entry.class_mask = raw->classMask;
            entry.minSkillLineRank = raw->minSkillLineRank;
            entry.next = raw->next;
            entry.acquireMethod = raw->acquireMethod;
            entry.grey = raw->grey;
            entry.green = raw->green;
            entries.emplace_back(entry.Id, entry);
        }
        sSkillLineAbilityStore.assignEntries(entries);
    }
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSkillLineStore, dbc_path, "SkillLine.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellStore, dbc_path, "Spell.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellCastTimesStore, dbc_path, "SpellCastTimes.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellDurationStore, dbc_path, "SpellDuration.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellItemEnchantmentStore, dbc_path, "SpellItemEnchantment.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellRadiusStore, dbc_path, "SpellRadius.dbc"); // todo handle max and level radius
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
        sTaxiPathSetBySource.clear();
        sTaxiPathNodesByPath.clear();

        for (uint32_t i = 1; i < sTaxiPathStore.getNumRows(); ++i)
        {
            if (WDB::Structures::TaxiPathEntry const* entry = sTaxiPathStore.lookupEntry(i))
                sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->id, entry->price);
        }

        uint32_t pathCount = sTaxiPathStore.getNumRows();

        // Calculate path nodes count
        std::vector<uint32_t> pathLength;
        pathLength.resize(pathCount); // 0 and some other indexes not used
        for (uint32_t i = 0; i < sTaxiPathNodeStore.getNumRows(); ++i)
        {
            if (WDB::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.lookupEntry(i))
            {
                if (entry->pathId >= pathLength.size())
                    continue;

                if (pathLength[entry->pathId] < entry->NodeIndex + 1)
                    pathLength[entry->pathId] = entry->NodeIndex + 1;
            }
        }

        // Set path length
        sTaxiPathNodesByPath.resize(pathCount); // 0 and some other indexes not used
        for (uint32_t i = 1; i < sTaxiPathNodesByPath.size(); ++i)
            sTaxiPathNodesByPath[i].resize(pathLength[i]);

        // fill data
        for (uint32_t i = 0; i < sTaxiPathNodeStore.getNumRows(); ++i)
        {
            if (WDB::Structures::TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.lookupEntry(i))
            {
                if (entry->pathId >= sTaxiPathNodesByPath.size() || entry->NodeIndex >= sTaxiPathNodesByPath[entry->pathId].size())
                    continue;

                sTaxiPathNodesByPath[entry->pathId][entry->NodeIndex] = entry;
            }
        }
    }

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTransportAnimationStore, dbc_path, "TransportAnimation.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sWMOAreaTableStore, dbc_path, "WMOAreaTable.dbc");
    {
        sWMOAreaInfoByTripple.clear();
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

#if VERSION_STRING == Mop
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellMiscStore, dbc_path, "SpellMisc.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sChrSpecializationStore, dbc_path, "ChrSpecialization.dbc");
    {
        for (uint32_t i = 0; i < sChrSpecializationStore.getNumRows(); ++i)
        {
            auto const specialization_info = sChrSpecializationStore.lookupEntry(i);
            if (specialization_info == nullptr)
                continue;

            if (specialization_info->classId >= 12 || specialization_info->tabPage >= 4)
                continue;

            ClassSpecializationTabs[specialization_info->classId][specialization_info->tabPage] = specialization_info->Id;
        }
    }
#endif

    /////////////////////////////////////////////////////////////////////////////////////////
    // Load multi version specific dbcs - WotLK, TBC and/or Classic

    WDB::loadUnifiedWDBStore<WDB::Structures::StableSlotPricesEntry>(
        bad_dbc_files, sStableSlotPricesStore, dbc_path,
        [](const auto& raw, WDB::Structures::StableSlotPricesEntry& entry) {
            entry.id = raw.id;
            entry.price = raw.price;
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::CharTitlesEntry>(
        bad_dbc_files, sCharTitlesStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::CharTitlesEntry& entry) {
            entry.id = raw.id;
            entry.bitIndex = raw.bitIndex;

            if constexpr (requires { { raw.nameMale[0] } -> std::convertible_to<const char*>; }) {
                uint8_t localeId = sWorld.getDbcLocaleLanguageId();
                entry.nameMale = raw.nameMale[localeId] ? raw.nameMale[localeId] : "";
            }
            else if constexpr (requires { raw.name; }) {
                entry.nameMale = raw.name ? raw.name : "";
            }
            else if constexpr (requires { raw.nameMale; }) {
                entry.nameMale = raw.nameMale ? raw.nameMale : "";
            }

            if constexpr (requires { { raw.nameFemale[0] } -> std::convertible_to<const char*>; }) {
                uint8_t localeId = sWorld.getDbcLocaleLanguageId();
                entry.nameFemale = raw.nameFemale[localeId] ? raw.nameFemale[localeId] : "";
            }
            else if constexpr (requires { raw.nameFemale; }) {
                entry.nameFemale = raw.nameFemale ? raw.nameFemale : "";
            }
            else {
                entry.nameFemale = entry.nameMale;
            }
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::GemPropertiesEntry>(
        bad_dbc_files, sGemPropertiesStore, dbc_path,
        [](const auto& raw, WDB::Structures::GemPropertiesEntry& entry) {
            entry.id = raw.id;
            entry.enchantmentId = raw.enchantmentId;
            entry.socketMask = raw.socketMask;
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::TotemCategoryEntry>(
        bad_dbc_files, sTotemCategoryStore, dbc_path,
        [](const auto& raw, WDB::Structures::TotemCategoryEntry& entry) {
            entry.id = raw.id;
            entry.categoryType = raw.categoryType;
            entry.categoryMask = raw.categoryMask;
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::WorldMapAreaEntry>(
        bad_dbc_files, sWorldMapAreaStore, dbc_path,
        [](const auto& raw, WDB::Structures::WorldMapAreaEntry& entry) {
            entry.mapId = raw.id;
            entry.zoneId = raw.zoneId;
            entry.continentMapId = raw.continentMapId;
        }
    );

    /////////////////////////////////////////////////////////////////////////////////////////
    // Load multi version specific dbcs available since TBC
#if VERSION_STRING >= TBC

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritStore, dbc_path, "gtChanceToMeleeCrit.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritBaseStore, dbc_path, "gtChanceToMeleeCritBase.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritStore, dbc_path, "gtChanceToSpellCrit.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritBaseStore, dbc_path, "gtChanceToSpellCritBase.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtCombatRatingsStore, dbc_path, "gtCombatRatings.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtRegenMPPerSptStore, dbc_path, "gtRegenMPPerSpt.dbc"); // loaded but not used
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemRandomSuffixStore, dbc_path, "ItemRandomSuffix.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSummonPropertiesStore, dbc_path, "SummonProperties.dbc");

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

    WDB::loadUnifiedWDBStore<WDB::Structures::AreaGroupEntry>(
        bad_dbc_files, sAreaGroupStore, dbc_path,
        [](const auto& raw, WDB::Structures::AreaGroupEntry& entry) {
            entry.id = raw.id;
            entry.nextGroup = raw.nextGroup;

            for (std::size_t i = 0; i < 6; ++i)
            {
                entry.areaId[i] = raw.areaId[i];
            }
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::BarberShopStyleEntry>(
        bad_dbc_files, sBarberShopStyleStore, dbc_path,
        [](const auto& raw, WDB::Structures::BarberShopStyleEntry& entry) {
            entry.id = raw.id;
            entry.type = raw.type;
            entry.race = raw.race;
            entry.gender = raw.gender;
            entry.hairId = raw.hairId;
        }
    );

    /////////////////////////////////////////////////////////////////////////////////////////
    // Load multi version specific dbcs available since WotLK
#if VERSION_STRING >= WotLK
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sAchievementStore, dbc_path, "Achievement.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sAchievementCriteriaStore, dbc_path, "Achievement_Criteria.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sCurrencyTypesStore, dbc_path, "CurrencyTypes.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sDungeonEncounterStore, dbc_path, "DungeonEncounter.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTransportRotationStore, dbc_path, "TransportRotation.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGlyphPropertiesStore, dbc_path, "GlyphProperties.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGlyphSlotStore, dbc_path, "GlyphSlot.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sBarberShopCostBaseStore, dbc_path, "gtBarberShopCostBase.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sHolidaysStore, dbc_path, "Holidays.dbc"); // loaded but not used
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemLimitCategoryStore, dbc_path, "ItemLimitCategory.dbc");

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

    WDB::loadUnifiedWDBStore<WDB::Structures::BannedAddOnsEntry>(
        bad_dbc_files, sBannedAddOnsStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::BannedAddOnsEntry& entry)
        {
            entry.id = raw.id;
        });

#if VERSION_STRING >= Cata
    WDB::loadUnifiedWDBStore<WDB::Structures::ChrPowerTypesEntry>(
        bad_dbc_files, sChrPowerTypesStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::ChrPowerTypesEntry& entry)
        {
            entry.entry = raw.entry;
            entry.classId = raw.classId;
            entry.power = raw.power;
        });

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

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellAuraOptionsStore, dbc_path, "SpellAuraOptions.dbc");
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
    #if VERSION_STRING == Mop
    // note: SpellPower.dbc rows are not keyed by spell id on Mop, map them by their spellId column
    {
        for (uint32_t i = 0; i < sSpellPowerStore.getNumRows(); ++i)
        {
            WDB::Structures::SpellPowerEntry const* spellPower = sSpellPowerStore.lookupEntry(i);
            if (spellPower == nullptr || spellPower->spellId == 0)
                continue;

            // A spell can own several rows (one per shapeshift form). Without a caster there is
            // no form to match against, so keep the first row that does not require one - this
            // is the row the reference picks for a caster without that shapeshift aura.
            auto itr = sSpellPowerMap.find(spellPower->spellId);
            if (itr == sSpellPowerMap.end())
                sSpellPowerMap[spellPower->spellId] = spellPower;
            else if (itr->second->ShapeShiftSpellId != 0 && spellPower->ShapeShiftSpellId == 0)
                itr->second = spellPower;
        }
    }
    #endif
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

    buildMapDifficultyMap();
    buildAreaMapCollection();
    buildPowerIndexByClass();

    return true;
#endif // AE_FOREVER
}

#if VERSION_STRING >= Cata
WDB::Structures::SpellEffectEntry const* GetSpellEffectEntry(uint32_t spellId, uint8_t effect)
{
    WDB::Structures::SpellEffectMap::const_iterator itr = sSpellEffectMap.find(spellId);
    if (itr == sSpellEffectMap.end())
        return nullptr;

    return itr->second.effects[effect];
}

#if VERSION_STRING >= Cata || defined(AE_FOREVER)
uint8_t getPowerIndexByClass(uint8_t playerClass, uint8_t powerType)
{
    if (playerClass >= MAX_PLAYER_CLASSES || powerType >= TOTAL_PLAYER_POWER_TYPES)
#if defined(AE_FOREVER)
        return 0;
#else
        return TOTAL_PLAYER_POWER_TYPES;
#endif

    return powerIndexByClass[playerClass][powerType];
}
#endif
#endif

#if VERSION_STRING == Mop
WDB::Structures::SpellPowerEntry const* getSpellPowerEntry(uint32_t spellId)
{
    WDB::Structures::SpellPowerMap::const_iterator itr = sSpellPowerMap.find(spellId);
    if (itr == sSpellPowerMap.end())
        return nullptr;

    return itr->second;
}
#elif defined(AE_FOREVER)
// Copied from MoP as a temporary baseline. Replace with dedicated Forever values once verified.
WDB::Structures::SpellPowerEntry const* getSpellPowerEntry(uint32_t spellId)
{
    WDB::Structures::SpellPowerMap::const_iterator itr = sSpellPowerMap.find(spellId);
    if (itr == sSpellPowerMap.end())
        return nullptr;

    return itr->second;
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
            tmpDiff -= 1; // any non-normal mode for raids like tbc (only one mode)

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

#if defined(AE_FOREVER)
WDB::Structures::ChrModelEntry const* getForeverChrModel(uint8_t race, uint8_t gender)
{
    for (auto const& [id, relation] : sChrRaceXChrModelStore)
    {
        (void)id;
        if (relation.chrRacesId != race || relation.sex != static_cast<int8_t>(gender))
            continue;

        if (relation.chrModelId <= 0)
            return nullptr;

        return sChrModelStore.lookupEntry(static_cast<uint32_t>(relation.chrModelId));
    }

    return nullptr;
}
#endif

WDB::Structures::CharStartOutfitEntry const* getStartOutfitByRaceClass(uint8_t race, uint8_t class_, uint8_t gender)
{
    const auto itr = sCharStartOutfitMap.find(race | (class_ << 8) | (gender << 16));
    if (itr != sCharStartOutfitMap.end())
        return itr->second;

    return nullptr;
}

WDB::Structures::MapDifficulty const* getMapDifficultyData(uint32_t mapId, InstanceDifficulty::Difficulties difficulty)
{
    MapDifficultyMap::const_iterator itr = sMapDifficultyMap.find(Util::MAKE_PAIR32(static_cast<uint16_t>(mapId), difficulty));
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

#if VERSION_STRING == Mop
uint32_t const* getClassSpecializations(uint8_t playerClass)
{
    return ClassSpecializationTabs[playerClass];
}
#elif defined(AE_FOREVER)
// Copied from MoP as a temporary baseline. Replace with dedicated Forever values once verified.
uint32_t const* getClassSpecializations(uint8_t playerClass)
{
    return ClassSpecializationTabs[playerClass];
}
#endif

uint32_t getLiquidFlags(uint32_t liquidType)
{
    if (WDB::Structures::LiquidTypeEntry const* liq = sLiquidTypeStore.lookupEntry(liquidType))
        return 1 << liq->Type;

    return 0;
}
