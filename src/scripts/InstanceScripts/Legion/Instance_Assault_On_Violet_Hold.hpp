/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const AssaultOnVioletHoldEncounterCount = 8;

enum AssaultOnVioletHoldData
{
    DATA_SHIVERMAW              = 0,  // 1845
    DATA_MINDFLAYER_KAAHRJ      = 1,  // 1846
    DATA_MILLIFICENT_MANASTORM  = 2,  // 1847
    DATA_FESTERFACE             = 3,  // 1848
    DATA_SAELORN                = 4,  // 1851
    DATA_ANUBESSET              = 5,  // 1852
    DATA_BLOOD_PRINCESS_THALENA = 6,  // 1855
    DATA_FEL_LORD_BETRUG        = 7   // 1856
};

enum AssaultOnVioletHoldCreatures
{
    NPC_LIEUTENANT_SINCLARI         = 102278,

    BOSS_SHIVERMAW                  = 101951,

    BOSS_MINDFLAYER_KAAHRJ          = 101950,
    NPC_FACELESS_TENDRIL            = 101994,

    BOSS_MILLIFICENT_MANASTORM      = 101976,
    NPC_THORIUM_ROCKET_CHICKEN      = 102103,
    NPC_REINFORCED_ROCKET_CHICKEN   = 102139,

    BOSS_FESTERFACE                 = 101995,
    NPC_CONGEALED_GOO               = 102158,
    NPC_BLACK_BILE                  = 102169,

    BOSS_SAELORN                    = 102387,
    NPC_PHASE_SPIDER                = 102434,

    BOSS_ANUBESSET                  = 102246,
    NPC_SPITTING_SCARAB             = 102271,

    BOSS_BLOOD_PRINCESS_THALENA     = 102431,

    BOSS_FEL_LORD_BETRUG            = 102446,

    // portal invaders
    NPC_PORTAL_GUARDIAN_1           = 102335,
    NPC_PORTAL_GUARDIAN_2           = 102337,
    NPC_PORTAL_KEEPER_1             = 102302,
    NPC_PORTAL_KEEPER_2             = 102336,
    NPC_FELGUARD_DESTROYER_1        = 102272,
    NPC_FELGUARD_DESTROYER_2        = 102368,
    NPC_SHADOW_COUNCIL_WARLOCK      = 102380,
    NPC_EREDAR_SHADOW_MENDER        = 102400,
    NPC_INFILTRATOR_ASSASSIN        = 102395,
    NPC_WRATHLORD_BULWARK           = 102397,
    NPC_FELSTALKER_RAVENER_1        = 102269,
    NPC_FELSTALKER_RAVENER_2        = 102369,
    NPC_EREDAR_INVADER_1            = 102270,
    NPC_EREDAR_INVADER_2            = 102370
};

enum AssaultOnVioletHoldGameObjects
{
    GO_PRISON_SEAL                  = 247002,

    GO_SHIVERMAW_CELL               = 246999,
    GO_KAAHRJ_CELL                  = 247000,
    GO_MILLIFICENT_CELL             = 246990,
    GO_FESTERFACE_CELL              = 246998,
    GO_SAELORN_CELL                 = 246993,
    GO_ANUBESSET_CELL               = 247385,
    GO_THALENA_CELL                 = 246997
};

enum AssaultOnVioletHoldActions
{
    ACTION_JAILER_RELEASE           = 1,
    ACTION_INVADER_CHARGE
};

// npc_gossip_texts entry of Sinclari
uint32_t const SinclariAssaultGossipText = 28225;

enum AssaultOnVioletHoldShivermawSpells
{
    SPELL_SHIVERMAW_FROST_BREATH        = 201379,
    SPELL_SHIVERMAW_RELENTLESS_STORM    = 201672,
    SPELL_SHIVERMAW_TAIL_SWEEP          = 201354,
    SPELL_SHIVERMAW_WING_BUFFET         = 201355,
    SPELL_SHIVERMAW_ICE_BOMB            = 201960,
    SPELL_SHIVERMAW_FRIGID_WINDS        = 202062
};

enum AssaultOnVioletHoldKaahrjSpells
{
    SPELL_KAAHRJ_SHADOW_CRASH           = 201920,
    SPELL_KAAHRJ_DOOM                   = 201148,
    SPELL_KAAHRJ_HYSTERIA               = 201146,
    SPELL_KAAHRJ_ETERNAL_DARKNESS       = 201153,
    SPELL_TENDRIL_ETERNAL_DARKNESS      = 201172
};

enum AssaultOnVioletHoldMillificentSpells
{
    SPELL_MILLIFICENT_FINGER_LASER      = 201159,
    SPELL_MILLIFICENT_RAGE              = 201572,

    SPELL_CHICKEN_SWARM_ROCKETS         = 201356,
    SPELL_CHICKEN_ROCKET                = 201369,
    SPELL_CHICKEN_THORIUM_PLATING       = 201441
};

enum AssaultOnVioletHoldFesterfaceSpells
{
    SPELL_FESTERFACE_AGGRO              = 202000,
    SPELL_FESTERFACE_CONGEALING_VOMIT   = 201598,

    SPELL_GOO_FETID_STENCH              = 201476,
    SPELL_GOO_SLIME_COSMETICS           = 205156
};

enum AssaultOnVioletHoldSaelornSpells
{
    SPELL_SAELORN_VENOM_SPRAY           = 202414,
    SPELL_SAELORN_WEB_GRAB              = 202462,
    SPELL_SAELORN_FEL_DETONATION        = 202473,
    SPELL_SAELORN_TOXIC_BLOOD           = 210504
};

enum AssaultOnVioletHoldAnubessetSpells
{
    SPELL_ANUBESSET_MANDIBLE_STRIKE     = 202217,
    SPELL_ANUBESSET_IMPALE              = 202341,

    SPELL_SCARAB_POISON_SPIT            = 202300
};

enum AssaultOnVioletHoldThalenaSpells
{
    SPELL_THALENA_SHROUD_OF_SORROW      = 203033,
    SPELL_THALENA_VAMPYRS_KISS          = 202766,
    SPELL_THALENA_BLOOD_SWARM           = 202659,
    SPELL_THALENA_BLOOD_CALL            = 203381
};

enum AssaultOnVioletHoldBetrugSpells
{
    SPELL_BETRUG_ARMED                  = 179670,
    SPELL_BETRUG_CHAOTIC_ENERGY         = 203622,
    SPELL_BETRUG_FEL_SLASH              = 203637,
    SPELL_BETRUG_MIGHTY_SMASH           = 202328,
    SPELL_BETRUG_SEED_OF_DESTRUCTION    = 210879
};
