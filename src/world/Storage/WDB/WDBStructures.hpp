/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "WDBDefines.hpp"

#include <cstring>
#include <string>
#include <map>
#include <set>

struct WMOAreaTableTripple
{
    WMOAreaTableTripple(int32_t r, int32_t a, int32_t g) : groupId(g), rootId(r), adtId(a) { }

    bool operator <(const WMOAreaTableTripple& b) const
    {
        return memcmp(this, &b, sizeof(WMOAreaTableTripple)) < 0;
    }

    int32_t groupId;
    int32_t rootId;
    int32_t adtId;
};

///\ These will be verified and ported to Spell/Definitions/SpellEffectTarget.hpp when spell targeting is being rewritten -Appled
enum Targets
{
    TARGET_NONE                                 = 0,
    TARGET_SELF_ONE                             = 1,
    TARGET_RANDOM_ENEMY_CHAIN_IN_AREA           = 2,  // only one spell has that, but regardless, it's a target type after all
    TARGET_RANDOM_FRIEND_CHAIN_IN_AREA          = 3,
    TARGET_RANDOM_UNIT_CHAIN_IN_AREA            = 4,  // some plague spells that are infectious - maybe targets not-infected friends inrange
    TARGET_PET                                  = 5,
    TARGET_CHAIN_DAMAGE                         = 6,
    TARGET_AREAEFFECT_INSTANT                   = 7,  // targets around provided destination point
    TARGET_AREAEFFECT_CUSTOM                    = 8,
    TARGET_INNKEEPER_COORDINATES                = 9,  // uses in teleport to innkeeper spells
    TARGET_11                                   = 11, // used by spell 4 'Word of Recall Other'
    TARGET_ALL_ENEMY_IN_AREA                    = 15,
    TARGET_ALL_ENEMY_IN_AREA_INSTANT            = 16,
    TARGET_TABLE_X_Y_Z_COORDINATES              = 17, // uses in teleport spells and some other
    TARGET_EFFECT_SELECT                        = 18, // highly depends on the spell effect
    TARGET_ALL_PARTY_AROUND_CASTER              = 20,
    TARGET_SINGLE_FRIEND                        = 21,
    TARGET_CASTER_COORDINATES                   = 22, // used only in TargetA, target selection dependent from TargetB
    TARGET_GAMEOBJECT                           = 23,
    TARGET_IN_FRONT_OF_CASTER                   = 24,
    TARGET_DUELVSPLAYER                         = 25,
    TARGET_GAMEOBJECT_ITEM                      = 26,
    TARGET_MASTER                               = 27,
    TARGET_ALL_ENEMY_IN_AREA_CHANNELED          = 28,
    TARGET_29                                   = 29,
    TARGET_ALL_FRIENDLY_UNITS_AROUND_CASTER     = 30, // select friendly for caster object faction (in different original caster faction) in TargetB used only with TARGET_ALL_AROUND_CASTER and in self casting range in TargetA
    TARGET_ALL_FRIENDLY_UNITS_IN_AREA           = 31,
    TARGET_MINION                               = 32,
    TARGET_ALL_PARTY                            = 33,
    TARGET_ALL_PARTY_AROUND_CASTER_2            = 34, // used in Tranquility
    TARGET_SINGLE_PARTY                         = 35,
    TARGET_ALL_HOSTILE_UNITS_AROUND_CASTER      = 36,
    TARGET_AREAEFFECT_PARTY                     = 37,
    TARGET_SCRIPT                               = 38,
    TARGET_SELF_FISHING                         = 39,
    TARGET_FOCUS_OR_SCRIPTED_GAMEOBJECT         = 40,
    TARGET_TOTEM_EARTH                          = 41,
    TARGET_TOTEM_WATER                          = 42,
    TARGET_TOTEM_AIR                            = 43,
    TARGET_TOTEM_FIRE                           = 44,
    TARGET_CHAIN_HEAL                           = 45,
    TARGET_SCRIPT_COORDINATES                   = 46,
    TARGET_DYNAMIC_OBJECT_FRONT                 = 47,
    TARGET_DYNAMIC_OBJECT_BEHIND                = 48,
    TARGET_DYNAMIC_OBJECT_LEFT_SIDE             = 49,
    TARGET_DYNAMIC_OBJECT_RIGHT_SIDE            = 50,
    TARGET_AREAEFFECT_GO_AROUND_SOURCE          = 51,
    TARGET_AREAEFFECT_GO_AROUND_DEST            = 52, // gameobject around destination, select by spell_script_target
    TARGET_CURRENT_ENEMY_COORDINATES            = 53, // set unit coordinates as dest, only 16 target B imlemented
    TARGET_LARGE_FRONTAL_CONE                   = 54,
    TARGET_ALL_RAID_AROUND_CASTER               = 56,
    TARGET_SINGLE_FRIEND_2                      = 57,
    TARGET_58                                   = 58,
    TARGET_FRIENDLY_FRONTAL_CONE                = 59,
    TARGET_NARROW_FRONTAL_CONE                  = 60,
    TARGET_AREAEFFECT_PARTY_AND_CLASS           = 61,
    TARGET_DUELVSPLAYER_COORDINATES             = 63,
    TARGET_INFRONT_OF_VICTIM                    = 64,
    TARGET_BEHIND_VICTIM                        = 65, // used in teleport behind spells, caster/target dependent from spell effect
    TARGET_RIGHT_FROM_VICTIM                    = 66,
    TARGET_LEFT_FROM_VICTIM                     = 67,
    TARGET_68                                   = 68,
    TARGET_69                                   = 69,
    TARGET_70                                   = 70,
    TARGET_RANDOM_NEARBY_LOC                    = 72, // used in teleport onto nearby locations
    TARGET_RANDOM_CIRCUMFERENCE_POINT           = 73,
    TARGET_74                                   = 74,
    TARGET_75                                   = 75,
    TARGET_DYNAMIC_OBJECT_COORDINATES           = 76,
    TARGET_SINGLE_ENEMY                         = 77,
    TARGET_POINT_AT_NORTH                       = 78, // 78-85 possible _COORDINATES at radius with pi/4 step around target in unknown order, N?
    TARGET_POINT_AT_SOUTH                       = 79, // S?
    TARGET_POINT_AT_EAST                        = 80, // 80/81 must be symmetric from line caster->target, E (base at 82/83, 84/85 order) ?
    TARGET_POINT_AT_WEST                        = 81, // 80/81 must be symmetric from line caster->target, W (base at 82/83, 84/85 order) ?
    TARGET_POINT_AT_NE                          = 82, // from spell desc: "(NE)"
    TARGET_POINT_AT_NW                          = 83, // from spell desc: "(NW)"
    TARGET_POINT_AT_SE                          = 84, // from spell desc: "(SE)"
    TARGET_POINT_AT_SW                          = 85, // from spell desc: "(SW)"
    TARGET_RANDOM_NEARBY_DEST                   = 86, // "Test Nearby Dest Random" - random around selected destination
    TARGET_SELF2                                = 87,
    TARGET_88                                   = 88, // Smoke Flare(s) and Hurricane
    TARGET_DIRECTLY_FORWARD                     = 89,
    TARGET_NONCOMBAT_PET                        = 90,
    TARGET_91                                   = 91,
    TARGET_SUMMONER                             = 92,
    TARGET_CONTROLLED_VEHICLE                   = 94,
    TARGET_VEHICLE_DRIVER                       = 95,
    TARGET_VEHICLE_PASSENGER_0                  = 96,
    TARGET_VEHICLE_PASSENGER_1                  = 97,
    TARGET_VEHICLE_PASSENGER_2                  = 98,
    TARGET_VEHICLE_PASSENGER_3                  = 99,
    TARGET_VEHICLE_PASSENGER_4                  = 100,
    TARGET_VEHICLE_PASSENGER_5                  = 101,
    TARGET_VEHICLE_PASSENGER_6                  = 102,
    TARGET_VEHICLE_PASSENGER_7                  = 103,
    TARGET_IN_FRONT_OF_CASTER_30                = 104,
    TARGET_105                                  = 105,
    TARGET_106                                  = 106,
    TARGET_107                                  = 107, // possible TARGET_WMO(GO?)_IN_FRONT_OF_CASTER(_30 ?) TODO: Verify the angle!
    TARGET_GO_IN_FRONT_OF_CASTER_90             = 108,
    TARGET_109                                  = 109, // spell 89008
    TARGET_NARROW_FRONTAL_CONE_2                = 110,
    TARGET_111                                  = 111, // not used
    TARGET_112                                  = 112, // spell 89549
    TARGET_113                                  = 113, // not used
    TARGET_114                                  = 114, // not used
    TARGET_115                                  = 115, // not used
    TARGET_116                                  = 116, // not used
    TARGET_117                                  = 117, // test spell 83658
    TARGET_118                                  = 118, // test spell 79579
    TARGET_119                                  = 119, // mass ressurection 83968
    TARGET_120                                  = 120,
    TARGET_121                                  = 121, // spell 95750
    TARGET_122                                  = 122, // spell 100661
    TARGET_123                                  = 123,
    TARGET_124                                  = 124,
    TARGET_125                                  = 125,
    TARGET_126                                  = 126,
    TARGET_127                                  = 127,
};

struct DBCPosition3D
{
    float X;
    float Y;
    float Z;
};

namespace WDB::Structures
{
    struct AchievementCategoryEntry
    {
        uint32_t id{0};
        int32_t parent{-1};
    };

    // a criteria of an achievement before 5.4, from 5.4 on the criteria trees replace it
    struct AchievementCriteriaEntry
    {
        uint32_t ID{0};
        uint32_t referredAchievement{0};
        uint32_t requiredType{0};
        uint32_t asset{0};                                          // creature, item, quest, map, ... of the type
        uint32_t quantity{0};
        uint32_t startEvent{0};
        uint32_t startAsset{0};
        uint32_t failEvent{0};
        uint32_t failAsset{0};
        std::string name;
        uint32_t flags{0};
        uint32_t timerStartEvent{0};
        uint32_t timerAsset{0};
        uint32_t timerTime{0};                                      // time limit in seconds
        uint32_t order{0};
        uint32_t worldStateId{0};                                   // 4.x and later
        int32_t worldStateValue{0};
        std::array<uint32_t, 3> conditionType{0, 0, 0};            // 4.x and later, the modifier types of 5.4
        std::array<uint32_t, 3> conditionValue{0, 0, 0};
    };

    struct AchievementEntry
    {
        uint32_t ID{0};
        int32_t factionFlag{-1};                                    // -1 both, 0 Horde, 1 Alliance
        int32_t mapID{-1};                                          // -1 every map
        uint32_t parentAchievement{0};
        std::string name;
        std::string description;
        uint32_t categoryId{0};
        uint32_t points{0};
        uint32_t orderInCategory{0};
        uint32_t flags{0};
        uint32_t icon{0};
        std::string rewardName;
        uint32_t count{0};
        uint32_t refAchievement{0};
        uint32_t criteriaTreeID{0};                                 // 5.4 and later
    };

    struct AreaGroupEntry
    {
        uint32_t id{0};
        uint32_t areaId[6]{};
        uint32_t nextGroup{0};
    };

    struct AreaTableEntry
    {
        uint32_t id{0};
        uint32_t map_id{0};
        uint32_t zone{0};
        uint32_t explore_flag{0};
        uint32_t flags{0};
        int32_t area_level{0};
        std::string area_name;
        uint32_t team{0};
        uint32_t liquid_type_override[4]{};
        float elevation{0.0f};
    };

    struct AreaTriggerEntry
    {
        uint32_t id{0};
        uint32_t mapId{0};
        float x{0.0f};
        float y{0.0f};
        float z{0.0f};
        float boxRadius{0.0f};
        float boxX{0.0f};
        float boxY{0.0f};
        float boxZ{0.0f};
        float boxOrientation{0.0f};
    };

    struct AuctionHouseEntry
    {
        uint32_t id{0};
        uint32_t faction{0};
        uint32_t fee{0};
        uint32_t tax{0};
    };

    struct BankBagSlotPricesEntry
    {
        uint32_t id{0};
        uint32_t price{0};
    };

    struct BannedAddOnsEntry
    {
        uint32_t id{0};
    };

    struct BarberShopStyleEntry
    {
        uint32_t id{0};
        uint32_t type{0};
        uint32_t race{0};
        uint32_t gender{0};
        uint32_t hairId{0};
    };

    struct ChatChannelsEntry
    {
        uint32_t id{0};
        uint32_t flags{0};
        std::string namePattern;
    };

    constexpr size_t MAX_OUTFIT_ITEMS = 24;

    struct CharStartOutfitEntry
    {
        uint8_t race{0};
        uint8_t classId{0};
        uint8_t gender{0};
        int32_t itemId[MAX_OUTFIT_ITEMS]{};
        uint32_t petDisplayId{0};
        uint32_t petFamilyEntry{0};

        [[nodiscard]] constexpr uint32_t makeKey() const noexcept
        {
            return generateKey(race, classId, gender);
        }

        [[nodiscard]] static constexpr uint32_t generateKey(uint8_t race, uint8_t classId, uint8_t gender) noexcept
        {
            return static_cast<uint32_t>(race)
                | (static_cast<uint32_t>(classId) << 8)
                | (static_cast<uint32_t>(gender) << 16);
        }
    };

    struct CharTitlesEntry
    {
        uint32_t id{0};
        uint32_t conditionId{0};
        std::string nameMale;
        std::string nameFemale;
        uint32_t bitIndex{0};
    };

    struct ChrClassesEntry
    {
        uint32_t classId{0};
        uint32_t powerType{0};
        std::string name;
        uint32_t spellClassSet{0};
        uint32_t cinematicSequenceId{0};
        uint32_t requiredExpansion{0};
        uint32_t apPerStr{0};
        uint32_t apPerAgi{0};
        uint32_t rapPerAgi{0};
    };

    struct ChrPowerTypesEntry
    {
        uint32_t entry{0};
        uint32_t classId{0};
        uint32_t power{0};
    };

    struct ChrRacesEntry
    {
        uint32_t raceId = 0;
        uint32_t flags = 0;
        uint32_t factionId = 0;
        uint32_t modelMale = 0;
        uint32_t modelFemale = 0;
        uint32_t teamId = 0;
        uint32_t startTaxiMask = 0;
        uint32_t cinematicId = 0;
        std::string name;
        uint32_t expansion = 0;
    };

    struct CreatureDisplayInfoEntry
    {
        uint32_t id{0};
        uint32_t modelId{0};
        uint32_t extendedDisplayInfoId{0};
        float creatureModelScale{0.0f};
    };

    struct CreatureDisplayInfoExtraEntry
    {
        uint32_t displayExtraId{0};
        uint32_t race{0};
        uint32_t displaySexId{0};
    };

    struct CreatureFamilyEntry
    {
        uint32_t id{0};
        float minSize{0.0f};
        uint32_t minLevel{0};
        float maxSize{0.0f};
        uint32_t maxLevel{0};
        uint32_t skillLine{0};
        uint32_t tameable{0};
        uint32_t petDietFlags{0};
        int32_t talentTree{-1};
        std::string name;
    };

    struct CreatureModelDataEntry
    {
        uint32_t id{0};
        uint32_t flags{0};
        std::string modelName;
        float modelScale{1.0f};
        float collisionHeight{0.0f};
        float mountHeight{0.0f};

        [[nodiscard]] constexpr bool hasFlag(CreatureModelDataFlags flag) const noexcept
        {
            return (flags & flag) != 0;
        }
    };

    struct CreatureSpellDataEntry
    {
        uint32_t id{0};
        std::array<uint32_t, 4> spells{0, 0, 0, 0};
        std::array<uint32_t, 4> cooldowns{0, 0, 0, 0};
    };

    // the criteria of achievements from 5.4 on: the leaves of the criteria trees
    struct CriteriaEntry
    {
        uint32_t id{0};
        uint32_t type{0};
        int32_t asset{0};
        uint32_t startEvent{0};
        int32_t startAsset{0};
        uint32_t startTimer{0};
        uint32_t failEvent{0};
        int32_t failAsset{0};
        uint32_t modifierTreeId{0};
        uint32_t flags{0};
        uint32_t eligibilityWorldStateId{0};
        int32_t eligibilityWorldStateValue{0};
    };

    struct CriteriaTreeEntry
    {
        uint32_t id{0};
        uint32_t criteriaId{0};
        uint32_t amount{0};
        uint32_t operatorType{0};
        uint32_t parent{0};
        uint32_t flags{0};
    };

    struct CurrencyTypesEntry
    {
        uint32_t item_id{0};                                        // the currency, in 3.x the item of the currency
        uint32_t bit_index{0};                                      // 3.x: A bit in the known currencies (1 << (index - 1))
        uint32_t Category{0};                                       // 4.x and later
        std::string name;
        uint32_t TotalCap{0};
        uint32_t WeekCap{0};
        uint32_t Flags{0};
    };

    struct DurabilityCostsEntry
    {
        uint32_t itemLevel{0};
        std::array<uint32_t, 29> modifier{0};
    };

    struct DurabilityQualityEntry
    {
        uint32_t id{0};
        float qualityModifier{0.0f};
    };

    struct EmotesTextEntry
    {
        uint32_t id{0};
        std::array<uint32_t, 6> textId{0, 0, 0, 0, 0, 0};
    };

    struct FactionEntry
    {
        uint32_t id{0};
        int32_t reputationIndex{-1};
        uint32_t reputationRaceMask[4]{};
        uint32_t reputationClassMask[4]{};
        int32_t reputationBase[4]{};
        uint32_t reputationFlags[4]{};
        uint32_t parentFactionId{0};
        float spilloverRateIn{0.0f};
        float spilloverRateOut{0.0f};
        uint32_t spilloverMaxIn{0};
        uint32_t expansion{0};
        std::string name;

        [[nodiscard]] bool canHaveReputation() const
        {
            return reputationIndex >= 0;
        }
    };

    inline constexpr std::size_t maxFactionRelations = 4;

    struct FactionTemplateEntry
    {
        uint32_t id{0};
        uint32_t faction{0};
        uint32_t factionFlags{0};
        uint32_t ourMask{0};
        uint32_t friendlyMask{0};
        uint32_t hostileMask{0};
        uint32_t enemyFaction[maxFactionRelations]{};
        uint32_t friendFaction[maxFactionRelations]{};

        [[nodiscard]] bool isFriendlyTo(FactionTemplateEntry const& entry) const
        {
            if (entry.faction != 0)
            {
                for (std::size_t i = 0; i < maxFactionRelations; ++i)
                    if (enemyFaction[i] == entry.faction)
                        return false;

                for (std::size_t i = 0; i < maxFactionRelations; ++i)
                    if (friendFaction[i] == entry.faction)
                        return true;
            }
            return (friendlyMask & entry.ourMask) != 0 || (ourMask & entry.friendlyMask) != 0;
        }

        [[nodiscard]] bool isHostileTo(FactionTemplateEntry const& entry) const
        {
            if (entry.faction != 0)
            {
                for (std::size_t i = 0; i < maxFactionRelations; ++i)
                    if (enemyFaction[i] == entry.faction)
                        return true;

                for (std::size_t i = 0; i < maxFactionRelations; ++i)
                    if (friendFaction[i] == entry.faction)
                        return false;
            }
            return (hostileMask & entry.ourMask) != 0;
        }

        [[nodiscard]] bool isHostileToPlayers() const
        {
            return (hostileMask & FACTION_MASK_PLAYER) != 0;
        }

        [[nodiscard]] bool isNeutralToAll() const
        {
            for (std::size_t i = 0; i < maxFactionRelations; ++i)
                if (enemyFaction[i] != 0)
                    return false;

            return hostileMask == 0 && friendlyMask == 0;
        }

        [[nodiscard]] bool isContestedGuardFaction() const
        {
            return (factionFlags & FACTION_TEMPLATE_FLAG_CONTESTED_GUARD) != 0;
        }
    };

    // the reputation steps of a friendship faction
    struct FriendshipRepReactionEntry
    {
        uint32_t id{0};
        uint32_t friendshipRepId{0};
        uint32_t reactionThreshold{0};
    };

    struct FriendshipReputationEntry
    {
        uint32_t id{0};
        uint32_t factionId{0};
    };

    struct GameObjectDisplayInfoEntry
    {
        uint32_t id = 0;
        char const* filename = nullptr;
        std::array<float, 3> geoBoxMin{};
        std::array<float, 3> geoBoxMax{};

        [[nodiscard]] float minX() const noexcept { return geoBoxMin[0]; }
        [[nodiscard]] float minY() const noexcept { return geoBoxMin[1]; }
        [[nodiscard]] float minZ() const noexcept { return geoBoxMin[2]; }
        [[nodiscard]] float maxX() const noexcept { return geoBoxMax[0]; }
        [[nodiscard]] float maxY() const noexcept { return geoBoxMax[1]; }
        [[nodiscard]] float maxZ() const noexcept { return geoBoxMax[2]; }
    };

    struct GemPropertiesEntry
    {
        uint32_t id{0};
        uint32_t enchantmentId{0};
        uint32_t socketMask{0};
    };

    constexpr uint32_t MAX_HOLIDAY_DURATIONS = 10;
    constexpr uint32_t MAX_HOLIDAY_DATES = 26;
    constexpr uint32_t MAX_HOLIDAY_FLAGS = 10;

    struct HolidaysEntry
    {
        uint32_t id{0};
        std::array<uint32_t, MAX_HOLIDAY_DURATIONS> duration{};
        std::array<uint32_t, MAX_HOLIDAY_DATES> date{};
        uint32_t region{0};
        uint32_t looping{0};
        std::array<uint32_t, MAX_HOLIDAY_FLAGS> calendarFlags{};
        std::string textureFilename;
        uint32_t priority{0};
        uint32_t calendarFilterType{0};
    };

    struct ItemDisplayInfoEntry
    {
        uint32_t id{0};
        uint32_t randomPropertyChance{0};
    };

    inline constexpr std::size_t MAX_ITEM_SET_ITEMS = 10;
    inline constexpr std::size_t MAX_ITEM_SET_SPELLS = 8;

    struct ItemEntry
    {
        uint32_t id{0};
        uint32_t itemClass{0};
        uint32_t itemSubClass{0};
        int32_t soundOverrideSubclass{0};
        int32_t material{0};
        uint32_t displayId{0};
        uint32_t inventoryType{0};
        uint32_t sheath{0};
    };

    struct ItemExtendedCostEntry
    {
        uint32_t id{0};
        uint32_t honorPoints{0};
        uint32_t arenaPoints{0};
        uint32_t arenaSlot{0};
        std::array<uint32_t, 5> item{};
        std::array<uint32_t, 5> count{};
        uint32_t personalRating{0};
        std::array<uint32_t, 5> reqCur{};
        std::array<uint32_t, 5> reqCurrCount{};
    };

    struct ItemLimitCategoryEntry
    {
        uint32_t id{0};
        uint32_t maxAmount{0};
        uint32_t equippedFlag{0};
    };

    struct ItemRandomSuffixEntry
    {
        uint32_t id{0};
        std::string nameSuffix;
        std::array<uint32_t, 5> enchantments{};
        std::array<uint32_t, 5> prefixes{};
    };

    struct ItemReforgeEntry
    {
        uint32_t id{0};
        uint32_t sourceStat{0};
        float sourceMultiplier{0.0f};
        uint32_t finalStat{0};
        float finalMultiplier{0.0f};
    };

    struct ItemSetEntry
    {
        uint32_t id = 0;
        char const* name = nullptr;
        std::array<uint32_t, MAX_ITEM_SET_ITEMS> itemId{};
        std::array<uint32_t, MAX_ITEM_SET_SPELLS> spellId{};
        std::array<uint32_t, MAX_ITEM_SET_SPELLS> itemsCount{};
        uint32_t requiredSkillId = 0;
        uint32_t requiredSkillAmt = 0;
    };

    struct MailTemplateEntry
    {
        uint32_t id{0};
        std::string subject{};
        std::string content{};
    };

    struct MapDifficulty
    {
        uint32_t resetTime{0};
        uint32_t maxPlayers{0};
        bool hasErrorMessage{false};

        MapDifficulty() = default;

        MapDifficulty(uint32_t resetTime_, uint32_t maxPlayers_, bool hasErrorMessage_)
            : resetTime(resetTime_), maxPlayers(maxPlayers_), hasErrorMessage(hasErrorMessage_) {}
    };

    struct MapDifficultyEntry
    {
        uint32_t id{0};
        uint32_t mapId{0};
        uint32_t difficulty{0};
        std::string message;
        uint32_t raidDuration{0};
        uint32_t maxPlayers{0};
    };

    struct MapEntry
    {
        uint32_t id{0};
        uint32_t mapType{0};
        std::string mapName;
        uint32_t linkedZone{0};
        uint32_t multimapId{0};
        int32_t parentMap{-1};
        float startX{0.0f};
        float startY{0.0f};
        uint32_t resetRaidTime{0};
        uint32_t resetHeroicTime{0};
        uint32_t addon{0};
        uint32_t unkTime{0};
        uint32_t maxPlayers{0};
        uint32_t nextPhaseMap{0};

        [[nodiscard]] bool isDungeon() const { return mapType == MAP_DUNGEON; }
        [[nodiscard]] bool isRaid() const { return mapType == MAP_RAID; }
        [[nodiscard]] bool isBattleground() const { return mapType == MAP_BATTLEGROUND; }
        [[nodiscard]] bool isArena() const { return mapType == MAP_ARENA; }
        [[nodiscard]] bool isWorldMap() const { return mapType == MAP_WORLD; }

        [[nodiscard]] bool isInstanceMap() const { return isDungeon() || isRaid(); }
        [[nodiscard]] bool isBattlegroundOrArena() const { return isBattleground() || isArena(); }
        [[nodiscard]] bool isInstanceableMap() const { return isInstanceMap() || isBattlegroundOrArena(); }

        [[nodiscard]] bool isContinent() const
        {
            return id == 0 || id == 1 || id == 530 || id == 571;
        }

        [[nodiscard]] bool getEntrancePos(int32_t& mapid, float& x, float& y) const
        {
            if (parentMap < 0)
                return false;
            mapid = parentMap;
            x = startX;
            y = startY;
            return true;
        }

        [[nodiscard]] uint32_t getAddon() const { return addon; }
        [[nodiscard]] uint32_t getResetTimeNormal() const { return resetRaidTime; }
        [[nodiscard]] uint32_t getResetTimeHeroic() const { return resetHeroicTime; }
    };

    struct ModifierTreeEntry
    {
        uint32_t id{0};
        uint32_t type{0};
        int32_t asset{0};
        int32_t secondaryAsset{0};
        uint32_t operatorType{0};
        uint32_t amount{0};
        uint32_t parent{0};
    };

    // the 136 columns of the 5.2 to 5.4 rows, the failure text at column 132 skipped
    struct PlayerConditionEntry
    {
        uint32_t id{0};
        std::array<int32_t, 134> values{};
    };

    struct SkillLineEntry
    {
        uint32_t id{0};
        uint32_t type{0};
        std::array<std::string, 16> name{};
        uint32_t spellIcon{0};
        uint32_t linkable{0};
    };

    struct SpellAuraOptionsEntry
    {
        uint32_t id{0};
        uint32_t maxStackAmount{0};
        uint32_t procChance{0};
        uint32_t procCharges{0};
        uint32_t procFlags{0};
    };

#define MAX_ITEM_ENCHANTMENT_EFFECTS 3

    struct SpellItemEnchantmentEntry
    {
        uint32_t id{0};
        std::array<uint32_t, MAX_ITEM_ENCHANTMENT_EFFECTS> type{};
        std::array<uint32_t, MAX_ITEM_ENCHANTMENT_EFFECTS> min{};
        std::array<uint32_t, MAX_ITEM_ENCHANTMENT_EFFECTS> max{};
        std::array<uint32_t, MAX_ITEM_ENCHANTMENT_EFFECTS> spell{};
        std::string name;
        uint32_t visual{0};
        uint32_t enchantGroups{0};
        uint32_t gemEntry{0};
        uint32_t enchantCondition{0};
        uint32_t reqSkill{0};
        uint32_t reqSkillValue{0};
        uint32_t reqLevel{0};
    };

    struct SpellMiscEntry
    {
        uint32_t id{0};
        uint32_t spellDifficultyId{0};
        uint32_t attributes{0};
        uint32_t attributesEx{0};
        uint32_t attributesExB{0};
        uint32_t attributesExC{0};
        uint32_t attributesExD{0};
        uint32_t attributesExE{0};
        uint32_t attributesExF{0};
        uint32_t attributesExG{0};
        uint32_t attributesExH{0};
        uint32_t attributesExI{0};
        uint32_t attributesExJ{0};
        uint32_t attributesExK{0};
        uint32_t attributesExL{0};
        uint32_t attributesExM{0};
        uint32_t castingTimeIndex{0};
        uint32_t durationIndex{0};
        int32_t powerType{0};
        uint32_t rangeIndex{0};
        float speed{0.0f};
        uint32_t spellVisual{0};
        uint32_t spellVisual1{0};
        uint32_t spellIconId{0};
        uint32_t activeIconId{0};
        uint32_t school{0};
    };

    struct SpellPowerEntry
    {
        uint32_t id{0};
        uint32_t spellId{0};
        uint32_t powerType{0};
        uint32_t manaCost{0};
        uint32_t manaCostPerLevel{0};
        uint32_t manaCostPercentage{0};
        uint32_t manaPerSecond{0};
        uint32_t manaPerSecondPerLevel{0};
        float manaCostPercentageFloat{0.0f};
        float channelCostPercentageFloat{0.0f};
        uint32_t shapeShiftSpellId{0};
    };

    struct SpellRangeEntry
    {
        uint32_t id{0};
        float minRange{0.0f};
        float minRangeFriendly{0.0f};
        float maxRange{0.0f};
        float maxRangeFriendly{0.0f};
        uint32_t rangeType{0};
    };

    struct SpellShapeshiftFormEntry
    {
        uint32_t id{0};
        uint32_t flags{0};
        uint32_t unitType{0};
        uint32_t attackSpeed{0};
        uint32_t modelId{0};
        uint32_t modelId2{0};
        std::array<uint32_t, 8> spells{};
    };

    // TODO: Modernize the SpellEntry::GetSpell*() / Spell*Entry structs
    // SpellAuraRestrictions.dbc
    struct SpellAuraRestrictionsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t CasterAuraState;                                   // 1
        uint32_t TargetAuraState;                                   // 2
        uint32_t CasterAuraStateNot;                                // 3
        uint32_t TargetAuraStateNot;                                // 4
        uint32_t casterAuraSpell;                                   // 5
        uint32_t targetAuraSpell;                                   // 6
        uint32_t CasterAuraSpellNot;                                // 7
        uint32_t TargetAuraSpellNot;                                // 8
    };

    // SpellCastingRequirements.dbc
    struct SpellCastingRequirementsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t FacingCasterFlags;                                 // 1
        //uint32_t MinFactionId;                                    // 2
        //uint32_t MinReputation;                                   // 3
        int32_t AreaGroupId;                                        // 4
        //uint32_t RequiredAuraVision;                              // 5
        uint32_t RequiresSpellFocus;                                // 6
    };

    // SpellCategories.dbc
    struct SpellCategoriesEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t Category;                                          // 1
        uint32_t DmgClass;                                          // 2
        uint32_t DispelType;                                        // 3
        uint32_t MechanicsType;                                     // 4
        uint32_t PreventionType;                                    // 5
        uint32_t StartRecoveryCategory;                             // 6
    };

    // SpellClassOptions.dbc
    struct SpellClassOptionsEntry
    {
        //uint32_t Id;                                              // 0
        //uint32_t modalNextSpell;                                  // 1
        uint32_t SpellFamilyFlags[MAX_SPELL_EFFECTS];               // 2 - 4
        uint32_t SpellFamilyName;                                   // 5
        //char* Description;                                        // 6
    };

    // SpellCooldowns.dbc
    struct SpellCooldownsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t CategoryRecoveryTime;                              // 1
        uint32_t RecoveryTime;                                      // 2
        uint32_t StartRecoveryTime;                                 // 3
    };

    // SpellEffect.dbc
    struct SpellEffectEntry
    {
        uint32_t id;                                                // 0
        //uint32_t unk;                                             // 1
        uint32_t Effect;                                            // 2
        float EffectMultipleValue;                                  // 3
        uint32_t EffectApplyAuraName;                               // 4
        uint32_t EffectAmplitude;                                   // 5
        int32_t EffectBasePoints;                                   // 6
        float EffectSpellPowerCoefficient;                          // 7
        float EffectDamageMultiplier;                               // 8
        uint32_t EffectChainTarget;                                 // 9
        int32_t EffectDieSides;                                     // 10
        uint32_t EffectItemType;                                    // 11
        uint32_t EffectMechanic;                                    // 12
        int32_t EffectMiscValue;                                    // 13
        int32_t EffectMiscValueB;                                   // 14
        float EffectPointsPerComboPoint;                            // 15
        uint32_t EffectRadiusIndex;                                 // 16
        uint32_t EffectRadiusMaxIndex;                              // 17
        float EffectRealPointsPerLevel;                             // 18
#if VERSION_STRING == Cata
        uint32_t EffectSpellClassMask[3];                           // 18-20
#else
        uint32_t EffectSpellClassMask[4];                           // 19-22
#endif
        //ClassFamilyMask EffectSpellClassMask;
        uint32_t EffectTriggerSpell;                                // 23
        uint32_t EffectImplicitTargetA;                             // 25
        uint32_t EffectImplicitTargetB;                             // 26
        uint32_t EffectSpellId;                                     // 27
        uint32_t EffectIndex;                                       // 28
        //uint32_t unk;                                             // 29

        uint32_t GetRadiusIndex() const
        {
            if (EffectRadiusIndex != 0)
                return EffectRadiusIndex;

            return EffectRadiusMaxIndex;
        }
    };

    // SpellEquippedItems.dbc
    struct SpellEquippedItemsEntry
    {
        //uint32_t Id;                                              // 0
        int32_t EquippedItemClass;                                  // 1
        int32_t EquippedItemInventoryTypeMask;                      // 2
        int32_t EquippedItemSubClassMask;                           // 3
    };

    // SpellFocusObject.dbc
    struct SpellFocusObjectEntry
    {
        uint32_t ID;                                                // 0
        //char* Name;                                               // 1
    };

    // SpellInterrupts.dbc
    struct SpellInterruptsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t AuraInterruptFlags;                                // 1
        //uint32_t unk2                                             // 2
        uint32_t ChannelInterruptFlags;                             // 3
        //uint32_t unk4                                             // 4
        uint32_t InterruptFlags;                                    // 5
    };

    // SpellItemEnchantmentCondition.dbc
    struct SpellItemEnchantmentConditionEntry
    {
        uint32_t ID;                                                // 0
        uint8_t Color[5];                                           // 1-5
        //uint32_t LT_Operand[5];                                   // 6-10
        uint8_t Comparator[5];                                      // 11-15
        uint8_t CompareColor[5];                                    // 15-20
        uint32_t Value[5];                                          // 21-25
        //uint8_t Logic[5]                                          // 25-30
    };

    // SpellLevels.dbc
    struct SpellLevelsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t baseLevel;                                         // 1
        uint32_t maxLevel;                                          // 2
        uint32_t spellLevel;                                        // 3
    };

    struct SpellReagentsEntry
    {
        //uint32_t Id;                                              // 0
        int32_t Reagent[MAX_SPELL_REAGENTS];                        // 54-61
#if VERSION_STRING == Cata
        uint32_t ReagentCount[MAX_SPELL_REAGENTS];                  // 62-69
#else
        uint32_t ReagentCount[10];                                  // 62-69
#endif
    };

    // SpellScaling.dbc
    struct SpellScalingEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t castTimeMin;                                       // 1
        uint32_t castTimeMax;                                       // 2
        uint32_t castScalingMaxLevel;                               // 3
        uint32_t playerClass;                                       // 4
#if VERSION_STRING == Cata
        float coeff1[3];                                            // 5-7
        float coeff2[3];                                            // 8-10
        float coeff3[3];                                            // 11-13
#endif
        float coefBase;                                             // 14
        int32_t coefLevelBase;                                      // 15

        bool IsScalableEffect(uint8_t /*i*/) const { return coefBase != 0.0f; }
    };

    // SpellShapeshift.dbc
    struct SpellShapeshiftEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t ShapeshiftsExcluded;                               // 1
        //uint32_t ShapeshiftsExcluded1;                            // 2 unused, all zeros
        uint32_t Shapeshifts;                                       // 3
        //uint32_t Shapeshifts1;                                    // 4 unused, all zeros
        //uint32_t StanceBarOrder;                                  // 5
    };

    // SpellTargetRestrictions.dbc
    struct SpellTargetRestrictionsEntry
    {
        uint32_t Id;                                                // 0
        float MaxTargetRadius;                                      // 1
        uint32_t MaxAffectedTargets;                                // 2
        uint32_t MaxTargetLevel;                                    // 3
        uint32_t TargetCreatureType;                                // 4
        uint32_t Targets;                                           // 5
    };

    // SpellTotems.dbc
    struct SpellTotemsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t TotemCategory[MAX_SPELL_TOTEM_CATEGORIES];         // 1 2
        uint32_t Totem[MAX_SPELL_TOTEMS];                           // 3 4
    };

    struct SpellCastTimesEntry
    {
        uint32_t ID;                                                // 0
        uint32_t CastTime;                                          // 1
        float CastTimePerLevel;                                     // 2
        int32_t MinCastTime;                                        // 3
    };

    struct SpellEntry
    {
        uint32_t id{0};
        std::array<std::string, 16> name{};
        std::array<std::string, 16> rank{};

        uint32_t school{0};
        uint32_t category{0};
        uint32_t dispelType{0};
        uint32_t mechanicsType{0};
        uint32_t attributes{0};
        uint32_t attributesEx{0};
        uint32_t attributesExB{0};
        uint32_t attributesExC{0};
        uint32_t attributesExD{0};
        uint32_t attributesExE{0};
        uint32_t attributesExF{0};
        uint32_t attributesExG{0};
        uint32_t attributesExH{0};
        uint32_t attributesExI{0};
        uint32_t attributesExJ{0};
        uint32_t shapeshifts{0};
        uint32_t shapeshiftsExcluded{0};
        uint32_t targets{0};
        uint32_t targetCreatureType{0};
        uint32_t requiresSpellFocus{0};
        uint32_t facingCasterFlags{0};
        uint32_t casterAuraState{0};
        uint32_t targetAuraState{0};
        uint32_t casterAuraStateNot{0};
        uint32_t targetAuraStateNot{0};
        uint32_t casterAuraSpell{0};
        uint32_t targetAuraSpell{0};
        uint32_t casterAuraSpellNot{0};
        uint32_t targetAuraSpellNot{0};
        uint32_t castingTimeIndex{0};
        uint32_t recoveryTime{0};
        uint32_t categoryRecoveryTime{0};
        uint32_t interruptFlags{0};
        uint32_t auraInterruptFlags{0};
        uint32_t channelInterruptFlags{0};
        uint32_t procFlags{0};
        uint32_t procChance{0};
        uint32_t procCharges{0};
        uint32_t maxLevel{0};
        uint32_t baseLevel{0};
        uint32_t spellLevel{0};
        uint32_t durationIndex{0};
        int32_t powerType{0};
        uint32_t manaCost{0};
        uint32_t manaCostPerLevel{0};
        uint32_t manaPerSecond{0};
        uint32_t manaPerSecondPerLevel{0};
        uint32_t rangeIndex{0};
        float speed{0.0f};
        uint32_t maxStackAmount{0};

        std::array<uint32_t, MAX_SPELL_TOTEMS> totem{};
        std::array<int32_t, MAX_SPELL_REAGENTS> reagent{};
        std::array<uint32_t, MAX_SPELL_REAGENTS> reagentCount{};
        int32_t equippedItemClass{0};
        int32_t equippedItemSubClass{0};
        int32_t equippedItemInventoryTypeMask{0};

        std::array<uint32_t, MAX_SPELL_EFFECTS> effect{};
        std::array<int32_t, MAX_SPELL_EFFECTS> effectDieSides{};
        std::array<uint32_t, MAX_SPELL_EFFECTS> effectBaseDice{};
        std::array<float, MAX_SPELL_EFFECTS> effectDicePerLevel{};
        std::array<float, MAX_SPELL_EFFECTS> effectRealPointsPerLevel{};
        std::array<int32_t, MAX_SPELL_EFFECTS> effectBasePoints{};
        std::array<uint32_t, MAX_SPELL_EFFECTS> effectMechanic{};
        std::array<uint32_t, MAX_SPELL_EFFECTS> effectImplicitTargetA{};
        std::array<uint32_t, MAX_SPELL_EFFECTS> effectImplicitTargetB{};
        std::array<uint32_t, MAX_SPELL_EFFECTS> effectRadiusIndex{};
        std::array<uint32_t, MAX_SPELL_EFFECTS> effectApplyAuraName{};
        std::array<uint32_t, MAX_SPELL_EFFECTS> effectAmplitude{};
        std::array<float, MAX_SPELL_EFFECTS> effectMultipleValue{};
        std::array<uint32_t, MAX_SPELL_EFFECTS> effectChainTarget{};
        std::array<uint32_t, MAX_SPELL_EFFECTS> effectItemType{};
        std::array<int32_t, MAX_SPELL_EFFECTS> effectMiscValue{};
        std::array<int32_t, MAX_SPELL_EFFECTS> effectMiscValueB{};
        std::array<uint32_t, MAX_SPELL_EFFECTS> effectTriggerSpell{};
        std::array<float, MAX_SPELL_EFFECTS> effectPointsPerComboPoint{};
        std::array<std::array<uint32_t, 3>, MAX_SPELL_EFFECTS> effectSpellClassMask{};
        std::array<float, MAX_SPELL_EFFECTS> effectDamageMultiplier{};
        std::array<float, MAX_SPELL_EFFECTS> effectSpellPowerCoefficient{};

        uint32_t spellVisual{0};
        uint32_t spellVisual1{0};
        uint32_t spellIconId{0};
        uint32_t activeIconId{0};
        uint32_t spellPriority{0};
        uint32_t manaCostPercentage{0};
        uint32_t startRecoveryCategory{0};
        uint32_t startRecoveryTime{0};
        uint32_t maxTargetLevel{0};
        uint32_t spellFamilyName{0};
        std::array<uint32_t, 3> spellFamilyFlags{};
        uint32_t maxTargets{0};
        uint32_t dmgClass{0};
        uint32_t preventionType{0};
        std::array<uint32_t, MAX_SPELL_TOTEM_CATEGORIES> totemCategory{};
        int32_t areaGroupId{0};
        uint32_t runeCostId{0};
        uint32_t spellDifficultyId{0};
        float attackPowerCoefficient{0.0f};

        // Cata+ normalized sub-table references
        uint32_t spellScalingId{0};
        uint32_t spellAuraOptionsId{0};
        uint32_t spellAuraRestrictionsId{0};
        uint32_t spellCastingRequirementsId{0};
        uint32_t spellCategoriesId{0};
        uint32_t spellClassOptionsId{0};
        uint32_t spellCooldownsId{0};
        uint32_t spellEquippedItemsId{0};
        uint32_t spellInterruptsId{0};
        uint32_t spellLevelsId{0};
        uint32_t spellPowerId{0};
        uint32_t spellReagentsId{0};
        uint32_t spellShapeshiftId{0};
        uint32_t spellTargetRestrictionsId{0};
        uint32_t spellTotemsId{0};
        uint32_t researchProject{0};
        uint32_t spellMiscId{0};

        // struct access functions
        SpellAuraOptionsEntry const* GetSpellAuraOptions() const;
        SpellAuraRestrictionsEntry const* GetSpellAuraRestrictions() const;
        SpellCastingRequirementsEntry const* GetSpellCastingRequirements() const;
        SpellCategoriesEntry const* GetSpellCategories() const;
        SpellClassOptionsEntry const* GetSpellClassOptions() const;
        SpellCooldownsEntry const* GetSpellCooldowns() const;
        SpellEffectEntry const* GetSpellEffect(uint8_t eff) const;
        SpellEquippedItemsEntry const* GetSpellEquippedItems() const;
        SpellInterruptsEntry const* GetSpellInterrupts() const;
        SpellLevelsEntry const* GetSpellLevels() const;
        SpellPowerEntry const* GetSpellPower() const;
        SpellReagentsEntry const* GetSpellReagents() const;
        SpellScalingEntry const* GetSpellScaling() const;
        SpellShapeshiftEntry const* GetSpellShapeshift() const;
        SpellTargetRestrictionsEntry const* GetSpellTargetRestrictions() const;
        SpellTotemsEntry const* GetSpellTotems() const;
        SpellMiscEntry const* GetSpellMisc() const;

        // single fields
        uint32_t GetManaCost() const;
        uint32_t GetPreventionType() const;
        uint32_t GetCategory() const;
        uint32_t GetStartRecoveryTime() const;
        uint32_t GetMechanic() const;
        uint32_t GetRecoveryTime() const;
        uint32_t GetCategoryRecoveryTime() const;
        uint32_t GetStartRecoveryCategory() const;
        uint32_t GetSpellLevel() const;
        int32_t GetEquippedItemClass() const;
        uint32_t GetSpellFamilyName() const;
        uint32_t GetDmgClass() const;
        uint32_t GetDispel() const;
        uint32_t GetMaxAffectedTargets() const;
        uint32_t GetStackAmount() const;
        uint32_t GetManaCostPercentage() const;
        uint32_t GetProcCharges() const;
        uint32_t GetProcChance() const;
        uint32_t GetMaxLevel() const;
        uint32_t GetTargetAuraState() const;
        uint32_t GetManaPerSecond() const;
        uint32_t GetRequiresSpellFocus() const;
        uint32_t GetSpellEffectIdByIndex(uint8_t index) const;
        uint32_t GetAuraInterruptFlags() const;
        uint32_t GetEffectImplicitTargetAByIndex(uint8_t index) const;
        int32_t GetAreaGroupId() const;
        uint32_t GetFacingCasterFlags() const;
        uint32_t GetBaseLevel() const;
        uint32_t GetInterruptFlags() const;
        uint32_t GetTargetCreatureType() const;
        int32_t GetEffectMiscValue(uint8_t index) const;
        uint32_t GetStances() const;
        uint32_t GetStancesNot() const;
        uint32_t GetProcFlags() const;
        uint32_t GetChannelInterruptFlags() const;
        uint32_t GetManaCostPerLevel() const;
        uint32_t GetCasterAuraState() const;
        uint32_t GetTargets() const;
        uint32_t GetEffectApplyAuraNameByIndex(uint8_t index) const;
    };

    using SpellCategorySet = std::set<uint32_t>;
    using SpellCategoryStore = std::map<uint32_t, SpellCategorySet>;

    struct SpellEffect
    {
        std::array<SpellEffectEntry const*, 32> effects{};
    };

    using SpellEffectMap = std::map<uint32_t, SpellEffect>;

    struct StableSlotPricesEntry
    {
        uint32_t id{0};
        uint32_t price{0};
    };

    struct TalentEntry
    {
        uint32_t id{0};
        uint32_t talentTree{0};
        uint32_t row{0};
        uint32_t col{0};
        std::array<uint32_t, 5> rankId{};
        uint32_t dependsOn{0};
        uint32_t dependsOnRank{0};
        uint32_t spellId{0};
        uint32_t playerClass{0};
        uint32_t overrideSpellId{0};
    };

    struct TaxiNodesEntry
    {
        uint32_t id{0};
        uint32_t mapId{0};
        float x{0.0f};
        float y{0.0f};
        float z{0.0f};
        std::string name;
        std::array<uint32_t, 2> mountCreatureId{};
        uint32_t flags{0};
    };

    struct TaxiPathNodeEntry
    {
        uint32_t id{0};
        uint32_t pathId{0};
        uint32_t nodeIndex{0};
        uint32_t mapId{0};
        float x{0.0f};
        float y{0.0f};
        float z{0.0f};
        uint32_t flags{0};
        uint32_t waitTime{0};
        uint32_t arrivalEventId{0};
        uint32_t departureEventId{0};
    };

    struct TotemCategoryEntry
    {
        uint32_t id{0};
        uint32_t categoryType{0};
        uint32_t categoryMask{0};
    };

    struct WorldMapAreaEntry
    {
        uint32_t mapId{0};
        uint32_t zoneId{0};
        int32_t continentMapId{-1};
    };

    // the byte code of an expression over world states, written as hex text
    struct WorldStateExpressionEntry
    {
        uint32_t id{0};
        std::string expression;
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    // END OF MODERNIZED STRUCTURES

#if VERSION_STRING <= Classic
#define NAME_PATTERN 8
#else
#if VERSION_STRING < Cata
#define NAME_PATTERN 16
#else
#define NAME_PATTERN 1
#endif
#endif

#pragma pack(push, 1)
    struct BattlePetSpeciesEntry
    {
        uint32_t id;                                                // 0
        uint32_t creatureId;                                        // 1
        //uint32_t iconFileId;                                      // 2
        uint32_t summonSpellId;                                     // 3
        uint32_t petType;                                           // 4
        //int32_t unknown;                                          // 5
        uint32_t flags;                                             // 6
        //char* description;                                       // 7
        //char* sourceText;                                         // 8
    };

    // the battle pet journal: species, the stat states of breeds and species, the quality multipliers
    struct BattlePetBreedStateEntry
    {
        uint32_t id;                                                // 0
        uint32_t breedId;                                           // 1
        uint32_t stateId;                                           // 2
        int32_t value;                                              // 3
    };

    struct BattlePetSpeciesStateEntry
    {
        uint32_t id;                                                // 0
        uint32_t speciesId;                                         // 1
        uint32_t stateId;                                           // 2
        int32_t value;                                              // 3
    };

    struct BattlePetBreedQualityEntry
    {
        uint32_t id;                                                // 0
        uint32_t quality;                                           // 1
        float multiplier;                                           // 2
    };

    // 5.x: the item that teaches a species
    struct ItemToBattlePetEntry
    {
        uint32_t itemId;                                            // 0
        uint32_t speciesId;                                         // 1
    };

    struct DungeonEncounterEntry
    {
        uint32_t id;                                                // 0 unique id
        uint32_t mapId;                                             // 1 map id
        uint32_t difficulty;                                        // 2 instance mode
        //uint32_t unk0;                                            // 3
        uint32_t encounterIndex;                                    // 4 encounter index for creating completed mask
        char* encounterName[NAME_PATTERN];                          // 5 encounter name
        //uint32_t nameFlags;                                       // 21
        //uint32_t unk1;                                            // 22
    };

    struct EmotesEntry
    {
        uint32_t Id;                                                // 0
        //char* name;                                               // 1
        //uint32_t animationId;                                     // 2
        uint32_t Flags;                                             // 3
        uint32_t EmoteType;                                         // 4
        uint32_t UnitStandState;                                    // 5
        //uint32_t soundId;                                         // 6
        //uint32_t unk;                                             // 7
    };

    struct GlyphPropertiesEntry
    {
        uint32_t Entry;                                             // 0
        uint32_t SpellID;                                           // 1
        uint32_t Type;                                              // 2 (0 = Major, 1 = Minor)
        uint32_t unk;                                               // 3 glyph_icon spell.dbc
    };

    struct GlyphSlotEntry
    {
        uint32_t Id;                                                // 0
        uint32_t Type;                                              // 1
        uint32_t Slot;                                              // 2
    };

    struct GtBarberShopCostBaseEntry
    {
        float cost;                                                 // 0 cost base
    };

    struct GtChanceToMeleeCritEntry
    {
        float val;                                                  // 0
    };

    struct GtChanceToMeleeCritBaseEntry
    {
        float val;                                                  // 0
    };

    struct GtChanceToSpellCritEntry
    {
        float val;                                                  // 0
    };

    struct GtChanceToSpellCritBaseEntry
    {
        float val;                                                  // 0
    };

    struct GtCombatRatingsEntry
    {
        float val;                                                  // 0
    };

#if VERSION_STRING < Cata
    struct GtOCTRegenHPEntry
    {
        float ratio;                                                // 0
    };
#endif

    struct GtOCTRegenMPEntry
    {
        float ratio;                                                // 0
    };

#if VERSION_STRING < Cata
    struct GtRegenHPPerSptEntry
    {
        float ratio;                                                // 0 regen base
    };
#endif

    struct GtRegenMPPerSptEntry
    {
        float ratio;                                                // 0 regen base
    };

    struct GtOCTBaseHPByClassEntry
    {
        float ratio;
    };

    struct GtOCTBaseMPByClassEntry
    {
        float ratio;
    };

    struct GtOCTClassCombatRatingScalarEntry
    {
        float val;                                                  // 0
    };

    struct GuildPerkSpellsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t Level;                                             // 1
        uint32_t SpellId;                                           // 2
    };

    struct ItemCurrencyCostEntry
    {
        //uint32_t id;                                              // 0
        uint32_t itemid;                                            // 1
    };

    struct ItemRandomPropertiesEntry
    {
        uint32_t ID;                                                // 0
        //uint32_t name1;                                           // 1
        uint32_t spells[MAX_ITEM_ENCHANTMENT_EFFECTS];              // 2-4
        //uint32_t unk1;                                            // 5
        //uint32_t unk2;                                            // 6
        char* name_suffix[NAME_PATTERN];                            // 7-22
        //uint32_t name_suffix_flags;                               // 23
    };

    struct LFGDungeonEntry
    {
        uint32_t ID;                                                // 0
        char* name[NAME_PATTERN];                                   // 1-17 Name lang
        uint32_t minlevel;                                          // 18
        uint32_t maxlevel;                                          // 19
        uint32_t reclevel;                                          // 20
        uint32_t recminlevel;                                       // 21
        uint32_t recmaxlevel;                                       // 22
        int32_t map;                                                // 23
        uint32_t difficulty;                                        // 24
        uint32_t flags;                                             // 25
        uint32_t type;                                              // 26
        //uint32_t  unk;                                            // 27
        //char* iconname;                                           // 28
        uint32_t expansion;                                         // 29
        //uint32_t unk4;                                            // 30
        uint32_t grouptype;                                         // 31
        //char* desc[16];                                           // 32-47 Description

        // Helpers
        uint32_t Entry() const { return ID + (type << 24); }
    };

    struct LiquidTypeEntry
    {
        uint32_t Id;                                                // 0
        //uint32_t liquid_id;                                       // 1
        uint32_t Type;                                              // 2
        uint32_t SpellId;                                           // 3
    };

#define LOCK_NUM_CASES 8

    struct LockEntry
    {
        uint32_t Id;                                                // 0
        uint32_t locktype[LOCK_NUM_CASES];                          // 1-8 If this is 1, then the next lockmisc is an item ID, if it's 2, then it's an iRef to LockTypes.dbc.
        uint32_t lockmisc[LOCK_NUM_CASES];                          // 9-16 Item to unlock or iRef to LockTypes.dbc depending on the locktype.
        uint32_t minlockskill[LOCK_NUM_CASES];                      // 17-24 Required skill needed for lockmisc (if locktype = 2).
        //uint32_t action[LOCK_NUM_CASES];                          // 25-32 Something to do with direction / opening / closing.
    };

    struct MountCapabilityEntry
    {
        uint32_t  id;                                               // 0 index
        uint32_t  flag;                                             // 1 some flag
        uint32_t  reqRidingSkill;                                   // 2 skill level of riding required
        uint32_t  reqArea;                                          // 3 required Area
        uint32_t  reqAura;                                          // 4 required Aura
        uint32_t  reqSpell;                                         // 5 spell that has to be known to you
        uint32_t  speedModSpell;                                    // 6 spell to cast to apply mount speed effects
        uint32_t  reqMap;                                           // 7 map where this is applicable
    };

#define MAX_MOUNT_CAPABILITIES 24

    struct MountTypeEntry
    {
        uint32_t id;                                                // 0 index
        uint32_t capabilities[MAX_MOUNT_CAPABILITIES];              // 1-17 capability ids from MountCapability.dbc
        //uint32_t  empty[7];                                       // 18-24 empty. maybe continues capabilities
    };

    struct NameGenEntry
    {
        uint32_t ID;                                                // 0
        char* Name;                                                 // 1
        uint32_t unk1;                                              // 2
        uint32_t type;                                              // 3
    };

    struct NumTalentsAtLevel
    {
        //uint32_t level;                                           // 0
        float talentPoints;                                         // 1
    };

    struct PhaseEntry
    {
        uint32_t Id;                                                // 0
        uint32_t PhaseShift;                                        // 1
        uint32_t Flags;                                             // 2
    };

    struct QuestSortEntry
    {
        uint32_t id;                                                // 0
        //char* name;                                               // 1
    };

    struct QuestXP
    {
        uint32_t questLevel;                                        // 0
        uint32_t xpIndex[10];                                       // 1-10
    };

    struct ScalingStatDistributionEntry
    {
        uint32_t id;                                                // 0
        int32_t stat[10];                                           // 1-10
        uint32_t statmodifier[10];                                  // 11-20
        uint32_t maxlevel;                                          // 21
    };

    struct ScalingStatValuesEntry
    {
        uint32_t id;                                                // 0
        uint32_t level;                                             // 1
#if VERSION_STRING == Cata
        uint32_t multiplier[20];
#elif VERSION_STRING >= Mop
        uint32_t multiplier[47];
#else
        uint32_t shoulderBudget;                                    // 2
        uint32_t trinketBudget;                                     // 3
        uint32_t weaponBudget1H;                                    // 4
        uint32_t rangedBudget;                                      // 5
        uint32_t clothShoulderArmor;                                // 6
        uint32_t leatherShoulderArmor;                              // 7
        uint32_t mailShoulderArmor;                                 // 8
        uint32_t plateShoulderArmor;                                // 9
        uint32_t weaponDPS1H;                                       // 10
        uint32_t weaponDPS2H;                                       // 11
        uint32_t spellcasterDPS1H;                                  // 12
        uint32_t spellcasterDPS2H;                                  // 13
        uint32_t rangedDPS;                                         // 14
        uint32_t wandDPS;                                           // 15
        uint32_t spellPower;                                        // 16
        uint32_t primaryBudget;                                     // 17
        uint32_t tertiaryBudget;                                    // 18
        uint32_t clothCloakArmor;                                   // 19
        uint32_t clothChestArmor;                                   // 20
        uint32_t leatherChestArmor;                                 // 21
        uint32_t mailChestArmor;                                    // 22
        uint32_t plateChestArmor;                                   // 23

        uint32_t getScalingStatDistributionMultiplier(uint32_t mask) const
        {
            if (mask & 0x4001F)
            {
                if (mask & 0x00000001) return shoulderBudget;
                if (mask & 0x00000002) return trinketBudget;
                if (mask & 0x00000004) return weaponBudget1H;
                if (mask & 0x00000008) return primaryBudget;
                if (mask & 0x00000010) return rangedBudget;
                if (mask & 0x00040000) return tertiaryBudget;
            }
            return 0;
        }

        uint32_t getArmorMod(uint32_t mask) const
        {
            if (mask & 0x00F001E0)
            {
                if (mask & 0x00000020) return clothShoulderArmor;
                if (mask & 0x00000040) return leatherShoulderArmor;
                if (mask & 0x00000080) return mailShoulderArmor;
                if (mask & 0x00000100) return plateShoulderArmor;

                if (mask & 0x00080000) return clothCloakArmor;
                if (mask & 0x00100000) return clothChestArmor;
                if (mask & 0x00200000) return leatherChestArmor;
                if (mask & 0x00400000) return mailChestArmor;
                if (mask & 0x00800000) return plateChestArmor;
            }
            return 0;
        }

        uint32_t getDPSMod(uint32_t mask) const
        {
            if (mask & 0x7E00)
            {
                if (mask & 0x00000200) return weaponDPS1H;
                if (mask & 0x00000400) return weaponDPS2H;
                if (mask & 0x00000800) return spellcasterDPS1H;
                if (mask & 0x00001000) return spellcasterDPS2H;
                if (mask & 0x00002000) return rangedDPS;
                if (mask & 0x00004000) return wandDPS;
            }
            return 0;
        }

        uint32_t getSpellBonus(uint32_t mask) const
        {
            if (mask & 0x00008000) return spellPower;
            return 0;
        }
#endif
    };

    struct SkillLineAbilityEntry
    {
        uint32_t Id;                                                // 0
        uint32_t skilline;                                          // 1 skill id
        uint32_t spell;                                             // 2
        uint32_t race_mask;                                         // 3
        uint32_t class_mask;                                        // 4
        //uint32_t excludeRace;                                     // 5
        //uint32_t excludeClass;                                    // 6
        uint32_t minSkillLineRank;                                  // 7 req skill value
        uint32_t next;                                              // 8
        uint32_t acquireMethod;                                     // 9 auto learn
        uint32_t grey;                                              // 10 max
        uint32_t green;                                             // 11 min
        //uint32_t abandonable;                                     // 12
        //uint32_t reqTP;                                           // 13
        //uint32_t reqtrainpoints;                                  // 14
    };

    struct SpellDifficultyEntry
    {
        uint32_t ID;                                                // 0
        int32_t SpellId[InstanceDifficulty::MAX_DIFFICULTY];        // 1-4
    };

    struct SpellRuneCostEntry
    {
        uint32_t ID;                                                // 0
        uint32_t bloodRuneCost;                                     // 1
        uint32_t frostRuneCost;                                     // 2
        uint32_t unholyRuneCost;                                    // 3
#if VERSION_STRING >= Mop
        uint32_t deathRuneCost;
#endif
        uint32_t runePowerGain;                                     // 4
    };

    struct SpellDurationEntry
    {
        uint32_t ID;                                                // 0
        int32_t Duration1;                                          // 1
        int32_t Duration2;                                          // 2
        int32_t Duration3;                                          // 3
    };

    struct SpellRadiusEntry
    {
        uint32_t ID;                                                // 0
        float radius_min;                                           // 1 Radius
        float radius_per_level;                                     // 2
        float radius_max;                                           // 3 Radius2
    };

    struct SummonPropertiesEntry
    {
        uint32_t ID;                                                // 0
        uint32_t ControlType;                                       // 1
        uint32_t FactionID;                                         // 2
        uint32_t Type;                                              // 3
        uint32_t Slot;                                              // 4
        uint32_t Flags;                                             // 5
    };

    struct TalentTabEntry
    {
        uint32_t TalentTabID;                                       // 0
        //char* Name[16];                                           // 1-16
        //uint32_t name_flags;                                      // 17
        //uint32_t unk4;                                            // 18
        //uint32_t unk5;                                            // 19
        uint32_t ClassMask;                                         // 20
#if VERSION_STRING >= WotLK
        uint32_t PetTalentMask;                                     // 21
#endif
        uint32_t TabPage;                                           // 21
        //char* InternalName;                                       // 22
#if VERSION_STRING >= Cata
        uint32_t rolesMask;                                         // 8
        uint32_t masterySpells[2];                                  // 9-10
#endif
    };

    struct ChrSpecializationEntry
    {
        uint32_t Id;                                                // 0
        uint32_t classId;                                           // 2
        uint32_t masterySpellId;                                    // 3
        uint32_t tabPage;                                           // 5
        uint32_t petTabPage;                                        // 6
    };

    struct TalentTreePrimarySpells
    {
        uint32_t ID;                                                // 0
        uint32_t tabID;                                             // 1
        uint32_t SpellID;                                           // 2
        //uint32_t unk                                              // 3
    };

    struct TaxiPathEntry
    {
        uint32_t id;                                                // 0
        uint32_t from;                                              // 1
        uint32_t to;                                                // 2
        uint32_t price;                                             // 3
    };

    struct TransportAnimationEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t TransportID;                                       // 1
        uint32_t TimeIndex;                                         // 2
        DBCPosition3D Pos;                                          // 3
        //uint32_t SequenceID;                                      // 4
    };

    struct TransportRotationEntry
    {
        //uint32_t ID;                                              // 0
        uint32_t GameObjectsID;                                     // 1
        uint32_t TimeIndex;                                         // 2
        float X;                                                    // 3
        float Y;                                                    // 4
        float Z;                                                    // 5
        float W;                                                    // 6
    };

#define MAX_VEHICLE_SEATS 8

    struct VehicleEntry
    {
        uint32_t ID;                                                // 0
        uint32_t flags;                                             // 1
        float turnSpeed;                                            // 2
        float pitchSpeed;                                           // 3
        float pitchMin;                                             // 4
        float pitchMax;                                             // 5
        uint32_t seatID[MAX_VEHICLE_SEATS];                         // 6-13
        float mouseLookOffsetPitch;                                 // 14
        float cameraFadeDistScalarMin;                              // 15
        float cameraFadeDistScalarMax;                              // 16
        float cameraPitchOffset;                                    // 17
        float facingLimitRight;                                     // 18
        float facingLimitLeft;                                      // 19
        float msslTrgtTurnLingering;                                // 20
        float msslTrgtPitchLingering;                               // 21
        float msslTrgtMouseLingering;                               // 22
        float msslTrgtEndOpacity;                                   // 23
        float msslTrgtArcSpeed;                                     // 24
        float msslTrgtArcRepeat;                                    // 25
        float msslTrgtArcWidth;                                     // 26
        float msslTrgtImpactRadius[2];                              // 27-28
        char* msslTrgtArcTexture;                                   // 29
        char* msslTrgtImpactTexture;                                // 30
        char* msslTrgtImpactModel[2];                               // 31-32
        float cameraYawOffset;                                      // 33
        uint32_t uiLocomotionType;                                  // 34
        float msslTrgtImpactTexRadius;                              // 35
        uint32_t uiSeatIndicatorType;                               // 36
        uint32_t powerType;                                         // 37, new in 3.1
        //uint32_t unk1;                                            // 38
        //uint32_t unk2;                                            // 39
    };

    enum VehicleSeatFlags
    {
        VEHICLE_SEAT_FLAG_HAS_LOWER_ANIM_FOR_ENTER = 0x00000001,
        VEHICLE_SEAT_FLAG_HAS_LOWER_ANIM_FOR_RIDE = 0x00000002,
        VEHICLE_SEAT_FLAG_UNK3 = 0x00000004,
        VEHICLE_SEAT_FLAG_SHOULD_USE_VEH_SEAT_EXIT_ANIM_ON_VOLUNTARY_EXIT = 0x00000008,
        VEHICLE_SEAT_FLAG_UNK5 = 0x00000010,
        VEHICLE_SEAT_FLAG_UNK6 = 0x00000020,
        VEHICLE_SEAT_FLAG_UNK7 = 0x00000040,
        VEHICLE_SEAT_FLAG_UNK8 = 0x00000080,
        VEHICLE_SEAT_FLAG_UNK9 = 0x00000100,
        VEHICLE_SEAT_FLAG_HIDE_PASSENGER = 0x00000200,              // Passenger is hidden
        VEHICLE_SEAT_FLAG_ALLOW_TURNING = 0x00000400,
        VEHICLE_SEAT_FLAG_CAN_CONTROL = 0x00000800,                 // Lua_UnitInVehicleControlSeat
        VEHICLE_SEAT_FLAG_CAN_CAST_MOUNT_SPELL = 0x00001000,        // Can cast spells with SPELL_AURA_MOUNTED from seat (possibly 4.x only, 0 seats on 3.3.5a)
        VEHICLE_SEAT_FLAG_UNCONTROLLED = 0x00002000,                // can override !& VEHICLE_SEAT_FLAG_CAN_ENTER_OR_EXIT
        VEHICLE_SEAT_FLAG_CAN_ATTACK = 0x00004000,                  // Can attack, cast spells and use items from vehicle?
        VEHICLE_SEAT_FLAG_SHOULD_USE_VEH_SEAT_EXIT_ANIM_ON_FORCED_EXIT = 0x00008000,
        VEHICLE_SEAT_FLAG_UNK17 = 0x00010000,
        VEHICLE_SEAT_FLAG_UNK18 = 0x00020000,                       // Needs research and support (28 vehicles): Allow entering vehicles while keeping specific permanent(?) auras that impose visuals (states like beeing under freeze/stun mechanic, emote state animations).
        VEHICLE_SEAT_FLAG_HAS_VEH_EXIT_ANIM_VOLUNTARY_EXIT = 0x00040000,
        VEHICLE_SEAT_FLAG_HAS_VEH_EXIT_ANIM_FORCED_EXIT = 0x00080000,
        VEHICLE_SEAT_FLAG_PASSENGER_NOT_SELECTABLE = 0x00100000,
        VEHICLE_SEAT_FLAG_UNK22 = 0x00200000,
        VEHICLE_SEAT_FLAG_REC_HAS_VEHICLE_ENTER_ANIM = 0x00400000,
        VEHICLE_SEAT_FLAG_IS_USING_VEHICLE_CONTROLS = 0x00800000,   // Lua_IsUsingVehicleControls
        VEHICLE_SEAT_FLAG_ENABLE_VEHICLE_ZOOM = 0x01000000,
        VEHICLE_SEAT_FLAG_USABLE = 0x02000000,                      // Lua_CanExitVehicle
        VEHICLE_SEAT_FLAG_CAN_SWITCH = 0x04000000,                  // Lua_CanSwitchVehicleSeats
        VEHICLE_SEAT_FLAG_HAS_START_WARITING_FOR_VEH_TRANSITION_ANIM_ENTER = 0x08000000,
        VEHICLE_SEAT_FLAG_HAS_START_WARITING_FOR_VEH_TRANSITION_ANIM_EXIT = 0x10000000,
        VEHICLE_SEAT_FLAG_CAN_CAST = 0x20000000,                    // Lua_UnitHasVehicleUI
        VEHICLE_SEAT_FLAG_UNK2 = 0x40000000,                        // checked in conjunction with 0x800 in CastSpell2
    };

    enum VehicleSeatFlagsB
    {
        VEHICLE_SEAT_FLAG_B_NONE = 0x00000000,
        VEHICLE_SEAT_FLAG_B_USABLE_FORCED = 0x00000002,
        VEHICLE_SEAT_FLAG_B_TARGETS_IN_RAIDUI = 0x00000008,         // Lua_UnitTargetsVehicleInRaidUI
        VEHICLE_SEAT_FLAG_B_EJECTABLE = 0x00000020,                 // ejectable
        VEHICLE_SEAT_FLAG_B_USABLE_FORCED_2 = 0x00000040,
        VEHICLE_SEAT_FLAG_B_USABLE_FORCED_3 = 0x00000100,
        VEHICLE_SEAT_FLAG_B_KEEP_PET = 0x00020000,
        VEHICLE_SEAT_FLAG_B_USABLE_FORCED_4 = 0x02000000,
        VEHICLE_SEAT_FLAG_B_CAN_SWITCH = 0x04000000,
        VEHICLE_SEAT_FLAG_B_VEHICLE_PLAYERFRAME_UI = 0x80000000     // Lua_UnitHasVehiclePlayerFrameUI - actually checked for flagsb &~ 0x80000000
    };

    struct VehicleSeatEntry
    {
        uint32_t ID;                                                // 0
        uint32_t flags;                                             // 1
        int32_t attachmentID;                                       // 2
        float attachmentOffsetX;                                    // 3
        float attachmentOffsetY;                                    // 4
        float attachmentOffsetZ;                                    // 5
        float enterPreDelay;                                        // 6
        float enterSpeed;                                           // 7
        float enterGravity;                                         // 8
        float enterMinDuration;                                     // 9
        float enterMaxDuration;                                     // 10
        float enterMinArcHeight;                                    // 11
        float enterMaxArcHeight;                                    // 12
        int32_t enterAnimStart;                                     // 13
        int32_t enterAnimLoop;                                      // 14
        int32_t rideAnimStart;                                      // 15
        int32_t rideAnimLoop;                                       // 16
        int32_t rideUpperAnimStart;                                 // 17
        int32_t rideUpperAnimLoop;                                  // 18
        float exitPreDelay;                                         // 19
        float exitSpeed;                                            // 20
        float exitGravity;                                          // 21
        float exitMinDuration;                                      // 22
        float exitMaxDuration;                                      // 23
        float exitMinArcHeight;                                     // 24
        float exitMaxArcHeight;                                     // 25
        int32_t exitAnimStart;                                      // 26
        int32_t exitAnimLoop;                                       // 27
        int32_t exitAnimEnd;                                        // 28
        float passengerYaw;                                         // 29
        float passengerPitch;                                       // 30
        float passengerRoll;                                        // 31
        int32_t passengerAttachmentID;                              // 32
        int32_t vehicleEnterAnim;                                   // 33
        int32_t vehicleExitAnim;                                    // 34
        int32_t vehicleRideAnimLoop;                                // 35
        int32_t vehicleEnterAnimBone;                               // 36
        int32_t vehicleExitAnimBone;                                // 37
        int32_t vehicleRideAnimLoopBone;                            // 38
        float vehicleEnterAnimDelay;                                // 39
        float vehicleExitAnimDelay;                                 // 40
        uint32_t vehicleAbilityDisplay;                             // 41
        uint32_t enterUISoundID;                                    // 42
        uint32_t exitUISoundID;                                     // 43
        int32_t uiSkin;                                             // 44
        uint32_t flagsB;                                            // 45

        bool hasFlag(VehicleSeatFlags flag) const { return (flags & flag) != 0; }
        bool hasFlag(VehicleSeatFlagsB flag) const { return (flagsB & flag) != 0; }

        bool IsUsable() const
        {
            if ((flags & VEHICLE_SEAT_FLAG_USABLE) != 0)
                return true;
            return false;
        }

        bool IsController() const
        {
            if ((flags & VEHICLE_SEAT_FLAG_CAN_CONTROL) != 0)
                return true;
            return false;
        }

        bool HidesPassenger() const
        {
            if ((flags & VEHICLE_SEAT_FLAG_HIDE_PASSENGER) != 0)
                return true;
            return false;
        }

        bool canEnterOrExit() const { return hasFlag(VehicleSeatFlags(VEHICLE_SEAT_FLAG_USABLE | VEHICLE_SEAT_FLAG_CAN_CONTROL | VEHICLE_SEAT_FLAG_SHOULD_USE_VEH_SEAT_EXIT_ANIM_ON_VOLUNTARY_EXIT)); }
        bool canSwitchFromSeat() const { return hasFlag(VEHICLE_SEAT_FLAG_CAN_SWITCH); }

        bool isUsableByOverride() const {
            return hasFlag(VehicleSeatFlags(VEHICLE_SEAT_FLAG_UNCONTROLLED | VEHICLE_SEAT_FLAG_UNK18))
                || hasFlag(VehicleSeatFlagsB(VEHICLE_SEAT_FLAG_B_USABLE_FORCED | VEHICLE_SEAT_FLAG_B_USABLE_FORCED_2 |
                    VEHICLE_SEAT_FLAG_B_USABLE_FORCED_3 | VEHICLE_SEAT_FLAG_B_USABLE_FORCED_4));
        }

        bool isEjectable() const { return hasFlag(VEHICLE_SEAT_FLAG_B_EJECTABLE); }
    };

    struct WMOAreaTableEntry
    {
        uint32_t id;                                                // 0
        int32_t rootId;                                             // 1
        int32_t adtId;                                              // 2
        int32_t groupId;                                            // 3
        //uint32_t field4;                                          // 4
        //uint32_t field5;                                          // 5
        //uint32_t field6;                                          // 6
        //uint32_t field7;                                          // 7
        //uint32_t field8;                                          // 8
        uint32_t flags;                                             // 9
        uint32_t areaId;                                            // 10 ref -> AreaTableEntry
        //char Name[16];                                            // 11-26
        //uint32_t nameflags;                                       // 27
    };

    struct WorldMapOverlayEntry
    {
        uint32_t ID;                                                // 0
        //uint32_t worldMapID;                                      // 1
        uint32_t areaID;                                            // 2 - index to AreaTable
        uint32_t areaID_2;                                          // 3 - index to AreaTable
        uint32_t areaID_3;                                          // 4 - index to AreaTable
        uint32_t areaID_4;                                          // 5 - index to AreaTable
        //uint32_t unk1[2];                                         // 6-7
        //uint32_t unk2;                                            // 8
        //uint32_t unk3[7];                                         // 9-16
    };

#pragma pack(pop)

    // SpellPower.dbc is keyed by its own row id on Mop; the spell it belongs to is the
    // spellId column, so like SpellEffect it needs a spell id -> row helper map.
    typedef std::map<uint32_t, SpellPowerEntry const*> SpellPowerMap;
}
