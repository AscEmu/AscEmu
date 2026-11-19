/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WDBStores.hpp"

#include "AEVersion.hpp"
#include "Server/World.h"
#include "WDBContainer.hpp"
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

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <map>
#include <ranges>
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

SERVER_DECL WDB::WDBStore<WDB::Structures::ItemExtendedCostEntry> sItemExtendedCostStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemRandomPropertiesEntry> sItemRandomPropertiesStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::LFGDungeonEntry> sLFGDungeonStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::LiquidTypeEntry> sLiquidTypeStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::LockEntry> sLockStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::NameGenEntry> sNameGenStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::SkillLineAbilityEntry> sSkillLineAbilityStore;

SERVER_DECL std::unordered_map<uint32_t, std::vector<uint32_t>> sSkillLineChildren;
SERVER_DECL std::unordered_map<uint32_t, std::vector<ClientItemEffect>> sItemEffects;

SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellCastTimesEntry> sSpellCastTimesStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellDurationEntry> sSpellDurationStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellRadiusEntry> sSpellRadiusStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::TalentTabEntry> sTalentTabStore;
static uint32_t InspectTalentTabPages[12][3];
// bit position of the first rank of a talent inside its tab and the bit size of a tab (talent inspect before WotLK)
static std::map<uint32_t, uint32_t> InspectTalentTabPos;
static std::map<uint32_t, uint32_t> InspectTalentTabSize;

static uint32_t ClassSpecializationTabs[12][4]; // >= MoP: specialization tabs for each class

TaxiPathSetBySource sTaxiPathSetBySource;
SERVER_DECL WDB::WDBContainer<WDB::Structures::TaxiPathEntry> sTaxiPathStore;
TaxiPathNodesByPath sTaxiPathNodesByPath;

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

#if VERSION_STRING < Cata
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTRegenHPEntry> sGtOCTRegenHPStore; // todo: available for versions > Classic
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtRegenHPPerSptEntry> sGtRegenHPPerSptStore; // todo: available for versions > Classic
#endif

SERVER_DECL WDB::WDBContainer<WDB::Structures::ChrSpecializationEntry> sChrSpecializationStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::GlyphPropertiesEntry> sGlyphPropertiesStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::DungeonEncounterEntry> sDungeonEncounterStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::TransportRotationEntry> sTransportRotationStore;


SERVER_DECL WDB::WDBContainer<WDB::Structures::GlyphSlotEntry> sGlyphSlotStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtBarberShopCostBaseEntry> sBarberShopCostBaseStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::HolidaysEntry> sHolidaysStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::QuestXP> sQuestXPStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::ScalingStatDistributionEntry> sScalingStatDistributionStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::ScalingStatValuesEntry> sScalingStatValuesStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellDifficultyEntry> sSpellDifficultyStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellRuneCostEntry> sSpellRuneCostStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTBaseHPByClassEntry> sGtOCTBaseHPByClassStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTBaseMPByClassEntry> sGtOCTBaseMPByClassStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::GuildPerkSpellsEntry> sGuildPerkSpellsStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::GtOCTClassCombatRatingScalarEntry> sGtOCTClassCombatRatingScalarStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::EmotesEntry> sEmotesStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemCurrencyCostEntry> sItemCurrencyCostStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::MountCapabilityEntry> sMountCapabilityStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::MountTypeEntry> sMountTypeStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::NumTalentsAtLevel> sNumTalentsAtLevel;

SERVER_DECL WDB::WDBContainer<WDB::Structures::PhaseEntry> sPhaseStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::QuestSortEntry> sQuestSortStore;

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
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellScalingEntry> sSpellScalingStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellReagentsEntry> sSpellReagentsStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellShapeshiftEntry> sSpellShapeshiftStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellTargetRestrictionsEntry> sSpellTargetRestrictionsStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::SpellTotemsEntry> sSpellTotemsStore;

SERVER_DECL WDB::WDBContainer<WDB::Structures::TalentTreePrimarySpells> sTalentTreePrimarySpellsStore;

#ifdef AE_CATA
SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemReforgeEntry> sItemReforgeStore;
#endif

SERVER_DECL WDB::WDBContainer<WDB::Structures::BattlePetSpeciesEntry> sBattlePetSpeciesStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::BattlePetBreedStateEntry> sBattlePetBreedStateStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::BattlePetSpeciesStateEntry> sBattlePetSpeciesStateStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::BattlePetBreedQualityEntry> sBattlePetBreedQualityStore;
SERVER_DECL WDB::WDBContainer<WDB::Structures::ItemToBattlePetEntry> sItemToBattlePetStore;

WDB::Structures::SpellPowerMap sSpellPowerMap;

namespace {
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

        // the client numbers the powers of a class in the row order of the file, the store has no order of its own
        std::vector<WDB::Structures::ChrPowerTypesEntry const*> rows;
        rows.reserve(sChrPowerTypesStore.size());
        for (auto const& powerEntry : sChrPowerTypesStore | std::views::values)
            rows.push_back(&powerEntry);
        std::ranges::sort(rows, {}, &WDB::Structures::ChrPowerTypesEntry::entry);

        for (auto const* row : rows)
        {
            auto const& powerEntry = *row;
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

    // The lookups built from the stores below do not depend on the file format
    // the stores were filled from, the DB2 path shares them with the dbc path.
    void buildCharStartOutfitMap()
    {
        sCharStartOutfitMap.clear();
        for (auto const& [id, outfit] : sCharStartOutfitStore)
        {
            sCharStartOutfitMap[outfit.makeKey()] = &outfit;
        }
    }

    void buildNameGenData()
    {
        for (auto& names : _namegenData)
            names.clear();

        for (uint32_t i = 0; i < sNameGenStore.getNumRows(); ++i)
        {
            const auto name_gen_entry = sNameGenStore.lookupEntry(i);
            if (name_gen_entry == nullptr)
                continue;

            NameGenData nameGenData;
            nameGenData.name = std::string(name_gen_entry->Name);
            nameGenData.type = name_gen_entry->type;
            if (nameGenData.type < 3)
                _namegenData[nameGenData.type].push_back(nameGenData);
        }
    }

    void buildTaxiPathData()
    {
        uint32_t maxPathId = 0;

        for (uint32_t i = 1; i < sTaxiPathStore.getNumRows(); ++i)
        {
            if (WDB::Structures::TaxiPathEntry const* entry = sTaxiPathStore.lookupEntry(i))
            {
                sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->id, entry->price);
                maxPathId = std::max(maxPathId, entry->id);
            }
        }

        uint32_t const pathCount = maxPathId + 1;

        // Calculate path nodes count
        std::vector<uint32_t> pathLength(pathCount, 0);
        for (const auto& node : sTaxiPathNodeStore | std::views::values)
        {
            if (node.pathId >= pathCount)
                continue;

            pathLength[node.pathId] = std::max(pathLength[node.pathId], node.nodeIndex + 1);
        }

        // Set path length
        sTaxiPathNodesByPath.resize(pathCount); // 0 and some other indexes not used
        for (uint32_t i = 0; i < sTaxiPathNodesByPath.size(); ++i)
            sTaxiPathNodesByPath[i].resize(pathLength[i]);

        // fill data
        for (const auto& node : sTaxiPathNodeStore | std::views::values)
        {
            if (node.pathId < pathCount && node.nodeIndex < sTaxiPathNodesByPath[node.pathId].size())
                sTaxiPathNodesByPath[node.pathId][node.nodeIndex] = &node;
        }
    }

    void buildWmoAreaTripples()
    {
        for (uint32_t i = 0; i < sWMOAreaTableStore.getNumRows(); ++i)
        {
            if (auto entry = sWMOAreaTableStore.lookupEntry(i))
                sWMOAreaInfoByTripple.insert(WMOAreaInfoByTripple::value_type(WMOAreaTableTripple(entry->rootId, entry->adtId, entry->groupId), entry));
        }
    }


    void buildClassSpecializationTabs()
    {
        std::memset(ClassSpecializationTabs, 0, sizeof(ClassSpecializationTabs));
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

    void buildSpellCategoryStore()
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

    void buildSpellEffectMap()
    {
        for (uint32_t i = 1; i < sSpellEffectStore.getNumRows(); ++i)
        {
            if (WDB::Structures::SpellEffectEntry const* spellEffect = sSpellEffectStore.lookupEntry(i))
            {
                sSpellEffectMap[spellEffect->EffectSpellId].effects[spellEffect->EffectIndex] = spellEffect;
            }
        }
    }

    // note: SpellPower rows are not keyed by spell id on Mop, map them by their spellId column
    void buildSpellPowerMap()
    {
        if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Mop))
            return;

        for (uint32_t i = 0; i < sSpellPowerStore.getNumRows(); ++i)
        {
            WDB::Structures::SpellPowerEntry const* spellPower = sSpellPowerStore.lookupEntry(i);
            if (spellPower == nullptr || spellPower->spellId == 0)
                continue;

            // one row per shapeshift form, without a caster the first row without a form counts
            auto itr = sSpellPowerMap.find(spellPower->spellId);
            if (itr == sSpellPowerMap.end())
                sSpellPowerMap[spellPower->spellId] = spellPower;
            else if (itr->second->shapeShiftSpellId != 0 && spellPower->shapeShiftSpellId == 0)
                itr->second = spellPower;
        }
    }
}

namespace
{
    // the text of the locale of the server, the files before 4.x carry one per locale
    template <typename Text>
    std::string localizedText(Text const& text)
    {
        if constexpr (std::is_array_v<Text>)
        {
            char const* const value = text[sWorld.getDbcLocaleLanguageId()];
            return value != nullptr ? value : "";
        }
        else
        {
            return text != nullptr ? text : "";
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

    // A client with its own data: the stores are filled from its DB2 tables,
    // only the lookups built from the stores are shared with the dbc path.
    if (WoW::hasOwnClientData(WoW::getServerExpansion()))
    {
        bool const loaded = loadDb2Stores(bad_dbc_files, dbc_path);
        for (std::string const& problem : bad_dbc_files)
            sLogger.failure("DB2: {}", problem);

        buildCharStartOutfitMap();
        buildNameGenData();
        buildTaxiPathData();
        buildWmoAreaTripples();
        buildClassSpecializationTabs();
        buildSpellCategoryStore();
        buildSpellEffectMap();
        buildSpellPowerMap();

        buildMapDifficultyMap();
        buildAreaMapCollection();
        buildPowerIndexByClass();

        return loaded;
    }

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
                            sLogger.info("DBC: Auto-detected locale ID {} ({}) from ChrClasses.dbc",
                                         detectedLocale,
                                         Util::getLanguagesStringFromId(detectedLocale));
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

    buildCharStartOutfitMap();

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

    // achievements and their conditions, a version without the file keeps the store empty
    WDB::loadUnifiedWDBStore<WDB::Structures::AchievementEntry>(
        bad_dbc_files, sAchievementStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::AchievementEntry& entry)
        {
            entry.ID = raw.id;
            entry.factionFlag = raw.factionFlag;
            entry.mapID = raw.mapId;
            entry.parentAchievement = raw.parentAchievement;
            entry.name = localizedText(raw.name);
            entry.description = localizedText(raw.description);
            entry.categoryId = raw.categoryId;
            entry.points = raw.points;
            entry.orderInCategory = raw.orderInCategory;
            entry.flags = raw.flags;
            entry.icon = raw.icon;
            entry.rewardName = localizedText(raw.rewardName);
            entry.count = raw.count;
            entry.refAchievement = raw.refAchievement;
            if constexpr (requires { raw.criteriaTreeId; })
                entry.criteriaTreeID = raw.criteriaTreeId;
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::AchievementCategoryEntry>(
        bad_dbc_files, sAchievementCategoryStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::AchievementCategoryEntry& entry)
        {
            entry.id = raw.id;
            entry.parent = raw.parent;
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::AchievementCriteriaEntry>(
        bad_dbc_files, sAchievementCriteriaStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::AchievementCriteriaEntry& entry)
        {
            entry.ID = raw.id;
            entry.referredAchievement = raw.achievementId;
            entry.requiredType = raw.type;
            entry.asset = raw.asset;
            entry.quantity = raw.quantity;
            entry.startEvent = raw.startEvent;
            entry.startAsset = raw.startAsset;
            entry.failEvent = raw.failEvent;
            entry.failAsset = raw.failAsset;
            entry.name = localizedText(raw.name);
            entry.flags = raw.flags;
            entry.timerStartEvent = raw.timerStartEvent;
            entry.timerAsset = raw.timerAsset;
            entry.timerTime = raw.timerTime;
            entry.order = raw.order;
            if constexpr (requires { raw.worldStateId; })
            {
                entry.worldStateId = raw.worldStateId;
                entry.worldStateValue = raw.worldStateValue;
                for (size_t i = 0; i < entry.conditionType.size(); ++i)
                {
                    entry.conditionType[i] = raw.conditionType[i];
                    entry.conditionValue[i] = raw.conditionValue[i];
                }
            }
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::CriteriaEntry>(
        bad_dbc_files, sCriteriaStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::CriteriaEntry& entry)
        {
            entry.id = raw.id;
            entry.type = raw.type;
            entry.asset = raw.asset;
            entry.startEvent = raw.startEvent;
            entry.startAsset = raw.startAsset;
            entry.startTimer = raw.startTimer;
            entry.failEvent = raw.failEvent;
            entry.failAsset = raw.failAsset;
            entry.modifierTreeId = raw.modifierTreeId;
            entry.flags = raw.flags;
            entry.eligibilityWorldStateId = raw.eligibilityWorldStateId;
            entry.eligibilityWorldStateValue = raw.eligibilityWorldStateValue;
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::CriteriaTreeEntry>(
        bad_dbc_files, sCriteriaTreeStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::CriteriaTreeEntry& entry)
        {
            entry.id = raw.id;
            entry.criteriaId = raw.criteriaId;
            entry.amount = raw.amount;
            entry.operatorType = raw.operatorType;
            entry.parent = raw.parent;
            entry.flags = raw.flags;
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::ModifierTreeEntry>(
        bad_dbc_files, sModifierTreeStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::ModifierTreeEntry& entry)
        {
            entry.id = raw.id;
            entry.type = raw.type;
            entry.asset = raw.asset;
            entry.secondaryAsset = raw.secondaryAsset;
            entry.operatorType = raw.operatorType;
            entry.amount = raw.amount;
            entry.parent = raw.parent;
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::PlayerConditionEntry>(
        bad_dbc_files, sPlayerConditionStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::PlayerConditionEntry& entry)
        {
            entry.id = raw.id;
            for (size_t i = 0; i < entry.values.size(); ++i)
                entry.values[i] = raw.values[i];
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::WorldStateExpressionEntry>(
        bad_dbc_files, sWorldStateExpressionStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::WorldStateExpressionEntry& entry)
        {
            entry.id = raw.id;
            entry.expression = localizedText(raw.expression);
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::CurrencyTypesEntry>(
        bad_dbc_files, sCurrencyTypesStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::CurrencyTypesEntry& entry)
        {
            entry.item_id = raw.id;
            if constexpr (requires { raw.bitIndex; })
            {
                entry.bit_index = raw.bitIndex;
            }
            else
            {
                entry.Category = raw.category;
                entry.name = localizedText(raw.name);
                entry.TotalCap = raw.totalCap;
                entry.WeekCap = raw.weekCap;
                entry.Flags = raw.flags;
            }
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::FriendshipRepReactionEntry>(
        bad_dbc_files, sFriendshipRepReactionStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::FriendshipRepReactionEntry& entry)
        {
            entry.id = raw.id;
            entry.friendshipRepId = raw.friendshipRepId;
            entry.reactionThreshold = raw.reactionThreshold;
        });

    WDB::loadUnifiedWDBStore<WDB::Structures::FriendshipReputationEntry>(
        bad_dbc_files, sFriendshipReputationStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::FriendshipReputationEntry& entry)
        {
            entry.id = raw.id;
            entry.factionId = raw.factionId;
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

    WDB::loadUnifiedWDBStore<WDB::Structures::GameObjectDisplayInfoEntry>(
        bad_dbc_files, sGameObjectDisplayInfoStore, dbc_path,
        [](const auto& raw, WDB::Structures::GameObjectDisplayInfoEntry& entry) {
            entry.id = raw.id;
            entry.filename = raw.filename;

            if constexpr (requires { raw.geoBoxMin; raw.geoBoxMax; })
            {
                for (std::size_t i = 0; i < 3; ++i)
                {
                    entry.geoBoxMin[i] = raw.geoBoxMin[i];
                    entry.geoBoxMax[i] = raw.geoBoxMax[i];
                }
            }
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::ItemSetEntry>(
        bad_dbc_files, sItemSetStore, dbc_path,
        [](const auto& raw, WDB::Structures::ItemSetEntry& entry) {
            entry.id = raw.id;

            if constexpr (std::is_array_v<decltype(raw.name)>)
            {
                entry.name = raw.name[0];
            }
            else
            {
                entry.name = raw.name;
            }

            for (std::size_t i = 0; i < WDB::Structures::MAX_ITEM_SET_ITEMS; ++i)
            {
                entry.itemId[i] = raw.itemId[i];
            }

            for (std::size_t i = 0; i < WDB::Structures::MAX_ITEM_SET_SPELLS; ++i)
            {
                entry.spellId[i] = raw.spellId[i];
                entry.itemsCount[i] = raw.itemsCount[i];
            }

            entry.requiredSkillId = raw.requiredSkillId;
            entry.requiredSkillAmt = raw.requiredSkillAmt;
        }
    );

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemRandomPropertiesStore, dbc_path, "ItemRandomProperties.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sLFGDungeonStore, dbc_path, "LFGDungeons.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sLiquidTypeStore, dbc_path, "LiquidType.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sLockStore, dbc_path, "Lock.dbc");

    WDB::loadUnifiedWDBStore<WDB::Structures::MailTemplateEntry>(
        bad_dbc_files, sMailTemplateStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::MailTemplateEntry& entry) {
            entry.id = raw.id;

            if constexpr (requires { raw.subject; })
            {
                entry.subject = raw.subject ? raw.subject : "";
                entry.content = raw.content ? raw.content : "";
            }
        }
    );

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
    buildNameGenData();

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSkillLineAbilityStore, dbc_path, "SkillLineAbility.dbc");

    WDB::loadUnifiedWDBStore<WDB::Structures::SkillLineEntry>(
        bad_dbc_files, sSkillLineStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::SkillLineEntry& entry) {
            entry.id = raw.id;
            entry.type = raw.type;

            if constexpr (std::is_array_v<decltype(raw.name)>)
            {
                constexpr size_t localeCount = std::extent_v<decltype(raw.name)>;
                for (size_t i = 0; i < localeCount; ++i)
                    entry.name[i] = raw.name[i] ? raw.name[i] : "";
            }
            else
            {
                entry.name[0] = raw.name ? raw.name : "";
            }

            entry.spellIcon = raw.spellIcon;

            if constexpr (requires { raw.linkable; })
                entry.linkable = raw.linkable;
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::SpellEntry>(
        bad_dbc_files, sSpellStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::SpellEntry& entry) {
            entry.id = raw.id;

            // Localized names & ranks (Classic: 8, TBC/WotLK: 16, Cata/MoP: single string)
            if constexpr (std::is_array_v<decltype(raw.name)>)
            {
                constexpr size_t localeCount = std::extent_v<decltype(raw.name)>;
                for (size_t i = 0; i < localeCount; ++i)
                {
                    entry.name[i] = raw.name[i] ? raw.name[i] : "";
                    entry.rank[i] = raw.rank[i] ? raw.rank[i] : "";
                }
            }
            else
            {
                entry.name[0] = raw.name ? raw.name : "";
                entry.rank[0] = raw.rank ? raw.rank : "";
            }

            if constexpr (requires { raw.school; })
                entry.school = raw.school;

            if constexpr (requires { raw.attributes; })
            {
                entry.attributes = raw.attributes;
                entry.attributesEx = raw.attributesEx;
                entry.attributesExB = raw.attributesExB;
                entry.attributesExC = raw.attributesExC;
                entry.attributesExD = raw.attributesExD;
                entry.castingTimeIndex = raw.castingTimeIndex;
                entry.durationIndex = raw.durationIndex;
                entry.powerType = raw.powerType;
                entry.rangeIndex = raw.rangeIndex;
                entry.speed = raw.speed;
                entry.spellVisual = raw.spellVisual;
                entry.spellVisual1 = raw.spellVisual1;
                entry.spellIconId = raw.spellIconId;
                entry.activeIconId = raw.activeIconId;
            }

            if constexpr (requires { raw.attributesExE; })
            {
                entry.attributesExE = raw.attributesExE;
                entry.attributesExF = raw.attributesExF;
            }

            if constexpr (requires { raw.attributesExG; })
                entry.attributesExG = raw.attributesExG;

            if constexpr (requires { raw.attributesExH; })
            {
                entry.attributesExH = raw.attributesExH;
                entry.attributesExI = raw.attributesExI;
                entry.attributesExJ = raw.attributesExJ;
            }

            if constexpr (requires { raw.category; })
            {
                entry.category = raw.category;
                entry.dispelType = raw.dispelType;
                entry.mechanicsType = raw.mechanicsType;
                entry.shapeshifts = raw.shapeshifts;
                entry.shapeshiftsExcluded = raw.shapeshiftsExcluded;
                entry.targets = raw.targets;
                entry.targetCreatureType = raw.targetCreatureType;
                entry.requiresSpellFocus = raw.requiresSpellFocus;
                entry.casterAuraState = raw.casterAuraState;
                entry.targetAuraState = raw.targetAuraState;
                entry.recoveryTime = raw.recoveryTime;
                entry.categoryRecoveryTime = raw.categoryRecoveryTime;
                entry.interruptFlags = raw.interruptFlags;
                entry.auraInterruptFlags = raw.auraInterruptFlags;
                entry.channelInterruptFlags = raw.channelInterruptFlags;
                entry.procFlags = raw.procFlags;
                entry.procChance = raw.procChance;
                entry.procCharges = raw.procCharges;
                entry.maxLevel = raw.maxLevel;
                entry.baseLevel = raw.baseLevel;
                entry.spellLevel = raw.spellLevel;
                entry.manaCost = raw.manaCost;
                entry.manaCostPerLevel = raw.manaCostPerLevel;
                entry.manaPerSecond = raw.manaPerSecond;
                entry.manaPerSecondPerLevel = raw.manaPerSecondPerLevel;
                entry.maxStackAmount = raw.maxStackAmount;

                for (size_t i = 0; i < MAX_SPELL_TOTEMS; ++i)
                    entry.totem[i] = raw.totem[i];
                for (size_t i = 0; i < MAX_SPELL_REAGENTS; ++i)
                {
                    entry.reagent[i] = raw.reagent[i];
                    entry.reagentCount[i] = raw.reagentCount[i];
                }

                entry.equippedItemClass = raw.equippedItemClass;
                entry.equippedItemSubClass = raw.equippedItemSubClass;
                entry.equippedItemInventoryTypeMask = raw.equippedItemInventoryTypeMask;

                for (size_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
                {
                    entry.effect[i] = raw.effect[i];
                    entry.effectDieSides[i] = raw.effectDieSides[i];
                    entry.effectRealPointsPerLevel[i] = raw.effectRealPointsPerLevel[i];
                    entry.effectBasePoints[i] = raw.effectBasePoints[i];
                    entry.effectMechanic[i] = raw.effectMechanic[i];
                    entry.effectImplicitTargetA[i] = raw.effectImplicitTargetA[i];
                    entry.effectImplicitTargetB[i] = raw.effectImplicitTargetB[i];
                    entry.effectRadiusIndex[i] = raw.effectRadiusIndex[i];
                    entry.effectApplyAuraName[i] = raw.effectApplyAuraName[i];
                    entry.effectAmplitude[i] = raw.effectAmplitude[i];
                    entry.effectMultipleValue[i] = raw.effectMultipleValue[i];
                    entry.effectChainTarget[i] = raw.effectChainTarget[i];
                    entry.effectItemType[i] = raw.effectItemType[i];
                    entry.effectMiscValue[i] = raw.effectMiscValue[i];
                    entry.effectTriggerSpell[i] = raw.effectTriggerSpell[i];
                    entry.effectPointsPerComboPoint[i] = raw.effectPointsPerComboPoint[i];
                    entry.effectDamageMultiplier[i] = raw.effectDamageMultiplier[i];
                }

                entry.spellPriority = raw.spellPriority;
                entry.manaCostPercentage = raw.manaCostPercentage;
                entry.startRecoveryCategory = raw.startRecoveryCategory;
                entry.startRecoveryTime = raw.startRecoveryTime;
                entry.maxTargetLevel = raw.maxTargetLevel;
                entry.spellFamilyName = raw.spellFamilyName;
                entry.maxTargets = raw.maxTargets;
                entry.dmgClass = raw.dmgClass;
                entry.preventionType = raw.preventionType;

                constexpr size_t familyFlagCount = std::extent_v<decltype(raw.spellFamilyFlags)>;
                for (size_t i = 0; i < familyFlagCount; ++i)
                    entry.spellFamilyFlags[i] = raw.spellFamilyFlags[i];
            }

            if constexpr (requires { raw.effectBaseDice; })
            {
                for (size_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
                {
                    entry.effectBaseDice[i] = raw.effectBaseDice[i];
                    entry.effectDicePerLevel[i] = raw.effectDicePerLevel[i];
                }
            }

            if constexpr (requires { raw.facingCasterFlags; })
            {
                entry.facingCasterFlags = raw.facingCasterFlags;
                entry.casterAuraStateNot = raw.casterAuraStateNot;
                entry.targetAuraStateNot = raw.targetAuraStateNot;
                entry.areaGroupId = raw.areaGroupId;
                for (size_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
                    entry.effectMiscValueB[i] = raw.effectMiscValueB[i];
                for (size_t i = 0; i < MAX_SPELL_TOTEM_CATEGORIES; ++i)
                    entry.totemCategory[i] = raw.totemCategory[i];
            }

            if constexpr (requires { raw.casterAuraSpell; })
            {
                entry.casterAuraSpell = raw.casterAuraSpell;
                entry.targetAuraSpell = raw.targetAuraSpell;
                entry.casterAuraSpellNot = raw.casterAuraSpellNot;
                entry.targetAuraSpellNot = raw.targetAuraSpellNot;
                for (size_t eff = 0; eff < MAX_SPELL_EFFECTS; ++eff)
                {
                    entry.effectSpellPowerCoefficient[eff] = raw.effectSpellPowerCoefficient[eff];
                    for (size_t mask = 0; mask < 3; ++mask)
                        entry.effectSpellClassMask[eff][mask] = raw.effectSpellClassMask[eff][mask];
                }
            }

            if constexpr (requires { raw.runeCostId; })
                entry.runeCostId = raw.runeCostId;

            if constexpr (requires { raw.spellDifficultyId; })
                entry.spellDifficultyId = raw.spellDifficultyId;

            if constexpr (requires { raw.attackPowerCoefficient; })
                entry.attackPowerCoefficient = raw.attackPowerCoefficient;

            if constexpr (requires { raw.spellScalingId; })
            {
                entry.spellScalingId = raw.spellScalingId;
                entry.spellAuraOptionsId = raw.spellAuraOptionsId;
                entry.spellAuraRestrictionsId = raw.spellAuraRestrictionsId;
                entry.spellCastingRequirementsId = raw.spellCastingRequirementsId;
                entry.spellCategoriesId = raw.spellCategoriesId;
                entry.spellClassOptionsId = raw.spellClassOptionsId;
                entry.spellCooldownsId = raw.spellCooldownsId;
                entry.spellEquippedItemsId = raw.spellEquippedItemsId;
                entry.spellInterruptsId = raw.spellInterruptsId;
                entry.spellLevelsId = raw.spellLevelsId;
                entry.spellReagentsId = raw.spellReagentsId;
                entry.spellShapeshiftId = raw.spellShapeshiftId;
                entry.spellTargetRestrictionsId = raw.spellTargetRestrictionsId;
                entry.spellTotemsId = raw.spellTotemsId;
            }

            if constexpr (requires { raw.spellPowerId; })
                entry.spellPowerId = raw.spellPowerId;

            if constexpr (requires { raw.researchProject; })
                entry.researchProject = raw.researchProject;

            if constexpr (requires { raw.spellMiscId; })
                entry.spellMiscId = raw.spellMiscId;
    }
    );


    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellCastTimesStore, dbc_path, "SpellCastTimes.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellDurationStore, dbc_path, "SpellDuration.dbc");

    WDB::loadUnifiedWDBStore<WDB::Structures::SpellItemEnchantmentEntry>(
        bad_dbc_files, sSpellItemEnchantmentStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::SpellItemEnchantmentEntry& entry) {
            entry.id = raw.id;

            for (size_t i = 0; i < MAX_ITEM_ENCHANTMENT_EFFECTS; ++i)
            {
                entry.type[i] = raw.type[i];
                entry.min[i] = raw.min[i];
                entry.spell[i] = raw.spell[i];
            }

            if constexpr (requires { raw.max; })
            {
                for (size_t i = 0; i < MAX_ITEM_ENCHANTMENT_EFFECTS; ++i)
                    entry.max[i] = raw.max[i];
            }
            else
            {
                entry.max = entry.min;
            }

            if constexpr (requires { { raw.name[0] } -> std::convertible_to<char const*>; })
            {
                uint8_t localeId = sWorld.getDbcLocaleLanguageId();
                entry.name = (localeId < std::size(raw.name) && raw.name[localeId]) ? raw.name[localeId] : (raw.name[0] ? raw.name[0] : "");
            }
            else if constexpr (requires { raw.name; })
            {
                entry.name = raw.name ? raw.name : "";
            }

            entry.visual = raw.visual;
            entry.enchantGroups = raw.enchantGroups;

            if constexpr (requires { raw.gemEntry; })
                entry.gemEntry = raw.gemEntry;

            if constexpr (requires { raw.enchCondition; })
                entry.enchantCondition = raw.enchCondition;

            if constexpr (requires { raw.reqSkill; })
                entry.reqSkill = raw.reqSkill;

            if constexpr (requires { raw.reqSkillValue; })
                entry.reqSkillValue = raw.reqSkillValue;

            if constexpr (requires { raw.reqLevel; })
                entry.reqLevel = raw.reqLevel;
        }
    );

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellRadiusStore, dbc_path, "SpellRadius.dbc"); // todo handle max and level radius

    WDB::loadUnifiedWDBStore<WDB::Structures::SpellRangeEntry>(
        bad_dbc_files, sSpellRangeStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::SpellRangeEntry& entry) {
            entry.id = raw.id;
            entry.minRange = raw.minRange;
            entry.maxRange = raw.maxRange;
            entry.rangeType = raw.rangeType;

            if constexpr (requires { raw.minRangeFriendly; })
            {
                entry.minRangeFriendly = raw.minRangeFriendly;
                entry.maxRangeFriendly = raw.maxRangeFriendly;
            }
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::SpellShapeshiftFormEntry>(
        bad_dbc_files, sSpellShapeshiftFormStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::SpellShapeshiftFormEntry& entry) {
            entry.id = raw.id;
            entry.flags = raw.flags;
            entry.unitType = raw.unitType;

            if constexpr (requires { raw.attackSpeed; })
            {
                entry.attackSpeed = raw.attackSpeed;
                entry.modelId = raw.modelId;
                entry.modelId2 = raw.modelId2;
                for (size_t i = 0; i < entry.spells.size(); ++i)
                    entry.spells[i] = raw.spells[i];
            }
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::TalentEntry>(
        bad_dbc_files, sTalentStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::TalentEntry& entry) {
            entry.id = raw.id;
            entry.row = raw.row;
            entry.col = raw.col;

            if constexpr (requires { raw.talentTree; })
            {
                entry.talentTree = raw.talentTree;
                for (size_t i = 0; i < entry.rankId.size(); ++i)
                    entry.rankId[i] = raw.rankId[i];
                entry.dependsOn = raw.dependsOn;
                entry.dependsOnRank = raw.dependsOnRank;
            }

            if constexpr (requires { raw.spellId; })
            {
                entry.spellId = raw.spellId;
                entry.playerClass = raw.playerClass;
                entry.overrideSpellId = raw.overrideSpellId;
            }
        }
    );

#if VERSION_STRING < Mop
    // note: This is not valid for Mop
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTalentTabStore, dbc_path, "TalentTab.dbc");
    {
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
            if (talent_info->talentTree == 409 || talent_info->talentTree == 410 || talent_info->talentTree == 411)
                continue;

            auto talent_tab = sTalentTabStore.lookupEntry(talent_info->talentTree);
            if (talent_tab == nullptr)
                continue;

            talent_max_rank = 0;
            for (uint32_t j = 5; j > 0; --j)
            {
                if (talent_info->rankId[j - 1])
                {
                    talent_max_rank = j;
                    break;
                }
            }

            InspectTalentTabBit[(talent_info->row << 24) + (talent_info->col << 16) + talent_info->id] = talent_max_rank;
            InspectTalentTabSize[talent_info->talentTree] += talent_max_rank;
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

                if (talent_info->talentTree != talent_tab->TalentTabID)
                    continue;

                InspectTalentTabPos[talent_id] = talent_pos;
                talent_pos += itr->second;
            }
        }
    }
#endif

    WDB::loadUnifiedWDBStore<WDB::Structures::TaxiNodesEntry>(
        bad_dbc_files, sTaxiNodesStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::TaxiNodesEntry& entry) {
            entry.id = raw.id;
            entry.mapId = raw.mapId;
            entry.x = raw.x;
            entry.y = raw.y;
            entry.z = raw.z;

            if constexpr (requires { { raw.name[0] } -> std::convertible_to<char const*>; })
            {
                uint8_t localeId = sWorld.getDbcLocaleLanguageId();
                entry.name = (localeId < std::size(raw.name) && raw.name[localeId]) ? raw.name[localeId] : (raw.name[0] ? raw.name[0] : "");
            }
            else if constexpr (requires { raw.name; })
            {
                entry.name = raw.name ? raw.name : "";
            }

            entry.mountCreatureId[0] = raw.mountCreatureId[0];
            entry.mountCreatureId[1] = raw.mountCreatureId[1];

            if constexpr (requires { raw.flags; })
                entry.flags = raw.flags;
            else
                entry.flags = 0;
        });

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTaxiPathStore, dbc_path, "TaxiPath.dbc");

    WDB::loadUnifiedWDBStore<WDB::Structures::TaxiPathNodeEntry>(
        bad_dbc_files, sTaxiPathNodeStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::TaxiPathNodeEntry& entry) {
            entry.pathId = raw.pathId;
            entry.nodeIndex = raw.nodeIndex;
            entry.mapId = raw.mapId;
            entry.x = raw.x;
            entry.y = raw.y;
            entry.z = raw.z;
            entry.flags = raw.flags;
            entry.waitTime = raw.waitTime;

            if constexpr (requires { raw.arrivalEventId; })
            {
                entry.arrivalEventId = raw.arrivalEventId;
                entry.departureEventId = raw.departureEventId;
            }
        }
    );

    // note: Generate path data
    buildTaxiPathData();

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTransportAnimationStore, dbc_path, "TransportAnimation.dbc");

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sWMOAreaTableStore, dbc_path, "WMOAreaTable.dbc");
    buildWmoAreaTripples();

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sWorldMapOverlayStore, dbc_path, "WorldMapOverlay.dbc");

    /////////////////////////////////////////////////////////////////////////////////////////
    // Load single version specific dbcs
    WDB::loadUnifiedWDBStore<WDB::Structures::ItemDisplayInfoEntry>(
        bad_dbc_files, sItemDisplayInfoStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::ItemDisplayInfoEntry& entry) {
            entry.id = raw.id;
            entry.randomPropertyChance = raw.randomPropertyChance;
        }
    );

#ifdef AE_CATA
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemReforgeStore, dbc_path, "ItemReforge.dbc");
#endif

    WDB::loadUnifiedWDBStore<WDB::Structures::SpellMiscEntry>(
        bad_dbc_files, sSpellMiscStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::SpellMiscEntry& entry) {
            entry.id = raw.id;
            entry.attributes = raw.attributes[0];
            entry.attributesEx = raw.attributes[1];
            entry.attributesExB = raw.attributes[2];
            entry.attributesExC = raw.attributes[3];
            entry.attributesExD = raw.attributes[4];
            entry.attributesExE = raw.attributes[5];
            entry.attributesExF = raw.attributes[6];
            entry.attributesExG = raw.attributes[7];
            entry.attributesExH = raw.attributes[8];
            entry.attributesExI = raw.attributes[9];
            entry.attributesExJ = raw.attributes[10];
            entry.attributesExK = raw.attributes[11];
            entry.attributesExL = raw.attributes[12];
            entry.attributesExM = raw.attributes[13];
            entry.castingTimeIndex = raw.castingTimeIndex;
            entry.durationIndex = raw.durationIndex;
            entry.powerType = raw.powerType;
            entry.rangeIndex = raw.rangeIndex;
            entry.speed = raw.speed;
            entry.spellVisual = raw.spellVisual;
            entry.spellVisual1 = raw.spellVisual1;
            entry.spellIconId = raw.spellIconId;
            entry.activeIconId = raw.activeIconId;
            entry.school = raw.school;
        }
    );

#if VERSION_STRING >= Mop
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sChrSpecializationStore, dbc_path, "ChrSpecialization.dbc");
    buildClassSpecializationTabs();

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sBattlePetSpeciesStore, dbc_path, "BattlePetSpecies.db2");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sBattlePetBreedStateStore, dbc_path, "BattlePetBreedState.db2");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sBattlePetSpeciesStateStore, dbc_path, "BattlePetSpeciesState.db2");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sBattlePetBreedQualityStore, dbc_path, "BattlePetBreedQuality.db2");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sItemToBattlePetStore, dbc_path, "ItemToBattlePet.db2");
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

    WDB::loadUnifiedWDBStore<WDB::Structures::ItemEntry>(
        bad_dbc_files, sItemStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::ItemEntry& entry) {
            entry.id = raw.id;
            entry.displayId = raw.displayId;
            entry.inventoryType = raw.inventoryType;
            entry.sheath = raw.sheath;

            if constexpr (requires { raw.itemClass; }) {
                entry.itemClass = raw.itemClass;
                entry.itemSubClass = raw.itemSubClass;
                entry.soundOverrideSubclass = raw.soundOverrideSubclass;
                entry.material = raw.material;
            }
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
#endif

    #if VERSION_STRING < Mop
        WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGtOCTRegenMPStore, dbc_path, "gtOCTRegenMP.dbc");
    #endif
#endif

    WDB::loadUnifiedWDBStore<WDB::Structures::ItemExtendedCostEntry>(
        bad_dbc_files, sItemExtendedCostStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::ItemExtendedCostEntry& entry) {
            entry.id = raw.id;
            entry.honorPoints = raw.honorPoints;
            entry.arenaPoints = raw.arenaPoints;
            entry.personalRating = raw.personalRating;

            for (size_t i = 0; i < 5; ++i)
            {
                entry.item[i] = raw.item[i];
                entry.count[i] = raw.count[i];
            }

            if constexpr (requires { raw.arenaSlot; })
                entry.arenaSlot = raw.arenaSlot;

            if constexpr (requires { raw.reqCur; })
            {
                for (size_t i = 0; i < 5; ++i)
                {
                    entry.reqCur[i] = raw.reqCur[i];
                    entry.reqCurrCount[i] = raw.reqCurrCount[i];
                }
            }
        }
    );

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
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sDungeonEncounterStore, dbc_path, "DungeonEncounter.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sTransportRotationStore, dbc_path, "TransportRotation.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGlyphPropertiesStore, dbc_path, "GlyphProperties.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sGlyphSlotStore, dbc_path, "GlyphSlot.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sBarberShopCostBaseStore, dbc_path, "gtBarberShopCostBase.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sHolidaysStore, dbc_path, "Holidays.dbc");
    
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

    WDB::loadUnifiedWDBStore<WDB::Structures::ItemLimitCategoryEntry>(
        bad_dbc_files, sItemLimitCategoryStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::ItemLimitCategoryEntry& entry) {
            entry.id = raw.id;
            entry.maxAmount = raw.maxAmount;
            entry.equippedFlag = raw.equippedFlag;
        }
    );

    /////////////////////////////////////////////////////////////////////////////////////////
    // Load multi version specific dbcs available since WotLK

    WDB::loadUnifiedWDBStore<WDB::Structures::BannedAddOnsEntry>(
        bad_dbc_files, sBannedAddOnsStore, dbc_path,
        []<typename RawType>(RawType const& raw, WDB::Structures::BannedAddOnsEntry& entry)
        {
            entry.id = raw.id;
        }
    );

    WDB::loadUnifiedWDBStore<WDB::Structures::SpellPowerEntry>(
        bad_dbc_files, sSpellPowerStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::SpellPowerEntry& entry) {
            entry.manaCost = raw.manaCost;
            entry.manaCostPerLevel = raw.manaCostPerLevel;
            entry.manaPerSecond = raw.manaPerSecond;
            entry.manaPerSecondPerLevel = raw.manaPerSecondPerLevel;
            entry.manaCostPercentageFloat = raw.manaCostPercentageFloat;

            if constexpr (requires { raw.manaCostPercentage; })
                entry.manaCostPercentage = raw.manaCostPercentage;

            if constexpr (requires { raw.spellId; })
            {
                entry.spellId = raw.spellId;
                entry.powerType = raw.powerType;
                entry.channelCostPercentageFloat = raw.channelCostPercentageFloat;
                entry.shapeShiftSpellId = raw.shapeShiftSpellId;
            }
        }
    );

    buildSpellPowerMap();

    WDB::loadUnifiedWDBStore<WDB::Structures::SpellAuraOptionsEntry>(
        bad_dbc_files, sSpellAuraOptionsStore, dbc_path,
        []<typename RawType>(const RawType& raw, WDB::Structures::SpellAuraOptionsEntry& entry) {
        if constexpr (requires { raw.id; })
            entry.id = raw.id;

        entry.maxStackAmount = raw.maxStackAmount;
        entry.procChance = raw.procChance;
        entry.procCharges = raw.procCharges;
        entry.procFlags = raw.procFlags;
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

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellAuraRestrictionsStore, dbc_path, "SpellAuraRestrictions.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellCastingRequirementsStore, dbc_path, "SpellCastingRequirements.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellCategoriesStore, dbc_path, "SpellCategories.dbc");
    // note: generate spell categories and insert to helper storage
    buildSpellCategoryStore();

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellClassOptionsStore, dbc_path, "SpellClassOptions.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellCooldownsStore, dbc_path, "SpellCooldowns.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellEffectStore, dbc_path, "SpellEffect.dbc");
    // note: generate spell effects and add to helper map
    buildSpellEffectMap();

    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellEquippedItemsStore, dbc_path, "SpellEquippedItems.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellInterruptsStore, dbc_path, "SpellInterrupts.dbc");
    WDB::loadWDBFile(available_dbc_locales, bad_dbc_files, sSpellLevelsStore, dbc_path, "SpellLevels.dbc");

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

WDB::Structures::SpellPowerEntry const* getSpellPowerEntry(uint32_t spellId)
{
    WDB::Structures::SpellPowerMap::const_iterator itr = sSpellPowerMap.find(spellId);
    if (itr == sSpellPowerMap.end())
        return nullptr;

    return itr->second;
}

#if VERSION_STRING >= WotLK
WDB::Structures::MapDifficulty const* getDownscaledMapDifficultyData(uint32_t mapId, InstanceDifficulty::Difficulties& difficulty)
{
    // 5.x: difficulty ids, the fallbacks are followed until the map knows one
    if (InstanceDifficulty::usesDifficultyIds())
    {
        uint8_t tryDiff = difficulty;
        for (uint8_t step = 0; tryDiff != 0 && step < 8; ++step, tryDiff = InstanceDifficulty::fallbackDifficulty(tryDiff))
        {
            if (WDB::Structures::MapDifficulty const* mapDiff = getMapDifficultyData(mapId, InstanceDifficulty::Difficulties(tryDiff)))
            {
                difficulty = InstanceDifficulty::Difficulties(tryDiff);
                return mapDiff;
            }
        }
        return nullptr;
    }

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

uint32_t getTalentInspectBitPosInTab(uint32_t talentId)
{
    const auto itr = InspectTalentTabPos.find(talentId);
    return itr != InspectTalentTabPos.end() ? itr->second : 0;
}

uint32_t getTalentTabInspectBitSize(uint32_t talentTabId)
{
    const auto itr = InspectTalentTabSize.find(talentTabId);
    return itr != InspectTalentTabSize.end() ? itr->second : 0;
}

#if VERSION_STRING >= Mop
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
