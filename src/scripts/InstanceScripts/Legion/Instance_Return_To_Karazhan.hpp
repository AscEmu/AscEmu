/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const ReturnToKarazhanEncounterCount = 9;

enum ReturnToKarazhanData
{
    DATA_MAIDEN_OF_VIRTUE = 0,  // 1954
    // Wikket, Westfall Story or Beautiful Beast
    DATA_OPERA_HALL       = 1,  // 1957
    DATA_MANA_DEVOURER    = 2,  // 1959
    // Attumen the Huntsman and Midnight
    DATA_ATTUMEN          = 3,  // 1960
    // Moroes and his dinner guests
    DATA_MOROES           = 4,  // 1961
    DATA_CURATOR          = 5,  // 1964
    DATA_SHADE_OF_MEDIVH  = 6,  // 1965
    DATA_VIZADUUM         = 7,  // 2017
    DATA_NIGHTBANE        = 8   // 2031
};

enum ReturnToKarazhanOperaShows : uint32_t
{
    OPERA_SHOW_NONE            = 0,
    OPERA_SHOW_WIKKET          = 1,
    OPERA_SHOW_WESTFALL        = 2,
    OPERA_SHOW_BEAUTIFUL_BEAST = 3
};

enum ReturnToKarazhanCreatures
{
    BOSS_MAIDEN_OF_VIRTUE        = 113971,

    // Opera Hall: Wikket
    BOSS_GALINDRE                = 114251,
    BOSS_ELFYRA                  = 114284,
    NPC_WINGED_ASSISTANT         = 114324,

    // Opera Hall: Westfall Story
    BOSS_TOE_KNEE                = 114261,
    BOSS_MRRGRIA                 = 114260,
    NPC_GANG_RUFFIAN             = 114265,
    NPC_SHORELINE_TIDESPEAKER    = 114266,

    // Opera Hall: Beautiful Beast
    BOSS_COGGLESTON              = 114328,
    BOSS_LUMINORE                = 114329,
    BOSS_BABBLET                 = 114330,
    BOSS_MRS_CAULDRONS           = 114522,

    BOSS_MANA_DEVOURER           = 114252,

    BOSS_MIDNIGHT                = 114264,
    BOSS_ATTUMEN                 = 114262,

    BOSS_MOROES                  = 114312,
    NPC_BARONESS_DOROTHEA        = 114316,
    NPC_LADY_CATRIONA            = 114317,
    NPC_BARON_RAFE               = 114318,
    NPC_LADY_KEIRA               = 114319,
    NPC_LORD_ROBIN               = 114320,
    NPC_LORD_CRISPIN             = 114321,

    BOSS_CURATOR                 = 114247,
    NPC_VOLATILE_ENERGY          = 114249,

    BOSS_SHADE_OF_MEDIVH         = 114350,
    NPC_GUARDIANS_IMAGE          = 114675,

    BOSS_VIZADUUM                = 114790,
    NPC_FELGUARD_SENTRY          = 115730,
    NPC_SHADOW_SPITTER           = 115734,

    BOSS_NIGHTBANE               = 114895,
    NPC_BONECURSE                = 114903
};

enum ReturnToKarazhanGameObjects
{
    GO_OPERA_STAGE_CURTAIN    = 259684,  // open while a show is on stage
    GO_OPERA_STAGE_DOOR_LEFT  = 259694,  // open after the Opera Hall
    GO_OPERA_STAGE_DOOR_RIGHT = 259702,  // closed during the show
    GO_OPERA_BALLROOM_DOOR    = 259730,  // open after the Opera Hall
    GO_MOROES_DOOR_1          = 269977,  // closed while Moroes is fought
    GO_MOROES_DOOR_2          = 269978,
    GO_CURATOR_WALL           = 266508,  // open after The Curator
    GO_MEDIVH_DOOR            = 266846,  // closed while the Shade of Medivh is fought
    GO_MEDIVH_BOOKCASE        = 266510,  // open after the Shade of Medivh
    GO_VIZADUUM_TOWER_DOOR    = 266593   // closed while Viz'aduum is fought
};

enum ReturnToKarazhanActions
{
    ACTION_OPERA_RESERVE_JOINS = 1,
    ACTION_TOE_KNEE_RETURNS,
    ACTION_BEAST_CAST_FALLEN,
    ACTION_ATTUMEN_SLAIN,
    ACTION_ATTUMEN_REMOUNT
};

enum ReturnToKarazhanMaidenSpells
{
    SPELL_MAIDEN_HOLY_BOLT       = 227809,
    SPELL_MAIDEN_SACRED_GROUND   = 227789,
    SPELL_MAIDEN_HOLY_SHOCK      = 227800,
    SPELL_MAIDEN_MASS_REPENTANCE = 227508,
    SPELL_MAIDEN_HOLY_BULWARK    = 227817,
    SPELL_MAIDEN_HOLY_WRATH      = 227823
};

enum ReturnToKarazhanWikketSpells
{
    SPELL_GALINDRE_FLASHY_BOLT        = 227341,
    SPELL_GALINDRE_WONDROUS_RADIANCE  = 227410,
    SPELL_GALINDRE_MAGNIFICENT_SHIELD = 227346,
    SPELL_GALINDRE_MAGIC_MAGNIFICENT  = 227776,

    SPELL_ELFYRA_DREARY_BOLT          = 227543,
    SPELL_ELFYRA_SUMMON_ASSISTANTS    = 227477,
    SPELL_ELFYRA_DEFY_GRAVITY         = 227447
};

enum ReturnToKarazhanWestfallSpells
{
    SPELL_TOE_KNEE_BURNING_LEG_SWEEP  = 227568,
    SPELL_TOE_KNEE_DASHING_FLAME_GALE = 227449,
    SPELL_TOE_KNEE_FLAME_GALE_CHARGE  = 227453,

    SPELL_MRRGRIA_THUNDER_RITUAL      = 227777,
    SPELL_MRRGRIA_WASH_AWAY           = 227783,

    SPELL_RUFFIAN_POISONOUS_SHANK     = 227325,
    SPELL_TIDESPEAKER_BUBBLE_BLAST    = 227420
};

enum ReturnToKarazhanBeautifulBeastSpells
{
    SPELL_BEAST_EMINENCE              = 228729,

    SPELL_COGGLESTON_SPECTRAL_SERVICE = 232156,
    SPELL_COGGLESTON_DENT_ARMOR       = 227985,
    SPELL_COGGLESTON_DINNER_BELL      = 227987,
    SPELL_COGGLESTON_KARA_KAZHAM      = 232153,

    SPELL_LUMINORE_CANDLE_HAT         = 227497,
    SPELL_LUMINORE_BURNING_BLAZE      = 228193,
    SPELL_LUMINORE_HEAT_WAVE          = 228025,

    SPELL_BABBLET_SEVERE_DUSTING      = 228213,
    SPELL_BABBLET_DUSTING_FIXATE      = 228221,

    SPELL_CAULDRONS_SOUP_SPRAY        = 228011,
    SPELL_CAULDRONS_LEFTOVERS         = 228019
};

enum ReturnToKarazhanManaDevourerSpells
{
    SPELL_DEVOURER_ARCANE_BOMB        = 227618,
    SPELL_DEVOURER_ENERGY_VOID        = 227523,
    SPELL_DEVOURER_ENERGY_DISCHARGE   = 227457,
    SPELL_DEVOURER_COALESCE_POWER     = 227297,
    SPELL_DEVOURER_LOOSE_MANA         = 228066,
    SPELL_DEVOURER_DECIMATING_ESSENCE = 227507
};

enum ReturnToKarazhanAttumenSpells
{
    SPELL_MIDNIGHT_INTANGIBLE_PRESENCE = 227404,
    SPELL_MIDNIGHT_MIGHTY_STOMP        = 227363,
    SPELL_MIDNIGHT_RIDERLESS           = 227338,
    SPELL_MIDNIGHT_MEZAIR              = 227339,
    SPELL_MIDNIGHT_ENRAGE              = 228895,

    SPELL_ATTUMEN_MORTAL_STRIKE        = 227493,
    SPELL_ATTUMEN_SHARED_SUFFERING     = 228852
};

enum ReturnToKarazhanMoroesSpells
{
    SPELL_MOROES_VANISH           = 227736,
    SPELL_MOROES_GARROTE          = 227742,
    SPELL_MOROES_COAT_CHECK       = 227851,
    SPELL_MOROES_GHASTLY_PURGE    = 227872,

    SPELL_GUEST_DINNER_PARTY      = 233669,
    SPELL_DOROTHEA_ARCANE_BLAST   = 227575,
    SPELL_DOROTHEA_MANA_DRAIN     = 227545,
    SPELL_CATRIONA_SMITE          = 227542,
    SPELL_CATRIONA_HEALING_STREAM = 227578,
    SPELL_RAFE_IRON_WHIRLWIND     = 227637,
    SPELL_KEIRA_EMPOWERED_ARMS    = 227616,
    SPELL_ROBIN_WHIRLING_EDGE     = 227463,
    SPELL_CRISPIN_WILL_BREAKER    = 227672
};

enum ReturnToKarazhanCuratorSpells
{
    SPELL_CURATOR_POWER_DISCHARGE       = 227278,
    SPELL_CURATOR_EVOCATION             = 227254,
    SPELL_CURATOR_OVERLOAD              = 227257,

    SPELL_VOLATILE_ENERGY_PASSIVE       = 227268,
    SPELL_VOLATILE_ENERGY_STATIC_CHARGE = 228735
};

enum ReturnToKarazhanShadeSpells
{
    SPELL_SHADE_FROSTBITE         = 227592,
    SPELL_SHADE_INFERNO_BOLT      = 227615,
    SPELL_SHADE_PIERCING_MISSILES = 227628,
    SPELL_SHADE_CEASELESS_WINTER  = 227779,
    SPELL_SHADE_FLAME_WREATH      = 228269,
    SPELL_SHADE_DISSOLVE          = 228572,

    SPELL_IMAGE_SHIMMER           = 228521,
    SPELL_IMAGE_ARCANE_BOLT       = 228991
};

enum ReturnToKarazhanVizaduumSpells
{
    SPELL_VIZADUUM_BURNING_BLAST        = 229083,
    SPELL_VIZADUUM_COMMAND_FEL_BEAM     = 229242,
    SPELL_VIZADUUM_COMMAND_BOMBARDMENT  = 229284,
    SPELL_VIZADUUM_DISINTEGRATE         = 229151,
    SPELL_VIZADUUM_CHAOTIC_SHADOWS      = 229159,
    SPELL_VIZADUUM_STABILIZE_RIFT       = 230084,

    SPELL_FELGUARD_LEGION_SPAWN         = 230280,
    SPELL_SPITTER_SHADOW_PHLEGM         = 230066
};

enum ReturnToKarazhanNightbaneSpells
{
    SPELL_NIGHTBANE_CHARRED_EARTH         = 228863,
    SPELL_NIGHTBANE_CINDER_BREATH         = 228785,
    SPELL_NIGHTBANE_TAIL_SWIPE            = 228786,
    SPELL_NIGHTBANE_REVERBERATING_SHADOWS = 229307,
    SPELL_NIGHTBANE_BURNING_BONES         = 228829,
    SPELL_NIGHTBANE_IGNITE_SOUL           = 228796,
    SPELL_NIGHTBANE_CONCENTRATED_POWER    = 228790,
    SPELL_NIGHTBANE_INFERNAL_POWER        = 228792,
    SPELL_NIGHTBANE_RAIN_OF_BONES         = 228839,
    SPELL_NIGHTBANE_BELLOWING_ROAR        = 228837
};
