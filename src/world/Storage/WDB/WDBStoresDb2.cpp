/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Fills the runtime stores of WDBStores.hpp from the DB2 tables of a client
// that ships its own data (WoW::hasOwnClientData). The stores and their
// structures are the ones the rest of the core already uses, only the source
// of the rows differs: every table is read with WDC5File and the columns are
// mapped by the schema of the client version (WDC5Schema.hpp). Tables the
// modern clients no longer have (the gt* tables, ScalingStat*, SpellRuneCost,
// TalentTab, ...) stay empty; their consumers handle a missing row.

#include "AEVersion.hpp"

#if VERSION_STRING >= WoD

#include "WDBStores.hpp"

#include "WDBContainer.hpp"
#include "WDBGlobals.hpp"
#include "WDBStructures.hpp"
#include "WDC5File.hpp"
#include "WDC5Schema.hpp"
#include "Logging/Logger.hpp"
#include "Management/AchievementMgr.h"
#include "Map/Area/AreaStorage.hpp"

#include <algorithm>
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <iterator>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
    using WDB::WDC5File;
    using WDB::WDC5TableSchema;

    // the client whose column layout is loaded (see WoW::hasOwnClientData)
    namespace Layout = WDB::Formats::MidnightClient;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Access helpers

    // The DB2 strings are copied out of the files, the entry structures keep
    // char pointers like the dbc loader's string pool.
    std::deque<std::string> gStringPool;

    char* keepString(std::string_view value)
    {
        static char empty[1] = { '\0' };
        if (value.empty())
            return empty;

        return gStringPool.emplace_back(value).data();
    }

    // One record of a loaded table. The accessors name the column type of the
    // schema; narrower columns widen implicitly where the entry field is wider.
    class Row
    {
    public:
        Row(WDC5File const& file, uint32_t index) noexcept : m_file(&file), m_index(index) {}

        [[nodiscard]] uint32_t id() const { return m_file->getRecordId(m_index); }
        [[nodiscard]] uint32_t parent() const noexcept { return m_file->getParentId(m_index); }

        // A column that is also the relation parent of its table is stored in the
        // relationship data of the section, the record itself then holds 0 for it.
        [[nodiscard]] uint32_t parentOr(uint32_t field) const
        {
            uint32_t const parentId = parent();
            return parentId != 0 ? parentId : u32(field);
        }

        [[nodiscard]] uint8_t u8(uint32_t field, uint32_t index = 0) const { return m_file->getUInt8(m_index, field, index); }
        [[nodiscard]] int8_t i8(uint32_t field, uint32_t index = 0) const { return m_file->getInt8(m_index, field, index); }
        [[nodiscard]] uint16_t u16(uint32_t field, uint32_t index = 0) const { return m_file->getUInt16(m_index, field, index); }
        [[nodiscard]] int16_t i16(uint32_t field, uint32_t index = 0) const { return m_file->getInt16(m_index, field, index); }
        [[nodiscard]] uint32_t u32(uint32_t field, uint32_t index = 0) const { return m_file->getUInt32(m_index, field, index); }
        [[nodiscard]] int32_t i32(uint32_t field, uint32_t index = 0) const { return m_file->getInt32(m_index, field, index); }
        [[nodiscard]] float f32(uint32_t field, uint32_t index = 0) const { return m_file->getFloat(m_index, field, index); }
        [[nodiscard]] std::string_view str(uint32_t field) const { return m_file->getString(m_index, field); }

        // a copy of a string column that outlives the file (the entries keep char pointers)
        [[nodiscard]] char* text(uint32_t field) const { return keepString(str(field)); }

    private:
        WDC5File const* m_file;
        uint32_t m_index;
    };

    [[nodiscard]] auto rows(WDC5File const& file)
    {
        return std::views::iota(uint32_t{ 0 }, file.getRecordCount())
            | std::views::transform([&file](uint32_t index) { return Row(file, index); });
    }

    [[nodiscard]] uint32_t maxRecordId(WDC5File const& file)
    {
        uint32_t maxId = 0;
        for (Row const row : rows(file))
            maxId = std::max(maxId, row.id());
        return maxId;
    }

    [[nodiscard]] constexpr uint64_t makeUInt64(uint32_t low, uint32_t high) noexcept
    {
        return static_cast<uint64_t>(low) | (static_cast<uint64_t>(high) << 32);
    }

    struct TableLoad
    {
        WDC5File& file;
        WDC5TableSchema const& schema;
    };

    // loads every table of the group, a missing or unreadable one is an error of the whole group
    bool loadTables(WDB::StoreProblemList& errors, std::string const& dbcPath, std::initializer_list<TableLoad> tables)
    {
        bool ok = true;
        for (auto const& [file, schema] : tables)
        {
            std::string error;
            if (file.load(dbcPath + schema.filename, schema, &error))
            {
                sLogger.debug("DB2: {} loaded ({} records, layout 0x{:08X}).", schema.filename, file.getRecordCount(), file.getLayoutHash());
                continue;
            }

            errors.push_back(std::string(schema.filename) + ": " + error);
            ok = false;
        }
        return ok;
    }

    // Fills an id-indexed container: one entry per record, keyed by the record id.
    template <typename T, typename Fill>
    void fillContainer(WDB::WDBContainer<T>& store, WDC5File const& file, Fill&& fill)
    {
        store.reserveManualEntries(maxRecordId(file), file.getRecordCount());
        for (Row const row : rows(file))
        {
            if (T* entry = store.addManualEntry(row.id()))
                fill(*entry, row);
        }
    }

    // Fills a map store: one entry per record, keyed by the record id.
    template <typename T, typename Fill>
    void fillStore(WDB::WDBStore<T>& store, WDC5File const& file, Fill&& fill)
    {
        store.clear();
        store.reserve(file.getRecordCount());
        for (Row const row : rows(file))
        {
            T entry{};
            fill(entry, row);
            store.insert_or_assign(row.id(), std::move(entry));
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Characters, classes, races, factions

    bool loadCharacterStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDC5File chrClasses, chrClassesXPowerTypes, chrRaces, chrModel, chrRaceXChrModel, chrSpecialization, faction, factionTemplate;
        WDC5File charTitles, characterLoadout, characterLoadoutItem, nameGen;
        if (!loadTables(errors, dbcPath, {
            { chrClasses, Layout::ChrClasses }, { chrClassesXPowerTypes, Layout::ChrClassesXPowerTypes }, { chrRaces, Layout::ChrRaces },
            { chrModel, Layout::ChrModel }, { chrRaceXChrModel, Layout::ChrRaceXChrModel }, { chrSpecialization, Layout::ChrSpecialization },
            { faction, Layout::Faction }, { factionTemplate, Layout::FactionTemplate }, { charTitles, Layout::CharTitles },
            { characterLoadout, Layout::CharacterLoadout }, { characterLoadoutItem, Layout::CharacterLoadoutItem }, { nameGen, Layout::NameGen } }))
            return false;

        fillStore(sChrClassesStore, chrClasses, [](WDB::Structures::ChrClassesEntry& entry, Row const& row)
        {
            entry.classId = row.id();
            entry.powerType = row.i8(32);              // DisplayPower
            entry.name = row.str(0);
            entry.spellClassSet = row.u8(36);
            entry.cinematicSequenceId = row.u16(27);
            entry.apPerStr = row.u8(35);
            entry.apPerAgi = row.u8(34);
            entry.rapPerAgi = row.u8(33);
        });

        fillStore(sChrPowerTypesStore, chrClassesXPowerTypes, [](WDB::Structures::ChrPowerTypesEntry& entry, Row const& row)
        {
            entry.entry = row.id();
            entry.classId = row.parent();
            entry.power = row.u8(0);
        });

        // the race models come from the ChrRaceXChrModel relation, male = sex 0, female = sex 1
        std::unordered_map<uint32_t, uint32_t> displayIdByModel;
        for (Row const row : rows(chrModel))
            displayIdByModel[row.id()] = row.parentOr(4);       // DisplayID

        std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> modelsByRace;
        for (Row const row : rows(chrRaceXChrModel))
        {
            auto const display = displayIdByModel.find(row.i32(1));
            if (display == displayIdByModel.end())
                continue;

            auto& [male, female] = modelsByRace[row.parentOr(0)];
            switch (row.i8(2))
            {
                case 0: male = display->second; break;
                case 1: female = display->second; break;
                default: break;
            }
        }

        fillStore(sChrRacesStore, chrRaces, [&modelsByRace](WDB::Structures::ChrRacesEntry& entry, Row const& row)
        {
            entry.raceId = row.id();
            entry.flags = row.i32(15);
            entry.factionId = row.i32(16);
            entry.cinematicId = row.i32(17);
            entry.name = row.str(2);

            // Alliance column: 0 alliance, 1 horde, 2 neutral. The runtime keeps the
            // dbc convention of team id 7 for the alliance races.
            switch (row.i8(36))
            {
                case 0: entry.teamId = 7; break;
                case 1: entry.teamId = 1; break;
                default: entry.teamId = 0; break;
            }

            if (auto const models = modelsByRace.find(entry.raceId); models != modelsByRace.end())
                std::tie(entry.modelMale, entry.modelFemale) = models->second;
        });

        fillContainer(sChrSpecializationStore, chrSpecialization, [](WDB::Structures::ChrSpecializationEntry& entry, Row const& row)
        {
            entry.Id = row.id();
            entry.classId = row.parentOr(4);
            entry.masterySpellId = row.i32(12, 0);
            entry.tabPage = row.i8(5);          // OrderIndex
            entry.petTabPage = row.i8(6);       // PetTalentType
        });

        // The four reputation relation sets are array columns, the race masks are
        // 64 bit of which the runtime consumes the low half.
        fillStore(sFactionStore, faction, [](WDB::Structures::FactionEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.reputationIndex = row.i16(2);
            entry.parentFactionId = row.u16(3);
            entry.expansion = row.u8(4);
            for (uint32_t i = 0; i < 4; ++i)
            {
                entry.reputationClassMask[i] = row.i16(10, i);
                entry.reputationFlags[i] = row.u16(11, i);
                entry.reputationBase[i] = row.i32(12, i);
                entry.reputationRaceMask[i] = row.i32(16 + i, 0);
            }
            entry.spilloverRateIn = row.f32(14, 0);
            entry.spilloverRateOut = row.f32(14, 1);
            entry.spilloverMaxIn = row.u8(15, 0);
            entry.name = row.str(0);
        });

        // eight friend and enemy relations in the client, the runtime keeps four
        fillStore(sFactionTemplateStore, factionTemplate, [](WDB::Structures::FactionTemplateEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.faction = row.u16(0);
            entry.factionFlags = row.i32(1);
            entry.ourMask = row.u8(2);
            entry.friendlyMask = row.u8(3);
            entry.hostileMask = row.u8(4);
            for (uint32_t i = 0; i < WDB::Structures::maxFactionRelations; ++i)
            {
                entry.enemyFaction[i] = row.u16(5, i);
                entry.friendFaction[i] = row.u16(6, i);
            }
        });

        fillStore(sCharTitlesStore, charTitles, [](WDB::Structures::CharTitlesEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.nameMale = row.str(0);
            entry.nameFemale = row.str(1);
            entry.bitIndex = row.i16(2);        // MaskID
        });

        // The starting gear is the CharacterLoadout with purpose 9 of a class and a
        // race mask; one legacy outfit row per race and gender of that mask.
        constexpr int32_t kLoadoutPurposeNewCharacter = 9;
        std::unordered_map<uint32_t, std::vector<int32_t>> itemsByLoadout;
        for (Row const row : rows(characterLoadoutItem))
            itemsByLoadout[row.parentOr(0)].push_back(row.u32(1));

        sCharStartOutfitStore.clear();
        for (Row const row : rows(characterLoadout))
        {
            if (row.i32(1) != kLoadoutPurposeNewCharacter)
                continue;

            auto const items = itemsByLoadout.find(row.id());
            if (items == itemsByLoadout.end())
                continue;

            uint8_t const classId = row.i8(0);
            uint64_t const raceMask = makeUInt64(row.i32(3, 0), row.i32(3, 1));
            for (uint32_t raceBit = 0; raceBit < 64; ++raceBit)
            {
                if ((raceMask & (uint64_t{ 1 } << raceBit)) == 0)
                    continue;

                for (uint8_t gender = 0; gender < 2; ++gender)
                {
                    WDB::Structures::CharStartOutfitEntry entry;
                    entry.race = static_cast<uint8_t>(raceBit + 1);
                    entry.classId = classId;
                    entry.gender = gender;
                    std::ranges::fill(entry.itemId, -1);
                    std::copy_n(items->second.begin(), std::min(items->second.size(), std::size(entry.itemId)), std::begin(entry.itemId));
                    sCharStartOutfitStore.try_emplace(entry.makeKey(), std::move(entry));
                }
            }
        }

        fillContainer(sNameGenStore, nameGen, [](WDB::Structures::NameGenEntry& entry, Row const& row)
        {
            entry.ID = row.id();
            entry.Name = row.text(0);
            entry.unk1 = row.i8(1);     // RaceID
            entry.type = row.i8(2);     // Sex
        });

        sLogger.info("DB2: character stores ready (classes={}, races={}, factions={}, factionTemplates={}, specializations={}, startOutfits={}).",
            sChrClassesStore.getNumRows(), sChrRacesStore.getNumRows(), sFactionStore.getNumRows(), sFactionTemplateStore.getNumRows(),
            chrSpecialization.getRecordCount(), sCharStartOutfitStore.getNumRows());
        return true;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Creatures, vehicles

    bool loadCreatureStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDC5File displayInfo, displayInfoExtra, modelData, creatureFamily, summonProperties, vehicle, vehicleSeat;
        if (!loadTables(errors, dbcPath, {
            { displayInfo, Layout::CreatureDisplayInfo }, { displayInfoExtra, Layout::CreatureDisplayInfoExtra }, { modelData, Layout::CreatureModelData },
            { creatureFamily, Layout::CreatureFamily }, { summonProperties, Layout::SummonProperties }, { vehicle, Layout::Vehicle }, { vehicleSeat, Layout::VehicleSeat } }))
            return false;

        fillStore(sCreatureDisplayInfoStore, displayInfo, [](WDB::Structures::CreatureDisplayInfoEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.modelId = row.u16(1);
            entry.extendedDisplayInfoId = row.i32(7);
            entry.creatureModelScale = row.f32(4);
        });

        fillStore(sCreatureDisplayInfoExtraStore, displayInfoExtra, [](WDB::Structures::CreatureDisplayInfoExtraEntry& entry, Row const& row)
        {
            entry.displayExtraId = row.id();
            entry.race = row.i8(1);
            entry.displaySexId = row.i8(2);
        });

        fillStore(sCreatureModelDataStore, modelData, [](WDB::Structures::CreatureModelDataEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.flags = row.i32(1);
            // the model is a file data id in the modern tables, the name stays empty
            entry.collisionHeight = row.f32(16);
            entry.modelScale = row.f32(21);
            entry.mountHeight = row.f32(25);
        });

        fillStore(sCreatureFamilyStore, creatureFamily, [](WDB::Structures::CreatureFamilyEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.minSize = row.f32(1);
            entry.minLevel = row.i8(2);
            entry.maxSize = row.f32(3);
            entry.maxLevel = row.i8(4);
            entry.skillLine = row.i16(9, 0);
            entry.tameable = row.i16(9, 1);         // the second skill line, as in the dbc layout
            entry.petDietFlags = row.i16(5);        // PetFoodMask
            entry.talentTree = row.i8(6);           // PetTalentType
            entry.name = row.str(0);
        });

        fillContainer(sSummonPropertiesStore, summonProperties, [](WDB::Structures::SummonPropertiesEntry& entry, Row const& row)
        {
            entry.ID = row.id();
            entry.ControlType = row.i32(0);
            entry.FactionID = row.i32(1);
            entry.Type = row.i32(2);       // Title
            entry.Slot = row.i32(3);
            entry.Flags = row.i32(4, 0);
        });

        fillContainer(sVehicleStore, vehicle, [](WDB::Structures::VehicleEntry& entry, Row const& row)
        {
            entry.ID = row.id();
            entry.flags = row.i32(0);
            entry.turnSpeed = row.f32(2);
            entry.pitchSpeed = row.f32(3);
            entry.pitchMin = row.f32(4);
            entry.pitchMax = row.f32(5);
            for (uint32_t i = 0; i < MAX_VEHICLE_SEATS; ++i)
                entry.seatID[i] = row.u16(16, i);
            entry.mouseLookOffsetPitch = row.f32(6);
            entry.cameraFadeDistScalarMin = row.f32(7);
            entry.cameraFadeDistScalarMax = row.f32(8);
            entry.cameraPitchOffset = row.f32(9);
            entry.facingLimitRight = row.f32(10);
            entry.facingLimitLeft = row.f32(11);
            entry.msslTrgtArcTexture = keepString({});
            entry.msslTrgtImpactTexture = keepString({});
            entry.msslTrgtImpactModel[0] = keepString({});
            entry.msslTrgtImpactModel[1] = keepString({});
            entry.cameraYawOffset = row.f32(12);
            entry.uiSeatIndicatorType = row.u16(13);
            entry.powerType = row.u16(17, 0);       // PowerDisplayID
        });

        fillContainer(sVehicleSeatStore, vehicleSeat, [](WDB::Structures::VehicleSeatEntry& entry, Row const& row)
        {
            entry.ID = row.id();
            entry.flags = row.i32(2);
            entry.attachmentID = row.i32(5);
            entry.attachmentOffsetX = row.f32(0, 0);
            entry.attachmentOffsetY = row.f32(0, 1);
            entry.attachmentOffsetZ = row.f32(0, 2);
            entry.enterPreDelay = row.f32(6);
            entry.enterSpeed = row.f32(7);
            entry.enterGravity = row.f32(8);
            entry.enterMinDuration = row.f32(9);
            entry.enterMaxDuration = row.f32(10);
            entry.enterMinArcHeight = row.f32(11);
            entry.enterMaxArcHeight = row.f32(12);
            entry.enterAnimStart = row.i16(13);
            entry.enterAnimLoop = row.i16(14);
            entry.rideAnimStart = row.i16(15);
            entry.rideAnimLoop = row.i16(16);
            entry.rideUpperAnimStart = row.i16(17);
            entry.rideUpperAnimLoop = row.i16(18);
            entry.exitPreDelay = row.f32(19);
            entry.exitSpeed = row.f32(20);
            entry.exitGravity = row.f32(21);
            entry.exitMinDuration = row.f32(22);
            entry.exitMaxDuration = row.f32(23);
            entry.exitMinArcHeight = row.f32(24);
            entry.exitMaxArcHeight = row.f32(25);
            entry.exitAnimStart = row.i16(26);
            entry.exitAnimLoop = row.i16(27);
            entry.exitAnimEnd = row.i16(28);
            entry.passengerYaw = row.f32(36);
            entry.passengerPitch = row.f32(37);
            entry.passengerRoll = row.f32(38);
            entry.passengerAttachmentID = row.i8(35);
            entry.vehicleEnterAnim = row.i16(29);
            entry.vehicleExitAnim = row.i16(31);
            entry.vehicleRideAnimLoop = row.i16(33);
            entry.vehicleEnterAnimBone = row.i8(30);
            entry.vehicleExitAnimBone = row.i8(32);
            entry.vehicleRideAnimLoopBone = row.i8(34);
            entry.vehicleEnterAnimDelay = row.f32(39);
            entry.vehicleExitAnimDelay = row.f32(40);
            entry.vehicleAbilityDisplay = row.i8(41);
            entry.enterUISoundID = row.u32(42);
            entry.exitUISoundID = row.u32(43);
            entry.uiSkin = row.i32(44);             // UiSkinFileDataID
            entry.flagsB = row.i32(3);
        });

        sLogger.info("DB2: creature stores ready (displays={}, models={}, families={}, vehicles={}, seats={}).",
            sCreatureDisplayInfoStore.getNumRows(), sCreatureModelDataStore.getNumRows(), sCreatureFamilyStore.getNumRows(),
            vehicle.getRecordCount(), vehicleSeat.getRecordCount());
        return true;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Maps, areas, taxi, transports

    bool loadWorldStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDC5File map, mapDifficulty, uiMapAssignment, worldMapOverlay, areaTable, areaTrigger, areaGroupMember, liquidType, wmoAreaTable;
        WDC5File dungeonEncounter, lfgDungeons, phase, transportAnimation, transportRotation, taxiNodes, taxiPath, taxiPathNode, holidays;
        if (!loadTables(errors, dbcPath, {
            { map, Layout::Map }, { mapDifficulty, Layout::MapDifficulty }, { uiMapAssignment, Layout::UiMapAssignment },
            { worldMapOverlay, Layout::WorldMapOverlay }, { areaTable, Layout::AreaTable }, { areaTrigger, Layout::AreaTrigger },
            { areaGroupMember, Layout::AreaGroupMember }, { liquidType, Layout::LiquidType }, { wmoAreaTable, Layout::WMOAreaTable },
            { dungeonEncounter, Layout::DungeonEncounter }, { lfgDungeons, Layout::LFGDungeons }, { phase, Layout::Phase },
            { transportAnimation, Layout::TransportAnimation }, { transportRotation, Layout::TransportRotation },
            { taxiNodes, Layout::TaxiNodes }, { taxiPath, Layout::TaxiPath }, { taxiPathNode, Layout::TaxiPathNode }, { holidays, Layout::Holidays } }))
            return false;

        fillStore(sMapStore, map, [](WDB::Structures::MapEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.mapType = row.i8(8);              // InstanceType, same 0..4 semantics
            entry.mapName = row.str(1);
            entry.linkedZone = row.u16(10);         // AreaTableID
            entry.multimapId = row.i16(11);         // LoadingScreenID
            entry.parentMap = row.i16(17);          // CorpseMapID
            entry.startX = row.f32(6, 0);
            entry.startY = row.f32(6, 1);
            entry.addon = row.u8(9);                // ExpansionID
            entry.maxPlayers = row.u8(18);
            entry.nextPhaseMap = row.i16(13);       // ParentMapID
        });

        fillStore(sMapDifficultyStore, mapDifficulty, [](WDB::Structures::MapDifficultyEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.mapId = row.parent();
            entry.difficulty = static_cast<uint16_t>(row.i16(2));
            entry.message = row.str(0);
            switch (row.u8(4))                      // ResetInterval
            {
                case 1: entry.raidDuration = 86400; break;
                case 2: entry.raidDuration = 604800; break;
                default: entry.raidDuration = 0; break;
            }
            entry.maxPlayers = std::max(row.i32(5), 0);
        });

        // WorldMapArea no longer exists, UiMapAssignment relates the areas to their maps
        sWorldMapAreaStore.clear();
        for (Row const row : rows(uiMapAssignment))
        {
            int32_t const mapId = row.i32(6);
            int32_t const areaId = row.i32(7);
            if (mapId < 0 || areaId <= 0)
                continue;

            WDB::Structures::WorldMapAreaEntry entry;
            entry.mapId = mapId;
            entry.zoneId = areaId;
            entry.continentMapId = mapId;
            sWorldMapAreaStore.try_emplace(entry.zoneId, entry);
        }

        fillContainer(sWorldMapOverlayStore, worldMapOverlay, [](WDB::Structures::WorldMapOverlayEntry& entry, Row const& row)
        {
            entry.ID = row.id();
            entry.areaID = row.u32(12, 0);
            entry.areaID_2 = row.u32(12, 1);
            entry.areaID_3 = row.u32(12, 2);
            entry.areaID_4 = row.u32(12, 3);
        });

        fillStore(sAreaStore, areaTable, [](WDB::Structures::AreaTableEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.map_id = row.u16(2);              // ContinentID
            entry.zone = row.u16(3);                // ParentAreaID
            entry.explore_flag = row.i16(4);        // AreaBit
            entry.flags = row.i32(21, 0);
            // the level range moved to ContentTuning, area_level stays 0
            entry.area_name = row.str(1);
            entry.team = row.u8(13);                // FactionGroupMask
            for (uint32_t i = 0; i < 4; ++i)
                entry.liquid_type_override[i] = row.u16(22, i);
        });
        MapManagement::AreaManagement::AreaStorage::initialise(&sAreaStore);

        fillStore(sAreaTriggerStore, areaTrigger, [](WDB::Structures::AreaTriggerEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.mapId = row.parentOr(2);          // ContinentID
            entry.x = row.f32(0, 0);
            entry.y = row.f32(0, 1);
            entry.z = row.f32(0, 2);
            entry.boxRadius = row.f32(6);
            entry.boxX = row.f32(7);                // BoxLength
            entry.boxY = row.f32(8);                // BoxWidth
            entry.boxZ = row.f32(9);                // BoxHeight
            entry.boxOrientation = row.f32(10);     // BoxYaw
        });

        // AreaGroup became a member relation; groups with more than six areas are
        // chained through synthetic group ids like the dbc did with nextGroup.
        {
            std::map<uint32_t, std::vector<uint32_t>> areasByGroup;
            for (Row const row : rows(areaGroupMember))
                areasByGroup[row.parent()].push_back(row.u16(0));

            constexpr size_t kAreasPerGroup = std::extent_v<decltype(WDB::Structures::AreaGroupEntry::areaId)>;
            uint32_t syntheticGroupId = 1000000;
            sAreaGroupStore.clear();
            for (auto const& [groupId, areas] : areasByGroup)
            {
                uint32_t currentId = groupId;
                for (size_t offset = 0; offset < areas.size(); offset += kAreasPerGroup)
                {
                    WDB::Structures::AreaGroupEntry entry;
                    entry.id = currentId;
                    std::copy_n(areas.begin() + offset, std::min(kAreasPerGroup, areas.size() - offset), std::begin(entry.areaId));
                    entry.nextGroup = offset + kAreasPerGroup < areas.size() ? syntheticGroupId : 0;
                    sAreaGroupStore[entry.id] = entry;
                    if (entry.nextGroup != 0)
                        currentId = syntheticGroupId++;
                }
            }
        }

        fillContainer(sLiquidTypeStore, liquidType, [](WDB::Structures::LiquidTypeEntry& entry, Row const& row)
        {
            entry.Id = row.id();
            entry.Type = row.u8(3);         // SoundBank, the liquid category of the map flags
            entry.SpellId = row.u32(5);
        });

        fillContainer(sWMOAreaTableStore, wmoAreaTable, [](WDB::Structures::WMOAreaTableEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.rootId = row.parentOr(2);     // WmoID
            entry.adtId = row.u8(3);            // NameSetID
            entry.groupId = row.i32(4);         // WmoGroupID
            entry.flags = row.i32(14);
            entry.areaId = row.u16(13);         // AreaTableID
        });

        fillContainer(sDungeonEncounterStore, dungeonEncounter, [](WDB::Structures::DungeonEncounterEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.mapId = row.parentOr(2);
            entry.difficulty = row.i16(3);
            entry.encounterIndex = row.i32(4);  // OrderIndex
            entry.encounterName[0] = row.text(0);
        });

        // the level requirements of the dungeons moved to ContentTuning, they stay 0 here
        fillContainer(sLFGDungeonStore, lfgDungeons, [](WDB::Structures::LFGDungeonEntry& entry, Row const& row)
        {
            entry.ID = row.id();
            entry.name[0] = row.text(0);
            entry.map = row.i16(9);
            entry.difficulty = row.i16(10);
            entry.flags = row.i32(32, 0);
            entry.type = row.u8(2);             // TypeID
            entry.expansion = row.u8(8);        // ExpansionLevel
            entry.grouptype = row.u8(12);       // GroupID
        });

        fillContainer(sPhaseStore, phase, [](WDB::Structures::PhaseEntry& entry, Row const& row)
        {
            entry.Id = row.id();
            entry.Flags = row.i32(0);
        });

        fillContainer(sTransportAnimationStore, transportAnimation, [](WDB::Structures::TransportAnimationEntry& entry, Row const& row)
        {
            entry.TransportID = row.parent();
            entry.TimeIndex = row.u32(2);
            entry.Pos = { row.f32(0, 0), row.f32(0, 1), row.f32(0, 2) };
        });

        fillContainer(sTransportRotationStore, transportRotation, [](WDB::Structures::TransportRotationEntry& entry, Row const& row)
        {
            entry.GameObjectsID = row.parent();
            entry.TimeIndex = row.u32(1);
            entry.X = row.f32(0, 0);
            entry.Y = row.f32(0, 1);
            entry.Z = row.f32(0, 2);
            entry.W = row.f32(0, 3);
        });

        fillContainer(sTaxiNodesStore, taxiNodes, [](WDB::Structures::TaxiNodesEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.mapid = row.parentOr(5);      // ContinentID
            entry.x = row.f32(1, 0);
            entry.y = row.f32(1, 1);
            entry.z = row.f32(1, 2);
            entry.name[0] = row.text(0);
            entry.mountCreatureID[0] = row.i32(14, 0);
            entry.mountCreatureID[1] = row.i32(14, 1);
            entry.flags = row.i32(8);
        });

        fillContainer(sTaxiPathStore, taxiPath, [](WDB::Structures::TaxiPathEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.from = row.parentOr(1);       // FromTaxiNode
            entry.to = row.u16(2);
            entry.price = row.u32(3);           // Cost
        });

        fillContainer(sTaxiPathNodeStore, taxiPathNode, [](WDB::Structures::TaxiPathNodeEntry& entry, Row const& row)
        {
            entry.pathId = row.parentOr(2);
            entry.NodeIndex = row.i32(3);
            entry.mapid = row.u16(4);           // ContinentID
            entry.x = row.f32(0, 0);
            entry.y = row.f32(0, 1);
            entry.z = row.f32(0, 2);
            entry.flags = row.i32(5);
            entry.waittime = row.u32(6);        // Delay
            entry.arivalEventID = row.i32(7);
            entry.departureEventID = row.i32(8);
        });

        fillContainer(sHolidaysStore, holidays, [](WDB::Structures::HolidaysEntry& entry, Row const& row)
        {
            entry.Id = row.id();
        });

        sLogger.info("DB2: world stores ready (maps={}, mapDifficulties={}, areas={}, areaTriggers={}, taxiNodes={}, taxiPaths={}).",
            sMapStore.getNumRows(), sMapDifficultyStore.getNumRows(), sAreaStore.getNumRows(), sAreaTriggerStore.getNumRows(),
            taxiNodes.getRecordCount(), taxiPath.getRecordCount());
        return sMapStore.lookupEntry(0) != nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Small tables of the game systems

    bool loadMiscStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDC5File chatChannels, auctionHouse, bankBagSlotPrices, mailTemplate, emotes, emotesText, questXP, questSort, bannedAddons;
        WDC5File lock, gameObjectDisplayInfo, totemCategory, gemProperties, mountCapability, mountTypeXCapability, currencyTypes;
        WDC5File guildPerkSpells, glyphProperties, achievement, criteria, criteriaTree;
        if (!loadTables(errors, dbcPath, {
            { chatChannels, Layout::ChatChannels }, { auctionHouse, Layout::AuctionHouse }, { bankBagSlotPrices, Layout::BankBagSlotPrices },
            { mailTemplate, Layout::MailTemplate }, { emotes, Layout::Emotes }, { emotesText, Layout::EmotesText }, { questXP, Layout::QuestXP },
            { questSort, Layout::QuestSort }, { bannedAddons, Layout::BannedAddons }, { lock, Layout::Lock },
            { gameObjectDisplayInfo, Layout::GameObjectDisplayInfo }, { totemCategory, Layout::TotemCategory }, { gemProperties, Layout::GemProperties },
            { mountCapability, Layout::MountCapability }, { mountTypeXCapability, Layout::MountTypeXCapability }, { currencyTypes, Layout::CurrencyTypes },
            { guildPerkSpells, Layout::GuildPerkSpells }, { glyphProperties, Layout::GlyphProperties }, { achievement, Layout::Achievement },
            { criteria, Layout::Criteria }, { criteriaTree, Layout::CriteriaTree } }))
            return false;

        fillStore(sChatChannelsStore, chatChannels, [](WDB::Structures::ChatChannelsEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.flags = row.i32(2);
            entry.namePattern = row.str(0);
        });

        fillStore(sAuctionHouseStore, auctionHouse, [](WDB::Structures::AuctionHouseEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.faction = row.u16(1);
            entry.fee = row.u8(2);          // DepositRate
            entry.tax = row.u8(3);          // ConsignmentRate
        });

        fillStore(sBankBagSlotPricesStore, bankBagSlotPrices, [](WDB::Structures::BankBagSlotPricesEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.price = row.u32(0);
        });

        fillContainer(sMailTemplateStore, mailTemplate, [](WDB::Structures::MailTemplateEntry& entry, Row const& row)
        {
            entry.ID = row.id();
            entry.subject = keepString({});
            entry.content = row.text(0);    // Body
        });

        fillContainer(sEmotesStore, emotes, [](WDB::Structures::EmotesEntry& entry, Row const& row)
        {
            entry.Id = row.id();
            entry.Flags = row.i32(2);
            entry.EmoteType = row.i32(3);           // EmoteSpecProc
            entry.UnitStandState = row.u32(4);      // EmoteSpecProcParam
        });

        fillStore(sEmotesTextStore, emotesText, [](WDB::Structures::EmotesTextEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.textId[0] = row.u16(1);           // EmoteID
        });

        fillContainer(sQuestXPStore, questXP, [](WDB::Structures::QuestXP& entry, Row const& row)
        {
            entry.questLevel = row.id();
            for (uint32_t i = 0; i < std::size(entry.xpIndex); ++i)
                entry.xpIndex[i] = row.u16(0, i);
        });

        fillContainer(sQuestSortStore, questSort, [](WDB::Structures::QuestSortEntry& entry, Row const& row)
        {
            entry.id = row.id();
        });

        fillStore(sBannedAddOnsStore, bannedAddons, [](WDB::Structures::BannedAddOnsEntry& entry, Row const& row)
        {
            entry.id = row.id();
        });

        fillContainer(sLockStore, lock, [](WDB::Structures::LockEntry& entry, Row const& row)
        {
            entry.Id = row.id();
            for (uint32_t i = 0; i < LOCK_NUM_CASES; ++i)
            {
                entry.locktype[i] = row.u8(3, i);       // Type
                entry.lockmisc[i] = row.i32(1, i);      // Index
                entry.minlockskill[i] = row.u16(2, i);  // Skill
            }
        });

        // the model is a file data id now, the game object models of the vmaps are keyed by display id
        fillStore(sGameObjectDisplayInfoStore, gameObjectDisplayInfo, [](WDB::Structures::GameObjectDisplayInfoEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.filename = keepString({});
            for (uint32_t i = 0; i < 3; ++i)
            {
                entry.geoBoxMin[i] = row.f32(0, i);
                entry.geoBoxMax[i] = row.f32(0, 3 + i);
            }
        });

        fillStore(sTotemCategoryStore, totemCategory, [](WDB::Structures::TotemCategoryEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.categoryType = row.u8(1);
            entry.categoryMask = row.i32(2);
        });

        fillStore(sGemPropertiesStore, gemProperties, [](WDB::Structures::GemPropertiesEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.enchantmentId = row.u16(0);
            entry.socketMask = row.i32(1);          // Type
        });

        fillContainer(sMountCapabilityStore, mountCapability, [](WDB::Structures::MountCapabilityEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.flag = row.i32(0);
            entry.reqRidingSkill = row.u16(1);
            entry.reqArea = row.u16(2);
            entry.reqAura = row.u32(3);             // ReqSpellAuraID
            entry.reqSpell = row.i32(4);            // ReqSpellKnownID
            entry.speedModSpell = row.i32(5);       // ModSpellAuraID
            entry.reqMap = row.i16(6);
        });

        // MountType is only the relation to its capabilities in the modern tables
        {
            std::map<uint32_t, std::vector<std::pair<uint8_t, uint32_t>>> capabilitiesByType;
            for (Row const row : rows(mountTypeXCapability))
                capabilitiesByType[row.parentOr(0)].emplace_back(row.u8(2), row.u16(1));       // OrderIndex, MountCapabilityID

            uint32_t const maxTypeId = capabilitiesByType.empty() ? 0 : capabilitiesByType.rbegin()->first;
            sMountTypeStore.reserveManualEntries(maxTypeId, static_cast<uint32_t>(capabilitiesByType.size()));
            for (auto& [typeId, capabilities] : capabilitiesByType)
            {
                auto* entry = sMountTypeStore.addManualEntry(typeId);
                if (entry == nullptr)
                    continue;

                entry->id = typeId;
                std::ranges::sort(capabilities);
                for (size_t i = 0; i < std::min<size_t>(capabilities.size(), MAX_MOUNT_CAPABILITIES); ++i)
                    entry->capabilities[i] = capabilities[i].second;
            }
        }

        fillContainer(sCurrencyTypesStore, currencyTypes, [](WDB::Structures::CurrencyTypesEntry& entry, Row const& row)
        {
            entry.item_id = row.id();
            entry.Category = row.i32(2);
            entry.name = row.text(0);
            entry.TotalCap = row.u32(6);            // MaxQty
            entry.WeekCap = row.u32(7);             // MaxEarnablePerWeek
            entry.Flags = row.i32(21, 0);
        });

        fillContainer(sGuildPerkSpellsStore, guildPerkSpells, [](WDB::Structures::GuildPerkSpellsEntry& entry, Row const& row)
        {
            entry.SpellId = row.i32(0);
        });

        fillContainer(sGlyphPropertiesStore, glyphProperties, [](WDB::Structures::GlyphPropertiesEntry& entry, Row const& row)
        {
            entry.Entry = row.id();
            entry.SpellID = row.u32(0);
            entry.Type = row.u8(1);                 // GlyphType
            entry.unk = row.i32(3);                 // SpellIconFileDataID
        });

        // Achievements keep their rows, the criteria of the dbc are the leaves of
        // the criteria trees: a leaf belongs to the achievement that owns the root
        // of its tree and is keyed by the criteria id the client reports progress for.
        std::unordered_map<uint32_t, uint32_t> achievementByRootTree;
        fillContainer(sAchievementStore, achievement, [&achievementByRootTree](WDB::Structures::AchievementEntry& entry, Row const& row)
        {
            entry.ID = row.id();
            entry.factionFlag = row.i8(5);
            entry.mapID = row.i16(4);               // InstanceID
            entry.parentAchievement = row.i32(6);   // Supercedes
            entry.name[0] = row.text(1);
            entry.description[0] = row.text(0);
            entry.categoryId = row.i16(7);
            entry.points = row.u8(9);
            entry.orderInCategory = row.i16(11);    // UiOrder
            entry.flags = row.i32(10);
            entry.icon = row.i32(12);               // IconFileID
            entry.rewardName[0] = row.text(2);
            entry.count = row.u8(8);                // MinimumCriteria
            entry.refAchievement = row.i16(15);     // SharesCriteria
            entry.criteriaTreeID = row.u32(14);
            if (entry.criteriaTreeID != 0)
                achievementByRootTree.try_emplace(entry.criteriaTreeID, entry.ID);
        });

        struct TreeNode
        {
            uint32_t parent;
            uint32_t criteriaId;
            uint32_t amount;
            uint32_t flags;
            uint32_t orderIndex;
            std::string_view description;
        };
        std::unordered_map<uint32_t, TreeNode> treeNodes;
        treeNodes.reserve(criteriaTree.getRecordCount());
        for (Row const row : rows(criteriaTree))
            treeNodes.emplace(row.id(), TreeNode{ row.u32(1), row.u32(4), row.u32(2), static_cast<uint32_t>(row.i32(6)), static_cast<uint32_t>(row.i32(5)), row.str(0) });

        std::unordered_map<uint32_t, Row> criteriaRows;
        criteriaRows.reserve(criteria.getRecordCount());
        for (Row const row : rows(criteria))
            criteriaRows.emplace(row.id(), row);

        // Criteria used by several trees keep the first achievement they were seen
        // with. Only the criteria types the core knows are kept, the modern types
        // above the legacy range have no handling yet.
        struct CriteriaOwner
        {
            uint32_t achievementId;
            TreeNode const* node;
        };
        std::unordered_map<uint32_t, CriteriaOwner> ownerByCriteria;
        for (auto const& [treeId, node] : treeNodes)
        {
            if (node.criteriaId == 0)
                continue;

            auto const criteriaRow = criteriaRows.find(node.criteriaId);
            if (criteriaRow == criteriaRows.end())
                continue;

            int16_t const type = criteriaRow->second.i16(1);
            if (type < 0 || type >= ACHIEVEMENT_CRITERIA_TYPE_TOTAL)
                continue;

            uint32_t rootId = treeId;
            for (uint32_t hop = 0; hop < 32; ++hop)
            {
                auto const itr = treeNodes.find(rootId);
                if (itr == treeNodes.end() || itr->second.parent == 0)
                    break;
                rootId = itr->second.parent;
            }

            auto const owner = achievementByRootTree.find(rootId);
            if (owner == achievementByRootTree.end())
                continue;

            ownerByCriteria.try_emplace(node.criteriaId, CriteriaOwner{ owner->second, &node });
        }

        sAchievementCriteriaStore.reserveManualEntries(maxRecordId(criteria), static_cast<uint32_t>(ownerByCriteria.size()));
        for (auto const& [criteriaId, owner] : ownerByCriteria)
        {
            auto* entry = sAchievementCriteriaStore.addManualEntry(criteriaId);
            if (entry == nullptr)
                continue;

            Row const& row = criteriaRows.at(criteriaId);
            entry->ID = criteriaId;
            entry->referredAchievement = owner.achievementId;
            entry->requiredType = row.i16(1);
            entry->raw.field3 = row.i32(2);                 // Asset
            entry->raw.field4 = owner.node->amount;
            entry->name[0] = keepString(owner.node->description);
            entry->completionFlag = owner.node->flags;
            entry->timeLimit = row.i32(6);                  // StartTimer
            entry->index = owner.node->orderIndex;
        }

        sLogger.info("DB2: misc stores ready (achievements={}, criteria={}, currencies={}, locks={}, mountTypes={}).",
            achievement.getRecordCount(), ownerByCriteria.size(), currencyTypes.getRecordCount(), lock.getRecordCount(), sMountTypeStore.getNumRows());
        return true;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Items

    bool loadItemStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDC5File item, itemAppearance, itemModifiedAppearance, itemExtendedCost, itemSet, itemSetSpell, itemCurrencyCost, itemLimitCategory;
        WDC5File durabilityCosts, durabilityQuality;
        if (!loadTables(errors, dbcPath, {
            { item, Layout::Item }, { itemAppearance, Layout::ItemAppearance }, { itemModifiedAppearance, Layout::ItemModifiedAppearance },
            { itemExtendedCost, Layout::ItemExtendedCost }, { itemSet, Layout::ItemSet }, { itemSetSpell, Layout::ItemSetSpell },
            { itemCurrencyCost, Layout::ItemCurrencyCost }, { itemLimitCategory, Layout::ItemLimitCategory },
            { durabilityCosts, Layout::DurabilityCosts }, { durabilityQuality, Layout::DurabilityQuality } }))
            return false;

        // the display id of an item is its unmodified appearance
        std::unordered_map<uint32_t, uint32_t> displayInfoByAppearance;
        displayInfoByAppearance.reserve(itemAppearance.getRecordCount());
        for (Row const row : rows(itemAppearance))
            displayInfoByAppearance.emplace(row.id(), row.i32(1));       // ItemDisplayInfoID

        std::unordered_map<uint32_t, uint32_t> displayInfoByItem;
        displayInfoByItem.reserve(itemModifiedAppearance.getRecordCount());
        for (Row const row : rows(itemModifiedAppearance))
        {
            if (row.i32(2) != 0)        // ItemAppearanceModifierID
                continue;

            if (auto const display = displayInfoByAppearance.find(row.i32(3)); display != displayInfoByAppearance.end())
                displayInfoByItem.try_emplace(row.parentOr(1), display->second);
        }

        fillContainer(sItemStore, item, [&displayInfoByItem](WDB::Structures::ItemEntry& entry, Row const& row)
        {
            entry.ID = row.id();
            entry.Class = row.i32(0);
            entry.SubClass = row.u8(1);
            entry.SoundOverrideSubclass = row.i8(5);
            entry.Material = row.u8(2);
            if (auto const display = displayInfoByItem.find(entry.ID); display != displayInfoByItem.end())
                entry.DisplayId = display->second;
            entry.InventoryType = row.i8(3);
            entry.Sheath = row.u8(4);
        });

        fillContainer(sItemExtendedCostStore, itemExtendedCost, [](WDB::Structures::ItemExtendedCostEntry& entry, Row const& row)
        {
            entry.costid = row.id();
            entry.arena_slot = row.i8(2);               // ArenaBracket
            entry.personalrating = row.u16(1);          // RequiredArenaRating
            for (uint32_t i = 0; i < 5; ++i)
            {
                entry.item[i] = row.i32(7, i);
                entry.count[i] = row.u16(8, i);
                entry.reqcur[i] = row.u16(9, i);
                entry.reqcurrcount[i] = row.u32(10, i);
            }
        });

        // ItemSet has 17 item slots, the runtime keeps the first ten; the set
        // bonuses are the ItemSetSpell rows of the set without a specialization
        fillStore(sItemSetStore, itemSet, [](WDB::Structures::ItemSetEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.name = row.text(0);
            for (uint32_t i = 0; i < WDB::Structures::MAX_ITEM_SET_ITEMS; ++i)
                entry.itemId[i] = row.u32(4, i);
            entry.requiredSkillId = row.u32(2);
            entry.requiredSkillAmt = row.u16(3);
        });

        std::unordered_map<uint32_t, uint32_t> bonusCountBySet;
        for (Row const row : rows(itemSetSpell))
        {
            if (row.u16(0) != 0 || row.u16(2) != 0)    // ChrSpecID, TraitSubTreeID
                continue;

            auto const set = sItemSetStore.find(row.parent());
            if (set == sItemSetStore.end())
                continue;

            uint32_t& bonusIndex = bonusCountBySet[row.parent()];
            if (bonusIndex >= WDB::Structures::MAX_ITEM_SET_SPELLS)
                continue;

            set->second.spellId[bonusIndex] = row.u32(1);
            set->second.itemsCount[bonusIndex] = row.u8(3);     // Threshold
            ++bonusIndex;
        }

        fillContainer(sItemCurrencyCostStore, itemCurrencyCost, [](WDB::Structures::ItemCurrencyCostEntry& entry, Row const& row)
        {
            entry.itemid = row.parentOr(0);
        });

        fillContainer(sItemLimitCategoryStore, itemLimitCategory, [](WDB::Structures::ItemLimitCategoryEntry& entry, Row const& row)
        {
            entry.Id = row.id();
            entry.maxAmount = row.u8(1);            // Quantity
            entry.equippedFlag = row.i32(2);        // Flags
        });

        fillStore(sDurabilityCostsStore, durabilityCosts, [](WDB::Structures::DurabilityCostsEntry& entry, Row const& row)
        {
            entry.itemLevel = row.id();
            for (uint32_t i = 0; i < 21; ++i)
                entry.modifier[i] = row.u16(0, i);      // WeaponSubClassCost
            for (uint32_t i = 0; i < 8; ++i)
                entry.modifier[21 + i] = row.u16(1, i); // ArmorSubClassCost
        });

        fillStore(sDurabilityQualityStore, durabilityQuality, [](WDB::Structures::DurabilityQualityEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.qualityModifier = row.f32(0);
        });

        // random properties and suffixes do not exist in the modern item system, the empty stores are
        // filled from the world tables item_randomprop and item_randomsuffix afterwards
        sItemRandomPropertiesStore.clear();
        sItemRandomSuffixStore.clear();

        sLogger.info("DB2: item stores ready (items={}, displayIds={}, extendedCosts={}, sets={}).",
            item.getRecordCount(), displayInfoByItem.size(), itemExtendedCost.getRecordCount(), sItemSetStore.getNumRows());
        return true;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Skills, talents, spells

    bool loadSpellStores(WDB::StoreProblemList& errors, std::string const& dbcPath)
    {
        WDC5File skillLine, skillLineAbility, talent, spellName, spellAuraOptions, spellAuraRestrictions, spellCastTimes, spellCastingRequirements;
        WDC5File spellCategories, spellClassOptions, spellCooldowns, spellDuration, spellEffect, spellEquippedItems, spellInterrupts, spellLevels;
        WDC5File spellMisc, spellPower, spellRadius, spellRange, spellReagents, spellScaling, spellShapeshift, spellShapeshiftForm;
        WDC5File spellTargetRestrictions, spellTotems, spellItemEnchantment, spellXSpellVisual;
        if (!loadTables(errors, dbcPath, {
            { skillLine, Layout::SkillLine }, { skillLineAbility, Layout::SkillLineAbility }, { talent, Layout::Talent }, { spellName, Layout::SpellName },
            { spellAuraOptions, Layout::SpellAuraOptions }, { spellAuraRestrictions, Layout::SpellAuraRestrictions }, { spellCastTimes, Layout::SpellCastTimes },
            { spellCastingRequirements, Layout::SpellCastingRequirements }, { spellCategories, Layout::SpellCategories }, { spellClassOptions, Layout::SpellClassOptions },
            { spellCooldowns, Layout::SpellCooldowns }, { spellDuration, Layout::SpellDuration }, { spellEffect, Layout::SpellEffect },
            { spellEquippedItems, Layout::SpellEquippedItems }, { spellInterrupts, Layout::SpellInterrupts }, { spellLevels, Layout::SpellLevels },
            { spellMisc, Layout::SpellMisc }, { spellPower, Layout::SpellPower }, { spellRadius, Layout::SpellRadius }, { spellRange, Layout::SpellRange },
            { spellReagents, Layout::SpellReagents }, { spellScaling, Layout::SpellScaling }, { spellShapeshift, Layout::SpellShapeshift },
            { spellShapeshiftForm, Layout::SpellShapeshiftForm }, { spellTargetRestrictions, Layout::SpellTargetRestrictions }, { spellTotems, Layout::SpellTotems },
            { spellItemEnchantment, Layout::SpellItemEnchantment }, { spellXSpellVisual, Layout::SpellXSpellVisual } }))
            return false;

        fillContainer(sSkillLineStore, skillLine, [](WDB::Structures::SkillLineEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.type = row.i8(6);             // CategoryID
            entry.Name[0] = row.text(0);
            entry.spell_icon = row.i32(7);      // SpellIconFileID
            entry.linkable = row.i8(8);         // CanLink
        });

        fillContainer(sSkillLineAbilityStore, skillLineAbility, [](WDB::Structures::SkillLineAbilityEntry& entry, Row const& row)
        {
            entry.Id = row.id();
            entry.skilline = row.parentOr(3);
            entry.spell = row.i32(4);
            entry.race_mask = row.i32(16, 0);
            entry.class_mask = row.i32(6);
            entry.minSkillLineRank = row.i16(5);
            entry.next = row.i32(7);            // SupercedesSpell
            entry.acquireMethod = row.i32(8);
            entry.grey = row.i16(9);            // TrivialSkillLineRankHigh
            entry.green = row.i16(10);          // TrivialSkillLineRankLow
        });

        fillContainer(sTalentStore, talent, [](WDB::Structures::TalentEntry& entry, Row const& row)
        {
            entry.TalentID = row.id();
            entry.Row = row.u8(1);              // TierID
            entry.Col = row.u8(3);              // ColumnIndex
            entry.SpellId = row.u32(7);
            entry.playerClass = row.i8(5);
            entry.overrideSpellId = row.u32(8);
        });

        // One SpellEntry per SpellName row, the split tables link themselves to it
        // through their spell id (a column or the relation parent). Rows of a
        // difficulty other than the base one are kept but not linked.
        std::unordered_map<uint32_t, WDB::Structures::SpellEntry*> spells;
        spells.reserve(spellName.getRecordCount());
        fillContainer(sSpellStore, spellName, [&spells](WDB::Structures::SpellEntry& entry, Row const& row)
        {
            entry.Id = row.id();
            entry.Name = row.text(0);
            entry.Rank = keepString({});
            spells.emplace(entry.Id, &entry);
        });

        auto const linkSpell = [&spells](uint32_t spellId, uint32_t rowId, uint32_t WDB::Structures::SpellEntry::* member)
        {
            if (spellId == 0)
                return;

            if (auto const itr = spells.find(spellId); itr != spells.end() && itr->second->*member == 0)
                itr->second->*member = rowId;
        };

        // the split tables with a difficulty column link only their base rows
        auto const isBaseDifficulty = [](Row const& row) { return row.i16(0) == 0; };

        fillContainer(sSpellAuraOptionsStore, spellAuraOptions, [&](WDB::Structures::SpellAuraOptionsEntry& entry, Row const& row)
        {
            entry.Id = row.id();
            entry.MaxStackAmount = row.u16(1);      // CumulativeAura
            entry.procChance = row.u8(3);
            entry.procCharges = row.i32(4);
            entry.procFlags = row.i32(6, 0);        // ProcTypeMask
            if (isBaseDifficulty(row))
                linkSpell(row.parent(), entry.Id, &WDB::Structures::SpellEntry::SpellAuraOptionsId);
        });

        fillContainer(sSpellAuraRestrictionsStore, spellAuraRestrictions, [&](WDB::Structures::SpellAuraRestrictionsEntry& entry, Row const& row)
        {
            entry.CasterAuraState = row.i32(1);
            entry.TargetAuraState = row.i32(2);
            entry.CasterAuraStateNot = row.i32(3);
            entry.TargetAuraStateNot = row.i32(4);
            entry.casterAuraSpell = row.i32(5);
            entry.targetAuraSpell = row.i32(6);
            entry.CasterAuraSpellNot = row.i32(7);
            entry.TargetAuraSpellNot = row.i32(8);
            if (isBaseDifficulty(row))
                linkSpell(row.parent(), row.id(), &WDB::Structures::SpellEntry::SpellAuraRestrictionsId);
        });

        fillContainer(sSpellCastTimesStore, spellCastTimes, [](WDB::Structures::SpellCastTimesEntry& entry, Row const& row)
        {
            entry.ID = row.id();
            entry.CastTime = row.i32(0);        // Base
            entry.MinCastTime = row.i32(1);     // Minimum
        });

        fillContainer(sSpellCastingRequirementsStore, spellCastingRequirements, [&](WDB::Structures::SpellCastingRequirementsEntry& entry, Row const& row)
        {
            entry.FacingCasterFlags = row.i32(1);
            entry.AreaGroupId = row.u16(4);         // RequiredAreasID
            entry.RequiresSpellFocus = row.u16(6);
            linkSpell(row.i32(0), row.id(), &WDB::Structures::SpellEntry::SpellCastingRequirementsId);
        });

        fillContainer(sSpellCategoriesStore, spellCategories, [&](WDB::Structures::SpellCategoriesEntry& entry, Row const& row)
        {
            entry.Category = row.i16(1);
            entry.DmgClass = row.i8(2);             // DefenseType
            entry.DispelType = row.i8(4);
            entry.MechanicsType = row.i8(5);        // Mechanic
            entry.PreventionType = row.i32(6);
            entry.StartRecoveryCategory = row.i16(7);
            if (isBaseDifficulty(row))
                linkSpell(row.parent(), row.id(), &WDB::Structures::SpellEntry::SpellCategoriesId);
        });

        fillContainer(sSpellClassOptionsStore, spellClassOptions, [&](WDB::Structures::SpellClassOptionsEntry& entry, Row const& row)
        {
            for (uint32_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
                entry.SpellFamilyFlags[i] = row.i32(3, i);  // SpellClassMask
            entry.SpellFamilyName = row.i32(2);             // SpellClassSet
            linkSpell(row.i32(0), row.id(), &WDB::Structures::SpellEntry::SpellClassOptionsId);
        });

        fillContainer(sSpellCooldownsStore, spellCooldowns, [&](WDB::Structures::SpellCooldownsEntry& entry, Row const& row)
        {
            entry.CategoryRecoveryTime = row.i32(1);
            entry.RecoveryTime = row.i32(2);
            entry.StartRecoveryTime = row.i32(3);
            if (isBaseDifficulty(row))
                linkSpell(row.parent(), row.id(), &WDB::Structures::SpellEntry::SpellCooldownsId);
        });

        fillContainer(sSpellDurationStore, spellDuration, [](WDB::Structures::SpellDurationEntry& entry, Row const& row)
        {
            entry.ID = row.id();
            entry.Duration1 = row.i32(0);       // Duration
            entry.Duration2 = row.i32(1);       // MaxDuration
            entry.Duration3 = row.i32(2);       // DurationPerResource
        });

        // Only the effects of the base difficulty are kept: the effect map holds one
        // effect per spell and index and the other difficulties would overwrite it.
        {
            auto baseEffects = rows(spellEffect) | std::views::filter([](Row const& row) { return row.i16(1) == 0; });
            sSpellEffectStore.reserveManualEntries(maxRecordId(spellEffect), static_cast<uint32_t>(std::ranges::distance(baseEffects)));
            for (Row const row : baseEffects)
            {
                auto* entry = sSpellEffectStore.addManualEntry(row.id());
                if (entry == nullptr)
                    continue;

                entry->id = row.id();
                entry->Effect = row.u32(3);
                entry->EffectMultipleValue = row.f32(4);            // EffectAmplitude
                entry->EffectApplyAuraName = row.i16(0);            // EffectAura
                entry->EffectAmplitude = row.i32(6);                // EffectAuraPeriod
                entry->EffectBasePoints = static_cast<int32_t>(row.f32(22));
                entry->EffectSpellPowerCoefficient = row.f32(7);    // EffectBonusCoefficient
                entry->EffectDamageMultiplier = row.f32(18);        // Coefficient
                entry->EffectChainTarget = row.i32(9);
                entry->EffectItemType = row.i32(10);
                entry->EffectMechanic = row.i32(11);
                entry->EffectMiscValue = row.i32(25, 0);
                entry->EffectMiscValueB = row.i32(25, 1);
                entry->EffectPointsPerComboPoint = row.f32(12);     // EffectPointsPerResource
                entry->EffectRadiusIndex = row.u32(26, 0);
                entry->EffectRadiusMaxIndex = row.u32(26, 1);
                entry->EffectRealPointsPerLevel = row.f32(14);
                for (uint32_t i = 0; i < std::size(entry->EffectSpellClassMask); ++i)
                    entry->EffectSpellClassMask[i] = row.i32(27, i);
                entry->EffectTriggerSpell = row.i32(15);
                entry->EffectImplicitTargetA = row.i16(28, 0);
                entry->EffectImplicitTargetB = row.i16(28, 1);
                entry->EffectSpellId = row.parent();
                entry->EffectIndex = std::min<uint32_t>(row.i32(2), 31);
            }
        }

        fillContainer(sSpellEquippedItemsStore, spellEquippedItems, [&](WDB::Structures::SpellEquippedItemsEntry& entry, Row const& row)
        {
            entry.EquippedItemClass = row.i32(1);
            entry.EquippedItemInventoryTypeMask = row.i32(2);   // EquippedItemInvTypes
            entry.EquippedItemSubClassMask = row.i32(3);        // EquippedItemSubclass
            linkSpell(row.i32(0), row.id(), &WDB::Structures::SpellEntry::SpellEquippedItemsId);
        });

        fillContainer(sSpellInterruptsStore, spellInterrupts, [&](WDB::Structures::SpellInterruptsEntry& entry, Row const& row)
        {
            entry.AuraInterruptFlags = row.i32(2, 0);
            entry.ChannelInterruptFlags = row.i32(3, 0);
            entry.InterruptFlags = row.i32(1);
            if (isBaseDifficulty(row))
                linkSpell(row.parent(), row.id(), &WDB::Structures::SpellEntry::SpellInterruptsId);
        });

        fillContainer(sSpellLevelsStore, spellLevels, [&](WDB::Structures::SpellLevelsEntry& entry, Row const& row)
        {
            entry.baseLevel = row.i32(3);
            entry.maxLevel = row.i16(1);
            entry.spellLevel = row.i32(4);
            if (isBaseDifficulty(row))
                linkSpell(row.parent(), row.id(), &WDB::Structures::SpellEntry::SpellLevelsId);
        });

        // The cast visual of a spell is a SpellXSpellVisual row the client resolves
        // itself; the server sends the row id. Of the rows of the base difficulty
        // an unconditional one with the highest priority is the default. Many
        // class spells only have rows with a caster condition (the class or the
        // specialization), those are taken when nothing unconditional exists, the
        // conditions themselves are not evaluated yet.
        struct SpellVisual
        {
            uint32_t rowId;
            uint32_t spellVisualId;
            int32_t priority;
            bool conditional;

            [[nodiscard]] bool betterThan(SpellVisual const& other) const noexcept
            {
                if (conditional != other.conditional)
                    return !conditional;
                return priority > other.priority;
            }
        };
        std::unordered_map<uint32_t, SpellVisual> visualBySpell;
        visualBySpell.reserve(spellXSpellVisual.getRecordCount());
        for (Row const row : rows(spellXSpellVisual))
        {
            if (row.i16(1) != 0)                        // DifficultyID
                continue;

            bool const conditional = row.u16(8) != 0 || row.u32(9) != 0 || row.u16(10) != 0 || row.u32(11) != 0;
            SpellVisual const visual{ row.id(), row.u32(2), row.i32(5), conditional };
            auto const [itr, inserted] = visualBySpell.try_emplace(row.parent(), visual);
            if (!inserted && visual.betterThan(itr->second))
                itr->second = visual;
        }

        fillContainer(sSpellMiscStore, spellMisc, [&](WDB::Structures::SpellMiscEntry& entry, Row const& row)
        {
            entry.Id = row.id();
            entry.SpellDifficultyId = row.i16(1);
            if (auto const visual = visualBySpell.find(row.parent()); visual != visualBySpell.end())
            {
                entry.SpellVisual = visual->second.rowId;
                entry.SpellVisual1 = visual->second.spellVisualId;
            }
            uint32_t* const attributes[] = {
                &entry.Attributes, &entry.AttributesEx, &entry.AttributesExB, &entry.AttributesExC, &entry.AttributesExD,
                &entry.AttributesExE, &entry.AttributesExF, &entry.AttributesExG, &entry.AttributesExH, &entry.AttributesExI,
                &entry.AttributesExJ, &entry.AttributesExK, &entry.AttributesExL, &entry.AttributesExM };
            for (uint32_t i = 0; i < std::size(attributes); ++i)
                *attributes[i] = row.i32(0, i);
            entry.CastingTimeIndex = row.u16(2);
            entry.DurationIndex = row.u16(3);
            entry.rangeIndex = row.u16(5);
            entry.speed = row.f32(7);
            entry.spellIconID = row.i32(10);        // SpellIconFileDataID
            entry.activeIconID = row.i32(11);       // ActiveIconFileDataID
            entry.School = row.u8(6);               // SchoolMask
            if (row.i16(1) == 0)
                linkSpell(row.parent(), entry.Id, &WDB::Structures::SpellEntry::SpellMiscId);
        });

        fillContainer(sSpellPowerStore, spellPower, [](WDB::Structures::SpellPowerEntry& entry, Row const& row)
        {
            entry.spellId = row.parent();
            entry.powerType = static_cast<int32_t>(row.i8(11));
            entry.manaCost = row.i32(2);
            entry.manaCostPerlevel = row.i32(3);
            entry.manaPerSecond = row.i32(4);
            entry.ManaCostPercentageFloat = row.f32(7);         // PowerCostPct
            entry.ChannelCostPercentageFloat = row.f32(10);     // PowerPctPerSecond
            entry.ShapeShiftSpellId = row.i32(12);              // RequiredAuraSpellID
        });

        fillContainer(sSpellRadiusStore, spellRadius, [](WDB::Structures::SpellRadiusEntry& entry, Row const& row)
        {
            entry.ID = row.id();
            entry.radius_min = row.f32(0);          // Radius, the value the dbc column held
            entry.radius_per_level = row.f32(1);
            entry.radius_max = row.f32(3);
        });

        fillContainer(sSpellRangeStore, spellRange, [](WDB::Structures::SpellRangeEntry& entry, Row const& row)
        {
            entry.ID = row.id();
            entry.minRange = row.f32(3, 0);
            entry.minRangeFriendly = row.f32(3, 1);
            entry.maxRange = row.f32(4, 0);
            entry.maxRangeFriendly = row.f32(4, 1);
            entry.range_type = row.i32(2);          // Flags
        });

        fillContainer(sSpellReagentsStore, spellReagents, [&](WDB::Structures::SpellReagentsEntry& entry, Row const& row)
        {
            for (uint32_t i = 0; i < MAX_SPELL_REAGENTS; ++i)
            {
                entry.Reagent[i] = row.i32(1, i);
                entry.ReagentCount[i] = row.i16(2, i);
            }
            linkSpell(row.i32(0), row.id(), &WDB::Structures::SpellEntry::SpellReagentsId);
        });

        // the scaling coefficients moved into SpellEffect, only the level range is left
        fillContainer(sSpellScalingStore, spellScaling, [&](WDB::Structures::SpellScalingEntry& entry, Row const& row)
        {
            entry.castScalingMaxLevel = row.u32(2);
            linkSpell(row.i32(0), row.id(), &WDB::Structures::SpellEntry::SpellScalingId);
        });

        fillContainer(sSpellShapeshiftStore, spellShapeshift, [&](WDB::Structures::SpellShapeshiftEntry& entry, Row const& row)
        {
            entry.ShapeshiftsExcluded = row.i32(2, 0);
            entry.Shapeshifts = row.i32(3, 0);
            linkSpell(row.i32(0), row.id(), &WDB::Structures::SpellEntry::SpellShapeshiftId);
        });

        fillContainer(sSpellShapeshiftFormStore, spellShapeshiftForm, [](WDB::Structures::SpellShapeshiftFormEntry& entry, Row const& row)
        {
            entry.id = row.id();
            entry.Flags = row.i32(3);
            entry.unit_type = row.u8(2);            // CreatureType
            entry.AttackSpeed = row.i16(6);         // CombatRoundTime
            entry.modelId = row.u32(1);             // CreatureDisplayID
            entry.modelId2 = entry.modelId;
            for (uint32_t i = 0; i < std::size(entry.spells); ++i)
                entry.spells[i] = row.u32(9, i);    // PresetSpellID
        });

        fillContainer(sSpellTargetRestrictionsStore, spellTargetRestrictions, [&](WDB::Structures::SpellTargetRestrictionsEntry& entry, Row const& row)
        {
            entry.Id = row.id();
            entry.MaxAffectedTargets = row.u8(2);   // MaxTargets
            entry.MaxTargetLevel = row.u32(3);
            entry.TargetCreatureType = row.i16(4);
            entry.Targets = row.i32(5);
            if (isBaseDifficulty(row))
                linkSpell(row.parent(), entry.Id, &WDB::Structures::SpellEntry::SpellTargetRestrictionsId);
        });

        fillContainer(sSpellTotemsStore, spellTotems, [&](WDB::Structures::SpellTotemsEntry& entry, Row const& row)
        {
            for (uint32_t i = 0; i < MAX_SPELL_TOTEM_CATEGORIES; ++i)
                entry.TotemCategory[i] = row.u16(1, i);
            for (uint32_t i = 0; i < MAX_SPELL_TOTEMS; ++i)
                entry.Totem[i] = row.i32(2, i);
            linkSpell(row.i32(0), row.id(), &WDB::Structures::SpellEntry::SpellTotemsId);
        });

        fillContainer(sSpellItemEnchantmentStore, spellItemEnchantment, [](WDB::Structures::SpellItemEnchantmentEntry& entry, Row const& row)
        {
            entry.Id = row.id();
            for (uint32_t i = 0; i < MAX_ITEM_ENCHANTMENT_EFFECTS; ++i)
            {
                entry.type[i] = row.i32(4, i);      // Effect
                entry.min[i] = row.i32(5, i);       // EffectPointsMin
                entry.spell[i] = row.u32(6, i);     // EffectArg
            }
            entry.Name[0] = row.text(0);
            entry.visual = row.u16(22);             // ItemVisual
            entry.EnchantGroups = row.i32(7);       // Flags
            entry.ench_condition = row.i32(11);     // ConditionID
            entry.req_skill = row.i32(12);
            entry.req_skill_value = row.i32(13);
            entry.req_level = row.i32(14);          // MinLevel
        });

        // tables of the dbc era without a modern counterpart
        sSpellRuneCostStore.clear();
        sSpellDifficultyStore.clear();
        sTalentTabStore.clear();
        sTalentTreePrimarySpellsStore.clear();
        sNumTalentsAtLevel.clear();
        sScalingStatDistributionStore.clear();
        sScalingStatValuesStore.clear();
        sGlyphSlotStore.clear();

        sLogger.info("DB2: spell stores ready (skills={}, skillAbilities={}, talents={}, spells={}, effects={}, spellMisc={}).",
            skillLine.getRecordCount(), skillLineAbility.getRecordCount(), talent.getRecordCount(), spellName.getRecordCount(),
            sSpellEffectStore.getNumRows(), spellMisc.getRecordCount());
        return true;
    }
}

bool loadDb2Stores(WDB::StoreProblemList& errors, std::string const& dbcPath)
{
    gStringPool.clear();

    // the modern clients keep their game tables (gt*) as text files, those stores stay empty
    sLogger.info("DB2: loading the client tables from {}", dbcPath);

    bool ok = true;
    for (auto const load : { &loadCharacterStores, &loadCreatureStores, &loadWorldStores, &loadMiscStores, &loadItemStores, &loadSpellStores })
        ok = load(errors, dbcPath) && ok;
    return ok;
}

#endif
