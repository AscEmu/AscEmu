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
#if VERSION_STRING >= Cata
    #include "Objects/Units/Players/PlayerDefines.hpp"
    #include "Spell/SpellAura.hpp"
#endif

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <map>
#include <string>
#include <type_traits>
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
// Copied from MoP as a temporary baseline. Replace with dedicated Forever values once verified.
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellMiscEntry> sSpellMiscStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::ChrSpecializationEntry> sChrSpecializationStore;
WDB::Structures::SpellPowerMap sSpellPowerMap;
#endif

namespace {
#if defined(AE_FOREVER)
    namespace ForeverFormat = WDB::Formats::Forever;

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
            sLogger.info("Loaded {} DB2 table.", format.filename);
            return true;
        }

        errors.push_back("Forever WDC5: " + error);
        sLogger.failure("Failed to load {} DB2 table.", format.filename);
        return false;
    }

    bool loadForeverWDC5Group(std::initializer_list<ForeverWDC5Load> loads, WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        for (ForeverWDC5Load const& load : loads)
            if (!loadForeverWDC5(load.file, load.format, errors, dbcPath))
                return false;

        return true;
    }

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
        WDB::WDC5File chrRaces;
        WDB::WDC5File faction;
        WDB::WDC5File factionTemplate;

        if (!loadForeverWDC5Group({ { chrClasses, ForeverFormat::ChrClasses }, { chrRaces, ForeverFormat::ChrRaces }, { faction, ForeverFormat::Faction }, { factionTemplate, ForeverFormat::FactionTemplate } }, errors, dbcPath))
            return false;

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

        // Faction.db2 in the 1.60.1.69800 beta has one additional modern field
        // compared with the current Forever metadata. The fields required by
        // AscEmu during login are stable at the front of the record, so keep
        // the unknown tail opaque for now instead of guessing its meaning.
        sFactionStore.clear();
        for (uint32_t row = 0; row < faction.getRecordCount(); ++row)
        {
            WDB::Structures::FactionEntry entry;
            entry.id = faction.getRecordId(row);
            entry.reputationIndex = faction.getInt32(row, 2);
            entry.parentFactionId = faction.getUInt32(row, 3);
            entry.expansion = faction.getUInt8(row, 4);
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

        if (!loadForeverWDC5Group({
            { customization, ForeverFormat::ChrCustomization }, { boneSet, ForeverFormat::ChrCustomizationBoneSet }, { category, ForeverFormat::ChrCustomizationCategory },
            { choice, ForeverFormat::ChrCustomizationChoice }, { condModel, ForeverFormat::ChrCustomizationCondModel }, { conversion, ForeverFormat::ChrCustomizationConversion },
            { displayInfo, ForeverFormat::ChrCustomizationDisplayInfo }, { element, ForeverFormat::ChrCustomizationElement }, { geoset, ForeverFormat::ChrCustomizationGeoset },
            { glyphPet, ForeverFormat::ChrCustomizationGlyphPet }, { material, ForeverFormat::ChrCustomizationMaterial }, { option, ForeverFormat::ChrCustomizationOption },
            { req, ForeverFormat::ChrCustomizationReq }, { reqChoice, ForeverFormat::ChrCustomizationReqChoice }, { skinnedModel, ForeverFormat::ChrCustomizationSkinnedModel },
            { visReq, ForeverFormat::ChrCustomizationVisReq }, { voice, ForeverFormat::ChrCustomizationVoice } }, errors, dbcPath))
            return false;

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
        WDB::WDC5File itemSet;
        WDB::WDC5File itemSetSpell;
        if (!loadForeverWDC5Group({ { itemSet, ForeverFormat::ItemSet }, { itemSetSpell, ForeverFormat::ItemSetSpell } }, errors, dbcPath))
            return false;

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
        // Trinity DB2 set. Keep the legacy stores available to old AscEmu code,
        // but intentionally empty instead of reporting missing .dbc files.
        sItemRandomPropertiesStore.clear();
        sItemRandomSuffixStore.clear();

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

        sLogger.info("Forever terrain DB2 stores: AreaTable={} LiquidType={} WMOAreaTable={}",
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
        for (auto& classPowers : powerIndexByClass)
        {
            classPowers.fill(TOTAL_PLAYER_POWER_TYPES);
        }

        for (auto const& powerEntry : sChrPowerTypesStore | std::views::values)
        {
            // Boundary Checks against Out-of-Bounds access
            if (powerEntry.classId >= MAX_PLAYER_CLASSES || powerEntry.power >= TOTAL_PLAYER_POWER_TYPES)
                continue;

            uint8_t index = 1;
            for (uint8_t power = POWER_TYPE_MANA; power < TOTAL_PLAYER_POWER_TYPES; ++power)
            {
                if (powerIndexByClass[powerEntry.classId][power] != TOTAL_PLAYER_POWER_TYPES)
                    ++index;
            }

            powerIndexByClass[powerEntry.classId][powerEntry.power] = index;
        }
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
    // Load the modern Forever WDC5 character baseline into the existing
    // AscEmu runtime stores. ChrClasses/ChrRaces replace their legacy DBC
    // sources for Forever; the rest of the core can keep using the same API.
    loadForeverModernCharacterStores(bad_dbc_files, dbc_path);
    loadForeverModernCreatureStores(bad_dbc_files, dbc_path);
    loadForeverModernCustomizationStores(bad_dbc_files, dbc_path);
    loadForeverModernTaxiStores(bad_dbc_files, dbc_path);
    loadForeverModernItemStores(bad_dbc_files, dbc_path);
    loadForeverModernMapStores(bad_dbc_files, dbc_path);
    loadForeverModernTerrainStores(bad_dbc_files, dbc_path);
#endif

    // Load ChrClasses.dbc first to ensure the dbcLocaleId is set correctly before loading other DBC files that may depend on it
#if !defined(AE_FOREVER)
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
#endif

#if !defined(AE_FOREVER)
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

#endif

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

#if !defined(AE_FOREVER)
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
#endif

#if !defined(AE_FOREVER)
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

#endif

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

#if !defined(AE_FOREVER)
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

#endif

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGameObjectDisplayInfoStore, dbc_path, "GameObjectDisplayInfo.dbc");

#if !defined(AE_FOREVER)
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemSetStore, dbc_path, "ItemSet.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemRandomPropertiesStore, dbc_path, "ItemRandomProperties.dbc");
#endif

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sLFGDungeonStore, dbc_path, "LFGDungeons.dbc");
#if !defined(AE_FOREVER)
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sLiquidTypeStore, dbc_path, "LiquidType.dbc");
#endif
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sLockStore, dbc_path, "Lock.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sMailTemplateStore, dbc_path, "MailTemplate.dbc");

#if !defined(AE_FOREVER)
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

#endif

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

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSkillLineAbilityStore, dbc_path, "SkillLineAbility.dbc");
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

#if !defined(AE_FOREVER)
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTaxiNodesStore, dbc_path, "TaxiNodes.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTaxiPathStore, dbc_path, "TaxiPath.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTaxiPathNodeStore, dbc_path, "TaxiPathNode.dbc");
#endif
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

#if !defined(AE_FOREVER)
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sWMOAreaTableStore, dbc_path, "WMOAreaTable.dbc");
#endif
    {
        sWMOAreaInfoByTripple.clear();
        for (uint32_t i = 0; i < sWMOAreaTableStore.getNumRows(); ++i)
        {
            if (auto entry = sWMOAreaTableStore.lookupEntry(i))
                sWMOAreaInfoByTripple.insert(WMOAreaInfoByTripple::value_type(WMOAreaTableTripple(entry->rootId, entry->adtId, entry->groupId), entry));
        }
    }

#if !defined(AE_FOREVER)
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sWorldMapOverlayStore, dbc_path, "WorldMapOverlay.dbc");
#endif

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
#elif defined(AE_FOREVER)
// Copied from MoP as a temporary baseline. Replace with dedicated Forever values once verified.
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

#if !defined(AE_FOREVER)
    WDB::loadUnifiedWDBStore<WDB::Structures::WorldMapAreaEntry>(
        bad_dbc_files, sWorldMapAreaStore, dbc_path,
        [](const auto& raw, WDB::Structures::WorldMapAreaEntry& entry) {
            entry.mapId = raw.id;
            entry.zoneId = raw.zoneId;
            entry.continentMapId = raw.continentMapId;
        }
    );
#endif

    /////////////////////////////////////////////////////////////////////////////////////////
    // Load multi version specific dbcs available since TBC
#if VERSION_STRING >= TBC

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritStore, dbc_path, "gtChanceToMeleeCrit.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtChanceToMeleeCritBaseStore, dbc_path, "gtChanceToMeleeCritBase.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritStore, dbc_path, "gtChanceToSpellCrit.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtChanceToSpellCritBaseStore, dbc_path, "gtChanceToSpellCritBase.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtCombatRatingsStore, dbc_path, "gtCombatRatings.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtRegenMPPerSptStore, dbc_path, "gtRegenMPPerSpt.dbc"); // loaded but not used
    #if !defined(AE_FOREVER)
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemRandomSuffixStore, dbc_path, "ItemRandomSuffix.dbc");
    #endif
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
    #elif defined(AE_FOREVER)
    // Copied from MoP as a temporary baseline. Replace with dedicated Forever values once verified.
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
