/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const OperationMechagonEncounterCount = 8;

enum OperationMechagonData
{
    // The Platinum Pummeler and Gnomercy 4.U.
    DATA_TUSSLE_TONKS      = 0, // 2257
    DATA_KUJO              = 1, // 2258
    // Head Machinist Sparkflux
    DATA_MACHINISTS_GARDEN = 2, // 2259
    // Aerial Unit R-21/X, then the Omega Buster
    DATA_KING_MECHAGON     = 3, // 2260
    DATA_KING_GOBBAMAK     = 4, // 2290
    // Tank Buster MK1, then the unit itself
    DATA_HK8               = 5, // 2291
    DATA_GUNKER            = 6, // 2292
    // Trixie Tazer and Naeno Megacrash
    DATA_TRIXIE_NAENO      = 7  // 2312
};

enum OperationMechagonCreatures
{
    BOSS_PLATINUM_PUMMELER        = 144244,
    BOSS_GNOMERCY                 = 145185,

    BOSS_KUJO                     = 144246,
    NPC_FLYING_CLAW               = 150442,

    BOSS_HEAD_MACHINIST_SPARKFLUX = 144248,

    BOSS_AERIAL_UNIT              = 150396,
    BOSS_OMEGA_BUSTER             = 144249,
    NPC_KING_MECHAGON             = 150397,
    NPC_PLASMA_ORB                = 150485,

    BOSS_KING_GOBBAMAK            = 150159,
    NPC_SCRAPBONE_GRUNTER         = 158871,

    BOSS_HK8                      = 150190,
    NPC_TANK_BUSTER_MK1           = 150295,
    NPC_WALKIE_SHOCKIE_X1         = 163746,

    BOSS_GUNKER                   = 150222,

    BOSS_TRIXIE_TAZER             = 150712,
    BOSS_NAENO_MEGACRASH          = 153755,
    NPC_JUMP_POINT_STALKER        = 153886
};

enum OperationMechagonActions
{
    ACTION_OMEGA_BUSTER_ACTIVATE = 1,
    ACTION_OMEGA_BUSTER_DORMANT,
    ACTION_KING_STAGE_TWO,
    ACTION_KING_DEFEATED,
    ACTION_KING_RESET,
    ACTION_PLASMA_ORB_RECALIBRATE,
    ACTION_PLASMA_ORB_MAGNETIZE,
    ACTION_HK8_STAGE_TWO,
    ACTION_MK1_REBOOT,
    ACTION_GNOME_PARTNER_DOWN
};

enum TussleTonksSpells
{
    SPELL_PUMMELER_PLATINUM_PLATING = 282801,
    SPELL_PUMMELER_WHIRLING_EDGE    = 285020,
    SPELL_PUMMELER_LAY_MINE         = 285344,

    SPELL_GNOMERCY_MAXIMUM_THRUST   = 283422,
    SPELL_GNOMERCY_VENT_JETS        = 285388
};

enum KujoSpells
{
    SPELL_KUJO_BLAZING_CHOMP   = 294929,
    SPELL_KUJO_VENTING_FLAMES  = 291946,
    SPELL_KUJO_EXPLOSIVE_LEAP  = 292035,
    SPELL_FLYING_CLAW_AIR_DROP = 291930
};

enum SparkfluxSpells
{
    SPELL_SPARKFLUX_HIDDEN_FLAME_CANNON = 285443,
    SPELL_SPARKFLUX_DISCOM_BOMBULATOR   = 285454
};

enum KingMechagonSpells
{
    SPELL_AERIAL_PULSE_BLAST        = 291878,
    SPELL_AERIAL_TAKE_OFF           = 291613,
    SPELL_AERIAL_CUTTING_BEAM       = 291626,

    SPELL_OMEGA_GIGA_ZAP            = 291928,
    SPELL_OMEGA_MAGNETO_ARM         = 283143,

    SPELL_KING_OBNOXIOUS_MONOLOGUE  = 291974,

    SPELL_PLASMA_ORB_VISUAL         = 291835,
    SPELL_PLASMA_ORB_BURST          = 291915,
    SPELL_PLASMA_ORB_RECALIBRATE    = 291856
};

enum KingGobbamakSpells
{
    SPELL_GOBBAMAK_CHARGED_SMASH     = 297254,
    SPELL_GOBBAMAK_ELECTRICAL_CHARGE = 297257,
    SPELL_GOBBAMAK_RUMBLE            = 297261
};

enum HK8Spells
{
    SPELL_HK8_FORCE_SHIELD        = 300011,
    SPELL_HK8_CANNON_BLAST        = 295536,
    SPELL_HK8_CANNON_BLAST_SHELL  = 302098,
    SPELL_HK8_ANNIHILATION_RAY    = 295939,
    SPELL_HK8_HAYWIRE             = 296080,
    SPELL_HK8_LIFT_OFF            = 301177,

    SPELL_MK1_WRECK               = 302279,
    SPELL_MK1_FULMINATING_ZAP     = 302274,

    SPELL_WALKIE_BATTERY_LIFE     = 296517,
    SPELL_WALKIE_SELF_DESTRUCT    = 296522
};

enum GunkerSpells
{
    SPELL_GUNKER_SLUDGE_BOLT  = 298212,
    SPELL_GUNKER_SPLATTER     = 297985,
    SPELL_GUNKER_COALESCE     = 297835,
    SPELL_GUNKER_TOXIC_WAVE   = 297834,
    SPELL_GUNKER_TOXIC_FUMES  = 298228
};

enum TrixieNaenoSpells
{
    SPELL_TRIXIE_TAZE                = 298669,
    SPELL_TRIXIE_ELECTRIC_SLIDE      = 298849,
    SPELL_TRIXIE_MEGA_TAZE           = 302682,
    SPELL_TRIXIE_MEGA_TAZE_JOLT      = 298718,
    SPELL_TRIXIE_SUPER_BOOST         = 303525,

    SPELL_NAENO_PEDAL_TO_THE_METAL   = 298651,
    SPELL_NAENO_BURNOUT              = 298571,
    SPELL_NAENO_SMOKE_CLOUD          = 298602,
    SPELL_NAENO_BOLT_BUSTER          = 298940,
    SPELL_NAENO_ROADKILL             = 298946,
    SPELL_NAENO_TURBO_BOOST          = 299153
};
