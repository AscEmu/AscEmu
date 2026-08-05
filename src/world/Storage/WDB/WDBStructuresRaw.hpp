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

    struct BarberShopStyleEntryWotlkCataMop
    {
        uint32_t id;
        uint32_t type;
        uint32_t race;
        uint32_t gender;
        uint32_t hairId;
    };

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
        uint32_t base_language;
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
        uint32_t base_language;
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
        uint32_t base_language;
        uint32_t cinematic_id;
        char const* name;
        uint32_t expansion;
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
#pragma pack(pop)
}
