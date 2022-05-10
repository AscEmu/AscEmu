/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum EncounterData
{
    DATA_MAULGAR                    = 0,
    DATA_GRUUL                      = 1
};

enum CreatureEntry
{
    NPC_MAULGAR                     = 18831,
    NPC_KROSH_FIREHAND              = 18832,
    NPC_OLM_THE_SUMMONER            = 18834,
    NPC_KIGGLER_THE_CRAZED          = 18835,
    NPC_BLINDEYE_THE_SEER           = 18836,
    NPC_GRUUL_THE_DRAGONKILLER      = 19044,
    NPC_WILD_FEL_STALKER            = 18847,
    NPC_GRONN_PRIEST                = 21350,
    NPC_LAIR_BRUTE                  = 19389
};

enum GameObjectEntry
{
    GO_MAULGAR_DOOR                 = 184468,
    GO_GRUUL_DOOR                   = 184662
};

enum CreatureSpells
{
    // Wild Fel Stalker
    SPELL_WILD_BITE                 = 33086,

    // Gronn Priest
    SPELL_PSYCHICSCREAM             = 22884,
    SPELL_RENEW                     = 36679,
    SPELL_HEAL_GP                   = 36678,

    // Lair Brute
    SPELL_MORTALSTRIKE              = 39171,
    SPELL_CLEAVE                    = 39174,
    SPELL_CHARGE                    = 24193,

    // High King Maulgar
    SPELL_ARCING_SMASH              = 39144,
    SPELL_MIGHTY_BLOW               = 33230,
    SPELL_WHIRLWIND                 = 33238,
    SPELL_BERSERKER_C               = 26561,
    SPELL_ROAR                      = 16508,
    SPELL_FLURRY                    = 33232,
    SPELL_DUAL_WIELD                = 29651,

    // Olm the Summoner
    SPELL_DARK_DECAY                = 33129,
    SPELL_DEATH_COIL                = 33130,
    SPELL_SUMMON_WFH                = 33131,

    // Kiggler the Craed
    SPELL_GREATER_POLYMORPH         = 33173,
    SPELL_LIGHTNING_BOLT            = 36152,
    SPELL_ARCANE_SHOCK              = 33175,
    SPELL_ARCANE_EXPLOSION          = 33237,

    // Blindeye the Seer
    SPELL_GREATER_PW_SHIELD         = 33147,
    SPELL_HEAL                      = 33144,
    SPELL_PRAYER_OH                 = 33152,

    // Krosh Firehand
    SPELL_GREATER_FIREBALL          = 33051,
    SPELL_SPELLSHIELD               = 33054,
    SPELL_BLAST_WAVE                = 33061,

    // Gruul
    SPELL_GROWTH                    = 36300,
    SPELL_CAVE_IN                   = 36240,
    SPELL_GROUND_SLAM               = 33525, // AoE Ground Slam applying Ground Slam to everyone with a script effect (most likely the knock back, we can code it to a set knockback)
    SPELL_REVERBERATION             = 36297,
    SPELL_SHATTER                   = 33654,

    SPELL_SHATTER_EFFECT            = 33671,
    SPELL_HURTFUL_STRIKE            = 33813,
    SPELL_STONED                    = 33652, // Spell is self cast by target

    SPELL_MAGNETIC_PULL             = 28337,
    SPELL_KNOCK_BACK                = 24199, // Knockback spell until correct implementation is made
};

enum Events
{
    EVENT_SHATTER                   = 1,
    EVENT_HURTFUL_STRIKE,
    EVENT_BLASTWAVE
};

enum Actions
{
    ACTION_ENABLE                   = 1,
    ACTION_DISABLE                  = 2,
    ACTION_ADD_DEATH                = 3
};

enum DataTypes
{
    DATA_DOOR_MAULGAR               = 1,
    DATA_DOOR_GRUUL                 = 2
};

enum Phases
{
    PHASE_1                        = 1,
    PHASE_2                        = 2
};

enum Yells
{
    GRUUL_SAY_AGGRO                 = 4820,
    GRUUL_SAY_SLAM_01               = 4821,
    GRUUL_SAY_SLAM_02               = 4822,
    GRUUL_SAY_SHATTER_01            = 4823,
    GRUUL_SAY_SHATTER_02            = 4824,
    GRUUL_SAY_SLAY_01               = 4825,
    GRUUL_SAY_SLAY_02               = 4826,
    GRUUL_SAY_SLAY_03               = 4827,
    GRUUL_SAY_DEATH                 = 4828,
    GRUUL_EMOTE_GROW                = 4829,

    MAUL_SAY_AGGRO                  = 4830,
    MAUL_SAY_ENRAGE                 = 4831,
    MAUL_SAY_OGRE_DEATH_01          = 4832,
    MAUL_SAY_OGRE_DEATH_02          = 4833,
    MAUL_SAY_OGRE_DEATH_03          = 4834,
    MAUL_SAY_OGRE_DEATH_04          = 4835,
    MAUL_SAY_SLAY_01                = 4836,
    MAUL_SAY_SLAY_02                = 4837,
    MAUL_SAY_SLAY_03                = 4838,
    MAUL_SAY_DEATH                  = 4839,
};
