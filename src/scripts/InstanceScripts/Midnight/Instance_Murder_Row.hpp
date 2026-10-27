/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const MurderRowEncounterCount = 4;

enum MurderRowData
{
    DATA_KYSTIA     = 0,    // 3101
    DATA_ZAEN       = 1,    // 3102
    DATA_XATHUUX    = 2,    // 3103
    DATA_LITHIEL    = 3     // 3105
};

enum MurderRowCreatures
{
    BOSS_KYSTIA                     = 234648,
    NPC_KYSTIA_NIBBLES              = 234660,

    BOSS_ZAEN                       = 234649,
    NPC_ZAEN_FORBIDDEN_FREIGHT      = 234852,

    BOSS_XATHUUX                    = 234647,

    BOSS_LITHIEL                    = 237415,
    // second template of the same boss
    BOSS_LITHIEL_ALT                = 234763,
    NPC_LITHIEL_WILD_IMP            = 237626,
    NPC_LITHIEL_FURIOUS_VILEFIEND   = 234799
};

enum MurderRowGameObjects
{
    // seals Kystia's hideout while she fights
    GO_KYSTIA_FEL_DOOR = 618401
};

enum MurderRowActions
{
    ACTION_NIBBLES_TURNED = 1,
    ACTION_NIBBLES_RECLAIMED,
    ACTION_NIBBLES_FLEES,
    ACTION_FREIGHT_DETONATE
};

enum KystiaSpells
{
    SPELL_KYSTIA_FELSHIELD              = 1217989,
    SPELL_KYSTIA_CHAOS_BARRAGE          = 1230298,
    SPELL_KYSTIA_MIRROR_IMAGES          = 1264095,
    SPELL_KYSTIA_FEL_NOVA               = 1223906,
    SPELL_KYSTIA_INFUSE_NIBBLES         = 474238,
    SPELL_KYSTIA_DESTABILIZED           = 1265412,

    SPELL_NIBBLES_ILLICIT_INFUSION      = 1230289,
    SPELL_NIBBLES_FEL_SPRAY             = 1253811,
    SPELL_NIBBLES_CORRODING_SPITTLE     = 1228198,
    SPELL_NIBBLES_LIGHT_INFUSION        = 1230304,
    SPELL_NIBBLES_ESCAPE                = 1248184
};

enum ZaenSpells
{
    SPELL_ZAEN_MURDER_IN_A_ROW          = 1218347,
    SPELL_ZAEN_SAME_DAY_DELIVERY        = 474765,
    SPELL_ZAEN_KILLING_SPREE            = 474478,
    SPELL_ZAEN_FIRE_BOMB                = 1214357,
    SPELL_ZAEN_ENVENOM                  = 1222795,

    SPELL_FREIGHT_EXPLOSION             = 1266241
};

enum XathuuxSpells
{
    SPELL_XATHUUX_DEMONIC_RAGE          = 474197,
    SPELL_XATHUUX_BURNING_STEPS         = 474234,
    SPELL_XATHUUX_AXE_TOSS              = 1214663,
    SPELL_XATHUUX_INFERNAL_CRUSH        = 1295455,
    SPELL_XATHUUX_LEGION_STRIKE         = 473898
};

enum LithielSpells
{
    SPELL_LITHIEL_MALEFIC_WAVE          = 1217384,
    SPELL_LITHIEL_MALEFIC_EMPOWERMENT   = 1226469,
    SPELL_LITHIEL_FELSHIELD             = 1217415,
    SPELL_LITHIEL_DEMONIC_GATEWAY       = 1214675,
    SPELL_LITHIEL_CHAOS_BOLT            = 474375,
    SPELL_LITHIEL_FINGERS_OF_GULDAN     = 474457,
    SPELL_LITHIEL_SEARING_FEL_FLAME     = 1216945,

    SPELL_WILD_IMP_FELFIRE_BURST        = 1223204,
    SPELL_VILEFIEND_SHADOW_BITE         = 1217881
};
