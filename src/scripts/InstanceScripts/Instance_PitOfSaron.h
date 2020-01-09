/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum ForgemasterGarfrostData
{
    SPELL_PERMAFROST                = 70326,
    SPELL_PERMAFROST_TRIGGER        = 68786,
    SPELL_FROZEBLADE                = 68774,
    H_SPELL_FORGE_BLADE             = 70334,
    SPELL_FROZEMACE                 = 68785,
    H_SPELL_FORGE_MACE              = 70335,
    SPELL_DEEPFREEZE                = 70381,
    H_SPELL_DEEP_FREEZE             = 72930,
    SPELL_STOMP                     = 68771,
    SPELL_CHILLINGWAVE              = 68778,
    H_SPELL_CHILLING_WAVE           = 70333,
    SPELL_THROWSARONITE             = 68788,
    EQUIP_ID_SWORD                  = 49345,
    EQUIP_ID_MACE                   = 49344
};

static LocationVector JumpCords[] =
{
    { 639.075f, -208.774f, 528.931f, 0.0f },
    { 725.325f, -236.978f, 528.848f, 0.0f },
};

enum Ick_KrickData
{
    SPELL_PURSUED                   = 68987,
    SPELL_CONFUSION                 = 69029,
    SPELL_EXPLOSIVE_BARRAGE_KRICK   = 69012,
    SPELL_EXPLOSIVE_BARRAGE         = 69263,
    SPELL_EXPLOSIVE_BARRAGE_SUMMON  = 69015,
    SPELL_EXPLODING_ORB             = 69017, // visual on exploding orb
    SPELL_AUTO_GROW                 = 69020, // grow effect on exploding orb
    SPELL_HASTY_GROW                = 44851, //need to stack growing stacks
    SPELL_EXPLOSIVE_BARRAGE_DAMAGE  = 69019, // damage done by orb exploding
    SPELL_MIGHTY_KICK               = 69021,
    SPELL_POISON_NOVA               = 68989,
    H_SPELL_POISON_NOVA             = 70434,
    SPELL_SHADOW_BOLT               = 69028,
    SPELL_TOXIC_WASTE               = 69024,
    H_SPELL_TOXIC_WASTE             = 70436,
    SPELL_STRANGULATE               = 69413, // Kricks spell in outro
    CREATURE_EXPLODING_ORB          = 36610
};

enum BattlePhases
{
    BATTLE,
    OUTRO
};

enum ScourgelordTyrannusData
{
};

enum CreatureIDs
{
    //Bosses
    CN_FORGEMASTER_GARFROST         = 36494,
    CN_ICK                          = 36476,
    CN_KRICK                        = 36477,
    CN_SCOURGELORD_TYRANNUS         = 36658,
    //Alliance Spawns
    CN_JAINA_PROUDMOORE             = 36993,
    CN_ARCHMAGE_ELANDRA             = 37774,
    CN_ARCHMAGE_KORELN              = 37582,
    //Horde Spawns
    CN_SYLVANAS_WINDRUNNER          = 36990,
    CN_DARK_RANGER_KALIRA           = 37583,
    CN_DARK_RANGER_LORALEN          = 37779
};
