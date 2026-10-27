/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const BlackrockFoundryEncounterCount = 10;

enum BlackrockFoundryData
{
    DATA_FLAMEBENDER_KAGRAZ    = 0,     // 1689
    DATA_BLAST_FURNACE         = 1,     // 1690
    DATA_GRUUL                 = 2,     // 1691
    DATA_OPERATOR_THOGAR       = 3,     // 1692
    DATA_HANSGAR_AND_FRANZOK   = 4,     // 1693
    DATA_BEASTLORD_DARMAC      = 5,     // 1694
    DATA_IRON_MAIDENS          = 6,     // 1695
    DATA_OREGORGER             = 7,     // 1696
    DATA_BLACKHAND             = 8,     // 1704
    DATA_KROMOG                = 9      // 1713
};

enum BlackrockFoundryCreatures
{
    // bosses
    BOSS_GRUUL                       = 76877,
    BOSS_OREGORGER                   = 77182,
    BOSS_BEASTLORD_DARMAC            = 76865,
    BOSS_FLAMEBENDER_KAGRAZ          = 76814,
    BOSS_HANSGAR                     = 76973,
    BOSS_FRANZOK                     = 76974,
    BOSS_OPERATOR_THOGAR             = 76906,
    BOSS_HEART_OF_THE_MOUNTAIN       = 76806,
    BOSS_FOREMAN_FELDSPAR            = 76809,
    BOSS_KROMOG                      = 77692,
    BOSS_ADMIRAL_GARAN               = 77557,
    BOSS_ENFORCER_SORKA              = 77231,
    BOSS_MARAK_THE_BLOODED           = 77477,
    BOSS_BLACKHAND                   = 77325,

    // Gruul
    NPC_CAVE_IN                      = 86596,

    // Beastlord Darmac
    NPC_CRUELFANG                    = 76884,
    NPC_DREADWING                    = 76874,
    NPC_IRONCRUSHER                  = 76945,
    NPC_PACK_BEAST                   = 77128,

    // Flamebender Ka'graz
    NPC_AKNOR_STEELBRINGER           = 77337,
    NPC_CINDER_WOLF                  = 76794,

    // Operator Thogar
    NPC_THOGAR_IRON_RAIDER           = 77394,
    NPC_THOGAR_IRON_CRACK_SHOT       = 77476,
    NPC_THOGAR_GROMKAR_FIREMENDER    = 77487,
    NPC_THOGAR_GROMKAR_MAN_AT_ARMS   = 80791,

    // The Blast Furnace
    NPC_HEAT_REGULATOR               = 76808,
    NPC_PRIMAL_ELEMENTALIST          = 76815,
    NPC_FURNACE_ENGINEER             = 76810,
    NPC_BELLOWS_OPERATOR             = 76811,
    NPC_SECURITY_GUARD               = 76812,
    NPC_SLAG_ELEMENTAL               = 78463,
    NPC_FIRECALLER                   = 76821,

    // Blackhand
    NPC_SIEGEMAKER                   = 77342,
    NPC_IRON_SOLDIER                 = 77665
};

// exit doors, opened once their encounter is done
enum BlackrockFoundryGameObjects
{
    GO_FURNACE_PORTCULLIS            = 237224,  // Oregorger
    GO_TERMINUS_DOOR                 = 237307,  // Beastlord Darmac
    GO_BURNING_FRONT_DOOR            = 236842,  // Flamebender Ka'graz
    GO_HANSGAR_AND_FRANZOK_EXIT      = 229319,  // Hans'gar and Franzok
    GO_FREIGHT_ELEVATOR_DOOR         = 236848,  // Operator Thogar
    GO_KROMOGS_DOOR_EAST             = 236840,  // Kromog
    GO_BLACK_FORGE_PORTCULLIS        = 236847,  // Kromog
    GO_BLACK_FORGE_GATE              = 237306,  // Kromog
    GO_KROMOG_DOOR                   = 234029   // Kromog
};

enum BlackrockFoundrySharedSpells
{
    SPELL_FOUNDRY_BERSERK            = 26662
};

enum FoundryGruulSpells
{
    SPELL_GRUUL_INFERNO_SLICE        = 155080,
    SPELL_GRUUL_OVERWHELMING_BLOWS   = 155077,
    SPELL_GRUUL_PETRIFYING_SLAM      = 155326,
    SPELL_GRUUL_OVERHEAD_SMASH       = 155301,
    SPELL_GRUUL_DESTRUCTIVE_RAMPAGE  = 155539,
    SPELL_GRUUL_CAVE_IN              = 173191
};

enum FoundryOregorgerSpells
{
    SPELL_OREGORGER_ACID_MAW             = 173471,
    SPELL_OREGORGER_RETCHED_BLACKROCK    = 156179,
    SPELL_OREGORGER_EXPLOSIVE_SHARD      = 156390,
    SPELL_OREGORGER_ACID_TORRENT         = 156297,
    SPELL_OREGORGER_BLACKROCK_SPINES     = 156834,
    SPELL_OREGORGER_BLACKROCK_BARRAGE    = 173461,
    SPELL_OREGORGER_WALLSHAKING_ROAR     = 160662,
    SPELL_OREGORGER_ROLLING_FURY         = 155898,
    SPELL_OREGORGER_HUNGER_DRIVE         = 165127,
    SPELL_OREGORGER_EARTHSHAKING_STOMP   = 159958
};

enum FoundryDarmacSpells
{
    SPELL_DARMAC_CALL_THE_PACK           = 154975,
    SPELL_DARMAC_PIN_DOWN                = 154951,
    SPELL_DARMAC_TANTRUM                 = 155520,
    SPELL_DARMAC_SUPERHEATED_SHRAPNEL    = 155497,
    SPELL_DARMAC_REND_AND_TEAR           = 162279,
    SPELL_DARMAC_FURY_OF_THE_ELEKK       = 155460,
    SPELL_DARMAC_CUNNING_OF_THE_WOLF     = 155458,
    SPELL_DARMAC_SPIRIT_OF_THE_RYLAK     = 155459,

    SPELL_CRUELFANG_REND_AND_TEAR        = 155060,
    SPELL_CRUELFANG_SAVAGE_HOWL          = 155198,

    SPELL_DREADWING_INFERNO_BREATH       = 154988,
    SPELL_DREADWING_CONFLAGRATION        = 154981,

    SPELL_IRONCRUSHER_STAMPEDE           = 155247,
    SPELL_IRONCRUSHER_TANTRUM            = 155221,
    SPELL_IRONCRUSHER_CRUSH_ARMOR        = 155236
};

enum FoundryKagrazSpells
{
    SPELL_KAGRAZ_LAVA_SLASH              = 155297,
    SPELL_KAGRAZ_MOLTEN_TORRENT          = 154932,
    SPELL_KAGRAZ_BLAZING_RADIANCE        = 155277,
    SPELL_KAGRAZ_FIRESTORM               = 155564,
    SPELL_KAGRAZ_FIRESTORM_PULSE         = 155493,
    SPELL_KAGRAZ_FLAMEFURY               = 163273,

    SPELL_AKNOR_DEVASTATING_SLAM         = 156018,
    SPELL_AKNOR_DROP_THE_HAMMER          = 156039,

    SPELL_CINDER_WOLF_FIXATE             = 154952,
    SPELL_CINDER_WOLF_CHARRING_BREATH    = 155074,
    SPELL_CINDER_WOLF_OVERHEATED         = 154950
};

enum FoundryHansgarFranzokSpells
{
    SPELL_HANSGAR_JUMP_SLAM              = 157923,
    SPELL_ANVIL_BODY_SLAM                = 155747,
    SPELL_FRANZOK_DISRUPTING_ROAR        = 160838,
    SPELL_FRANZOK_SKULLCRACKER           = 153470
};

enum FoundryThogarSpells
{
    SPELL_THOGAR_ENKINDLE                = 155921,
    SPELL_THOGAR_PROTOTYPE_PULSE_GRENADE = 155864,

    SPELL_IRON_RAIDER_THROW_GRENADE      = 156294,
    SPELL_IRON_RAIDER_SERRATED_SLASH     = 155701,
    SPELL_CRACK_SHOT_SHOOT_RIFLE         = 156270,
    SPELL_FIREMENDER_CAUTERIZING_BOLT    = 160140,
    SPELL_MAN_AT_ARMS_IRON_BELLOW        = 163753,
    SPELL_MAN_AT_ARMS_RECKLESS_SLASH     = 163750
};

enum FoundryBlastFurnaceSpells
{
    SPELL_HEART_BLAST                    = 155209,
    SPELL_HEART_BUST_LOOSE               = 160823,
    SPELL_HEART_HEAT                     = 155242,
    SPELL_HEART_TEMPERED                 = 155240,
    SPELL_HEART_MELT                     = 155225,

    SPELL_FELDSPAR_PYROCLASM             = 156937,
    SPELL_FELDSPAR_RUPTURE               = 156934,
    SPELL_FELDSPAR_HOT_BLOODED           = 158247,

    SPELL_ENGINEER_ELECTROCUTION         = 155201,
    SPELL_ENGINEER_BOMB                  = 155192,
    SPELL_SECURITY_GUARD_DEFENSE         = 177773,
    SPELL_BELLOWS_OPERATOR_DEAFENING_ROAR = 177756,
    SPELL_SLAG_ELEMENTAL_FIXATE          = 155196,
    SPELL_SLAG_ELEMENTAL_BURN            = 155200,
    SPELL_FIRECALLER_CAUTERIZE_WOUNDS    = 155186,
    SPELL_FIRECALLER_LAVA_BURST          = 159408,
    SPELL_FIRECALLER_VOLATILE_FIRE       = 176121,
    SPELL_ELEMENTALIST_DAMAGE_SHIELD     = 155176,
    SPELL_ELEMENTALIST_REACTIVE_SHIELD   = 155173
};

enum FoundryKromogSpells
{
    SPELL_KROMOG_STONE_BREATH            = 156852,
    SPELL_KROMOG_WARPED_ARMOR            = 156766,
    SPELL_KROMOG_SLAM                    = 156704,
    SPELL_KROMOG_RIPPLING_SMASH          = 157592,
    SPELL_KROMOG_GRASPING_EARTH          = 157060,
    SPELL_KROMOG_THUNDERING_BLOWS        = 157054,
    SPELL_KROMOG_FRENZY                  = 156861,
    SPELL_KROMOG_FISTS_OF_STONE          = 162348
};

enum FoundryIronMaidensSpells
{
    SPELL_MAIDEN_IRON_WILL               = 159337,
    SPELL_MAIDEN_BOMBARDMENT_ALPHA       = 157854,

    SPELL_GARAN_RAPID_FIRE               = 156626,
    SPELL_GARAN_PENETRATING_SHOT         = 164271,
    SPELL_GARAN_DEPLOY_TURRET            = 158599,

    SPELL_SORKA_BLADE_DASH               = 155794,
    SPELL_SORKA_CONVULSIVE_SHADOWS       = 156109,
    SPELL_SORKA_DARK_HUNT                = 158315,

    SPELL_MARAK_BLOOD_RITUAL             = 158078,
    SPELL_MARAK_BLOODSOAKED_HEARTSEEKER  = 158010,
    SPELL_MARAK_SANGUINE_STRIKES         = 156601
};

enum FoundryBlackhandSpells
{
    SPELL_BLACKHAND_THROW_SLAG_BOMBS     = 156030,
    SPELL_BLACKHAND_DEMOLITION           = 156425,
    SPELL_BLACKHAND_SHATTERING_SMASH     = 155992,
    SPELL_BLACKHAND_IMPALING_THROW       = 156107,
    SPELL_BLACKHAND_ATTACH_SLAG_BOMBS    = 157000,
    SPELL_BLACKHAND_SLAG_BOMBS_TANK      = 159179,
    SPELL_BLACKHAND_SLAG_ERUPTION        = 156928,
    SPELL_BLACKHAND_MASSIVE_SMASH        = 158054,

    SPELL_SIEGEMAKER_BLACKIRON_PLATING   = 156667,
    SPELL_SIEGEMAKER_FIXATE              = 156653,
    SPELL_SIEGEMAKER_BATTERING_RAM       = 156643,
    SPELL_SIEGEMAKER_MORTAR              = 156530,

    SPELL_IRON_SOLDIER_EXPLOSIVE_ROUND   = 156728
};
