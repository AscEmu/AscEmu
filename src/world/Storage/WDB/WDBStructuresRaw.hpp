/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace WDB::Structures::Raw
{
    inline constexpr size_t namePatternClassic = 8;
    inline constexpr size_t namePatternTbcWotlk = 16;

#pragma pack(push, 1)
    struct AchievementEntryWotlk
    {
        uint32_t id;
        int32_t factionFlag;
        int32_t mapId;
        uint32_t parentAchievement;
        char* name[namePatternTbcWotlk];
        char* description[namePatternTbcWotlk];
        uint32_t categoryId;
        uint32_t points;
        uint32_t orderInCategory;
        uint32_t flags;
        uint32_t icon;
        char* rewardName[namePatternTbcWotlk];
        uint32_t count;
        uint32_t refAchievement;
    };

    struct AchievementEntryCata
    {
        uint32_t id;
        int32_t factionFlag;
        int32_t mapId;
        uint32_t parentAchievement;
        char* name;
        char* description;
        uint32_t categoryId;
        uint32_t points;
        uint32_t orderInCategory;
        uint32_t flags;
        uint32_t icon;
        char* rewardName;
        uint32_t count;
        uint32_t refAchievement;
    };

    struct AchievementEntryMop
    {
        uint32_t id;
        int32_t factionFlag;
        int32_t mapId;
        uint32_t parentAchievement;
        char* name;
        char* description;
        uint32_t categoryId;
        uint32_t points;
        uint32_t orderInCategory;
        uint32_t flags;
        uint32_t icon;
        char* rewardName;
        uint32_t count;
        uint32_t refAchievement;
        uint32_t criteriaTreeId;
    };

    struct AchievementCategoryEntryMop
    {
        uint32_t id;
        int32_t parent;
    };

    struct AchievementCriteriaEntryWotlk
    {
        uint32_t id;
        uint32_t achievementId;
        uint32_t type;
        uint32_t asset;
        uint32_t quantity;
        uint32_t startEvent;
        uint32_t startAsset;
        uint32_t failEvent;
        uint32_t failAsset;
        char* name[namePatternTbcWotlk];
        uint32_t flags;
        uint32_t timerStartEvent;
        uint32_t timerAsset;
        uint32_t timerTime;
        uint32_t order;
    };

    // the quantity is 64 bit from 4.x on, its high half is skipped
    struct AchievementCriteriaEntryCata
    {
        uint32_t id;
        uint32_t achievementId;
        uint32_t type;
        uint32_t asset;
        uint32_t quantity;
        uint32_t startEvent;
        uint32_t startAsset;
        uint32_t failEvent;
        uint32_t failAsset;
        char* name;
        uint32_t flags;
        uint32_t timerStartEvent;
        uint32_t timerAsset;
        uint32_t timerTime;
        uint32_t order;
        uint32_t worldStateId;
        int32_t worldStateValue;
        uint32_t conditionType[3];
        uint32_t conditionValue[3];
    };

    using AchievementCriteriaEntryMop = AchievementCriteriaEntryCata;

    struct AreaGroupEntryWotlkCataMop
    {
        uint32_t id;
        uint32_t areaId[6];
        uint32_t nextGroup;
    };

    struct AreaTableEntryClassic
    {
        uint32_t id;
        uint32_t map_id;
        uint32_t zone;
        uint32_t explore_flag;
        uint32_t flags;
        int32_t area_level;
        char* area_name[namePatternClassic];
        uint32_t team;
        uint32_t liquid_type_override;
    };

    struct AreaTableEntryTbc
    {
        uint32_t id;
        uint32_t map_id;
        uint32_t zone;
        uint32_t explore_flag;
        uint32_t flags;
        int32_t area_level;
        char* area_name[namePatternTbcWotlk];
        uint32_t team;
        uint32_t liquid_type_override[4];
    };

    struct AreaTableEntryWotlk
    {
        uint32_t id;
        uint32_t map_id;
        uint32_t zone;
        uint32_t explore_flag;
        uint32_t flags;
        int32_t area_level;
        char* area_name[namePatternTbcWotlk];
        uint32_t team;
        uint32_t liquid_type_override[4];
    };

    struct AreaTableEntryCata
    {
        uint32_t id;
        uint32_t map_id;
        uint32_t zone;
        uint32_t explore_flag;
        uint32_t flags;
        uint32_t sound_provider_pref;
        uint32_t sound_provider_pref_underwater;
        uint32_t ambience_id;
        uint32_t zone_music;
        uint32_t intro_sound;
        int32_t area_level;
        char* area_name;
        uint32_t team;
        uint32_t liquid_type_override[4];
        float min_elevation;
        float ambient_multiplier;
        uint32_t light_id;
        uint32_t mount_flags;
        uint32_t uw_intro_sound;
        uint32_t uw_zone_music;
        uint32_t uw_ambience;
        uint32_t world_pvp_id;
        int32_t pvp_combat_world_state_id;
    };

    struct AreaTableEntryMop
    {
        uint32_t id;
        uint32_t map_id;
        uint32_t zone;
        uint32_t explore_flag;
        uint32_t flags;
        int32_t area_level;
        char* area_name;
        uint32_t team;
        uint32_t liquid_type_override[4];
        float elevation;
    };

    struct AreaTriggerEntryClassic
    {
        uint32_t id;
        uint32_t mapId;
        float x;
        float y;
        float z;
        float boxRadius;
        float boxX;
        float boxY;
        float boxZ;
        float boxOrientation;
    };

    using AreaTriggerEntryTbc = AreaTriggerEntryClassic;
    using AreaTriggerEntryWotlk = AreaTriggerEntryClassic;
    using AreaTriggerEntryCata = AreaTriggerEntryClassic;
    using AreaTriggerEntryMop = AreaTriggerEntryClassic;

    struct AuctionHouseEntryClassic
    {
        uint32_t id;
        uint32_t faction;
        uint32_t fee;
        uint32_t tax;
    };

    using AuctionHouseEntryTbc = AuctionHouseEntryClassic;
    using AuctionHouseEntryWotlk = AuctionHouseEntryClassic;
    using AuctionHouseEntryCata = AuctionHouseEntryClassic;
    using AuctionHouseEntryMop = AuctionHouseEntryClassic;

    struct BankBagSlotPricesEntryClassic
    {
        uint32_t id;
        uint32_t price;
    };

    using BankBagSlotPricesEntryTbc = BankBagSlotPricesEntryClassic;
    using BankBagSlotPricesEntryWotlk = BankBagSlotPricesEntryClassic;
    using BankBagSlotPricesEntryCata = BankBagSlotPricesEntryClassic;
    using BankBagSlotPricesEntryMop = BankBagSlotPricesEntryClassic;

    struct BannedAddOnsEntryCata
    {
        uint32_t id;
    };

    using BannedAddOnsEntryMop = BannedAddOnsEntryCata;

    struct BarberShopStyleEntryWotlkCataMop
    {
        uint32_t id;
        uint32_t type;
        uint32_t race;
        uint32_t gender;
        uint32_t hairId;
    };

    struct ChatChannelsEntryClassic
    {
        uint32_t id;
        uint32_t flags;
        char const* namePattern[namePatternClassic];
    };

    struct ChatChannelsEntryTbc
    {
        uint32_t id;
        uint32_t flags;
        char const* namePattern[namePatternTbcWotlk];
    };

    using ChatChannelsEntryWotlk = ChatChannelsEntryTbc;

    struct ChatChannelsEntryCata
    {
        uint32_t id;
        uint32_t flags;
        char const* namePattern;
    };

    using ChatChannelsEntryMop = ChatChannelsEntryCata;

    struct CharStartOutfitEntryClassic
    {
        uint8_t race;
        uint8_t classId;
        uint8_t gender;
        int32_t itemId[12];
    };

    using CharStartOutfitEntryTbc = CharStartOutfitEntryClassic;

    struct CharStartOutfitEntryWotlk
    {
        uint8_t race;
        uint8_t classId;
        uint8_t gender;
        int32_t itemId[24];
    };

    struct CharStartOutfitEntryCata
    {
        uint8_t race;
        uint8_t classId;
        uint8_t gender;
        int32_t itemId[24];
        uint32_t petDisplayId;
        uint32_t petFamilyEntry;
    };

    using CharStartOutfitEntryMop = CharStartOutfitEntryCata;

    struct CharTitlesEntryTbcWotlk
    {
        uint32_t id;
        char* nameMale[namePatternTbcWotlk];
        char* nameFemale[namePatternTbcWotlk];
        uint32_t bitIndex;
    };

    struct CharTitlesEntryCataMop
    {
        uint32_t id;
        char* name;
        uint32_t bitIndex;
    };

    struct ChrClassesEntryClassic
    {
        uint32_t id;
        uint32_t powerType;
        char* name[namePatternClassic];
        uint32_t spellClassSet;
    };

    struct ChrClassesEntryTbc
    {
        uint32_t id;
        uint32_t powerType;
        char* name[namePatternTbcWotlk];
        uint32_t spellClassSet;
    };

    struct ChrClassesEntryWotlk
    {
        uint32_t id;
        uint32_t powerType;
        char* name[namePatternTbcWotlk];
        uint32_t spellClassSet;
        uint32_t cinematicSequenceId;
        uint32_t requiredExpansion;
    };

    struct ChrClassesEntryCata
    {
        uint32_t id;
        uint32_t powerType;
        char* name;
        uint32_t spellClassSet;
        uint32_t cinematicSequenceId;
        uint32_t requiredExpansion;
        uint32_t apPerStr;
        uint32_t apPerAgi;
        uint32_t rapPerAgi;
    };

    struct ChrClassesEntryMop
    {
        uint32_t id;
        uint32_t powerType;
        char* name;
        uint32_t spellClassSet;
        uint32_t cinematicSequenceId;
        uint32_t apPerStr;
        uint32_t apPerAgi;
        uint32_t rapPerAgi;
    };

    struct ChrPowerTypesEntryCata
    {
        uint32_t entry;
        uint32_t classId;
        uint32_t power;
    };

    using ChrPowerTypesEntryMop = ChrPowerTypesEntryCata;

    struct ChrRacesEntryClassic
    {
        uint32_t id;
        uint32_t flags;
        uint32_t faction_id;
        uint32_t model_male;
        uint32_t model_female;
        uint32_t team_id;
        uint32_t start_taxi_mask;
        uint32_t cinematic_id;
        char const* name[namePatternClassic];
    };

    struct ChrRacesEntryTbc
    {
        uint32_t id;
        uint32_t flags;
        uint32_t faction_id;
        uint32_t model_male;
        uint32_t model_female;
        uint32_t team_id;
        uint32_t cinematic_id;
        char const* name[namePatternTbcWotlk];
        uint32_t expansion;
    };

    using ChrRacesEntryWotlk = ChrRacesEntryTbc;

    struct ChrRacesEntryCata
    {
        uint32_t id;
        uint32_t flags;
        uint32_t faction_id;
        uint32_t model_male;
        uint32_t model_female;
        uint32_t team_id;
        uint32_t cinematic_id;
        char const* name;
        uint32_t expansion;
    };

    struct ChrRacesEntryMop
    {
        uint32_t id;
        uint32_t flags;
        uint32_t faction_id;
        uint32_t model_male;
        uint32_t model_female;
        uint32_t team_id;
        uint32_t cinematic_id;
        char const* name;
        uint32_t expansion;
    };

    struct CreatureDisplayInfoEntryClassic
    {
        uint32_t id;
        uint32_t modelId;
        uint32_t extendedDisplayInfoId;
        float creatureModelScale;
    };

    using CreatureDisplayInfoEntryTbc = CreatureDisplayInfoEntryClassic;
    using CreatureDisplayInfoEntryWotlk = CreatureDisplayInfoEntryClassic;
    using CreatureDisplayInfoEntryCata = CreatureDisplayInfoEntryClassic;
    using CreatureDisplayInfoEntryMop = CreatureDisplayInfoEntryClassic;

    struct CreatureDisplayInfoExtraEntryClassic
    {
        uint32_t displayExtraId;
        uint32_t race;
        uint32_t displaySexId;
    };

    using CreatureDisplayInfoExtraEntryTbc = CreatureDisplayInfoExtraEntryClassic;
    using CreatureDisplayInfoExtraEntryWotlk = CreatureDisplayInfoExtraEntryClassic;
    using CreatureDisplayInfoExtraEntryCata = CreatureDisplayInfoExtraEntryClassic;
    using CreatureDisplayInfoExtraEntryMop = CreatureDisplayInfoExtraEntryClassic;

    struct CreatureFamilyEntryClassic
    {
        uint32_t id;
        float minSize;
        uint32_t minLevel;
        float maxSize;
        uint32_t maxLevel;
        uint32_t skillLine;
        uint32_t tameable;
        uint32_t petDietFlags;
        char const* name[namePatternClassic];
    };

    struct CreatureFamilyEntryTbc
    {
        uint32_t id;
        float minSize;
        uint32_t minLevel;
        float maxSize;
        uint32_t maxLevel;
        uint32_t skillLine;
        uint32_t tameable;
        uint32_t petDietFlags;
        char const* name[namePatternTbcWotlk];
    };

    struct CreatureFamilyEntryWotlk
    {
        uint32_t id;
        float minSize;
        uint32_t minLevel;
        float maxSize;
        uint32_t maxLevel;
        uint32_t skillLine;
        uint32_t tameable;
        uint32_t petDietFlags;
        int32_t talentTree;
        char const* name[namePatternTbcWotlk];
    };

    struct CreatureFamilyEntryCata
    {
        uint32_t id;
        float minSize;
        uint32_t minLevel;
        float maxSize;
        uint32_t maxLevel;
        uint32_t skillLine;
        uint32_t tameable;
        uint32_t petDietFlags;
        int32_t talentTree;
        char const* name;
    };

    using CreatureFamilyEntryMop = CreatureFamilyEntryCata;

    struct CreatureModelDataEntryClassic
    {
        uint32_t id;
        uint32_t flags;
        char const* modelName;
        float modelScale;
        float collisionHeight;
    };

    struct CreatureModelDataEntryTbc
    {
        uint32_t id;
        uint32_t flags;
        char const* modelName;
        float modelScale;
        float collisionHeight;
        float mountHeight;
    };

    using CreatureModelDataEntryWotlk = CreatureModelDataEntryTbc;

    struct CreatureModelDataEntryCata
    {
        uint32_t id;
        uint32_t flags;
        char const* modelName;
        float collisionHeight;
        float mountHeight;
    };

    using CreatureModelDataEntryMop = CreatureModelDataEntryCata;

    struct CreatureSpellDataEntryClassic
    {
        uint32_t id;
        uint32_t spells[4];
        uint32_t cooldowns[4];
    };

    using CreatureSpellDataEntryTbc = CreatureSpellDataEntryClassic;
    using CreatureSpellDataEntryWotlk = CreatureSpellDataEntryClassic;
    using CreatureSpellDataEntryCata = CreatureSpellDataEntryClassic;
    using CreatureSpellDataEntryMop = CreatureSpellDataEntryClassic;

    struct DurabilityCostsEntryClassic
    {
        uint32_t itemLevel;
        uint32_t modifier[29];
    };

    using DurabilityCostsEntryTbc = DurabilityCostsEntryClassic;
    using DurabilityCostsEntryWotlk = DurabilityCostsEntryClassic;
    using DurabilityCostsEntryCata = DurabilityCostsEntryClassic;
    using DurabilityCostsEntryMop = DurabilityCostsEntryClassic;

    struct DurabilityQualityEntryClassic
    {
        uint32_t id;
        float qualityModifier;
    };

    using DurabilityQualityEntryTbc = DurabilityQualityEntryClassic;
    using DurabilityQualityEntryWotlk = DurabilityQualityEntryClassic;
    using DurabilityQualityEntryCata = DurabilityQualityEntryClassic;
    using DurabilityQualityEntryMop = DurabilityQualityEntryClassic;

    struct CriteriaEntryMop
    {
        uint32_t id;
        uint32_t type;
        int32_t asset;
        uint32_t startEvent;
        int32_t startAsset;
        uint32_t startTimer;
        uint32_t failEvent;
        int32_t failAsset;
        uint32_t modifierTreeId;
        uint32_t flags;
        uint32_t eligibilityWorldStateId;
        int32_t eligibilityWorldStateValue;
    };

    struct CriteriaTreeEntryMop
    {
        uint32_t id;
        uint32_t criteriaId;
        uint32_t amount;
        uint32_t operatorType;
        uint32_t parent;
        uint32_t flags;
    };

    struct CurrencyTypesEntryWotlk
    {
        uint32_t id;                                                // the item of the currency
        uint32_t bitIndex;
    };

    struct CurrencyTypesEntryCata
    {
        uint32_t id;
        uint32_t category;
        char* name;
        uint32_t totalCap;
        uint32_t weekCap;
        uint32_t flags;
    };

    using CurrencyTypesEntryMop = CurrencyTypesEntryCata;

    struct EmotesTextEntryClassic
    {
        uint32_t id;
        uint32_t textId[6];
    };

    using EmotesTextEntryTbc = EmotesTextEntryClassic;
    using EmotesTextEntryWotlk = EmotesTextEntryClassic;
    using EmotesTextEntryCata = EmotesTextEntryClassic;
    using EmotesTextEntryMop = EmotesTextEntryClassic;

    struct FriendshipRepReactionEntryMop
    {
        uint32_t id;
        uint32_t friendshipRepId;
        uint32_t reactionThreshold;
    };

    struct FriendshipReputationEntryMop
    {
        uint32_t id;
        uint32_t factionId;
    };

    struct ModifierTreeEntryMop
    {
        uint32_t id;
        uint32_t type;
        int32_t asset;
        int32_t secondaryAsset;
        uint32_t operatorType;
        uint32_t amount;
        uint32_t parent;
    };

    // the 136 columns of the 5.2 to 5.4 rows, the failure text at column 132 skipped
    struct PlayerConditionEntryMop
    {
        uint32_t id;
        int32_t values[134];
    };

    struct WorldStateExpressionEntryMop
    {
        uint32_t id;
        char* expression;
    };

    struct FactionEntryClassic
    {
        uint32_t id;
        int32_t reputationIndex;
        uint32_t reputationRaceMask[4];
        uint32_t reputationClassMask[4];
        int32_t reputationBase[4];
        uint32_t reputationFlags[4];
        uint32_t parentFactionId;
        char const* name[namePatternClassic];
    };

    struct FactionEntryTbc
    {
        uint32_t id;
        int32_t reputationIndex;
        uint32_t reputationRaceMask[4];
        uint32_t reputationClassMask[4];
        int32_t reputationBase[4];
        uint32_t reputationFlags[4];
        uint32_t parentFactionId;
        char const* name[namePatternTbcWotlk];
    };

    struct FactionEntryWotlk
    {
        uint32_t id;
        int32_t reputationIndex;
        uint32_t reputationRaceMask[4];
        uint32_t reputationClassMask[4];
        int32_t reputationBase[4];
        uint32_t reputationFlags[4];
        uint32_t parentFactionId;
        float spilloverRateIn;
        float spilloverRateOut;
        uint32_t spilloverMaxIn;
        char const* name[namePatternTbcWotlk];
    };

    struct FactionEntryCata
    {
        uint32_t id;
        int32_t reputationIndex;
        uint32_t reputationRaceMask[4];
        uint32_t reputationClassMask[4];
        int32_t reputationBase[4];
        uint32_t reputationFlags[4];
        uint32_t parentFactionId;
        float spilloverRateIn;
        float spilloverRateOut;
        uint32_t spilloverMaxIn;
        char const* name;
    };

    struct FactionEntryMop
    {
        uint32_t id;
        int32_t reputationIndex;
        uint32_t reputationRaceMask[4];
        uint32_t reputationClassMask[4];
        int32_t reputationBase[4];
        uint32_t reputationFlags[4];
        uint32_t parentFactionId;
        float spilloverRateIn;
        float spilloverRateOut;
        uint32_t spilloverMaxIn;
        char const* name;
        uint32_t expansion;
    };

    struct FactionTemplateEntryAll
    {
        uint32_t id;
        uint32_t faction;
        uint32_t factionFlags;
        uint32_t ourMask;
        uint32_t friendlyMask;
        uint32_t hostileMask;
        uint32_t enemyFaction[4];
        uint32_t friendFaction[4];
    };

    struct GameObjectDisplayInfoEntryClassic
    {
        uint32_t id;
        char const* filename;
    };

    struct GameObjectDisplayInfoEntryTbc
    {
        uint32_t id;
        char const* filename;
        float geoBoxMin[3];
        float geoBoxMax[3];
    };

    using GameObjectDisplayInfoEntryWotlk = GameObjectDisplayInfoEntryTbc;
    using GameObjectDisplayInfoEntryCata = GameObjectDisplayInfoEntryTbc;
    using GameObjectDisplayInfoEntryMop = GameObjectDisplayInfoEntryTbc;

    struct GemPropertiesEntryTbcWotlkCataMop
    {
        uint32_t id;
        uint32_t enchantmentId;
        uint32_t socketMask;
    };

    struct ItemSetEntryClassic
    {
        uint32_t id;
        char const* name[namePatternClassic];
        uint32_t itemId[10];
        uint32_t spellId[8];
        uint32_t itemsCount[8];
        uint32_t requiredSkillId;
        uint32_t requiredSkillAmt;
    };

    struct ItemSetEntryTbc
    {
        uint32_t id;
        char const* name[namePatternTbcWotlk];
        uint32_t itemId[10];
        uint32_t spellId[8];
        uint32_t itemsCount[8];
        uint32_t requiredSkillId;
        uint32_t requiredSkillAmt;
    };

    using ItemSetEntryWotlk = ItemSetEntryTbc;

    struct ItemSetEntryCata
    {
        uint32_t id;
        char const* name;
        uint32_t itemId[10];
        uint32_t spellId[8];
        uint32_t itemsCount[8];
        uint32_t requiredSkillId;
        uint32_t requiredSkillAmt;
    };

    using ItemSetEntryMop = ItemSetEntryCata;

    struct MapDifficultyEntryWotlkCataMop
    {
        uint32_t mapId;
        uint32_t difficulty;
        char const* message;
        uint32_t raidDuration;
        uint32_t maxPlayers;
    };

    struct MapEntryClassic
    {
        uint32_t id;
        uint32_t mapType;
        char const* mapName[namePatternClassic]; // 8
        uint32_t linkedZone;
        uint32_t multimapId;
    };

    struct MapEntryTbc
    {
        uint32_t id;
        uint32_t mapType;
        char const* mapName[namePatternTbcWotlk]; // 16
        uint32_t linkedZone;
        uint32_t multimapId;
        int32_t parentMap;
        float startX;
        float startY;
        uint32_t resetRaidTime;
        uint32_t resetHeroicTime;
        uint32_t addon;
    };

    struct MapEntryWotlk
    {
        uint32_t id;
        uint32_t mapType;
        char const* mapName[namePatternTbcWotlk]; // 16
        uint32_t linkedZone;
        uint32_t multimapId;
        int32_t parentMap;
        float startX;
        float startY;
        uint32_t addon;
        uint32_t unkTime;
        uint32_t maxPlayers;
    };

    struct MapEntryCataMop
    {
        uint32_t id;
        uint32_t mapType;
        char const* mapName;
        uint32_t linkedZone;
        uint32_t multimapId;
        int32_t parentMap;
        float startX;
        float startY;
        uint32_t addon;
        uint32_t unkTime;
        uint32_t maxPlayers;
        uint32_t nextPhaseMap;
    };

    struct StableSlotPricesEntryClassicTbcWotlk
    {
        uint32_t id;
        uint32_t price;
    };

    struct TotemCategoryEntryTbcWotlkCataMop
    {
        uint32_t id;
        uint32_t categoryType;
        uint32_t categoryMask;
    };

    struct WorldMapAreaEntryTbcWotlkCataMop
    {
        uint32_t id;
        uint32_t zoneId;
        int32_t continentMapId;
    };
#pragma pack(pop)
}
