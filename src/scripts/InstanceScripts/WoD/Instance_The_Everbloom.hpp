/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TheEverbloomEncounterCount = 5;

enum TheEverbloomData
{
    DATA_WITHERBARK         = 0, // 1746
    DATA_ARCHMAGE_SOL       = 1, // 1751
    DATA_XERITAC            = 2, // 1752
    DATA_YALNU              = 3, // 1756
    DATA_ANCIENT_PROTECTORS = 4  // 1757
};

enum TheEverbloomCreatures
{
    BOSS_WITHERBARK               = 81522,
    BOSS_LIFE_WARDEN_GOLA         = 83892,
    BOSS_EARTHSHAPER_TELU         = 83893,
    BOSS_DULHU                    = 83894,
    BOSS_XERITAC                  = 84550,
    BOSS_ARCHMAGE_SOL             = 82682,
    BOSS_YALNU                    = 83846,

    NPC_EB_AQUEOUS_GLOBULE        = 81638,
    NPC_EB_UNCHECKED_GROWTH       = 81564,
    NPC_EB_TOXIC_SPIDERLING       = 84552,
    NPC_EB_VENOM_SPRAYER          = 86547,
    NPC_EB_VENOM_CRAZED_PALE_ONE  = 84554,
    NPC_EB_GORGED_BURSTER         = 86552,
    NPC_EB_FROZEN_RAIN            = 82846,
    NPC_EB_SPORE_IMAGE            = 84386,
    NPC_EB_LADY_BAIHU             = 84358,
    NPC_EB_KIRIN_TOR_BATTLE_MAGE  = 84329,
    NPC_EB_VICIOUS_MANDRAGORA     = 84399,
    NPC_EB_GNARLED_ANCIENT        = 84400,
    NPC_EB_SWIFT_SPROUTLING       = 84401,
    NPC_EB_FERAL_LASHER           = 86684
};

enum TheEverbloomGameObjects
{
    GO_EB_WITHERBARK_VINE_WALL_1  = 231966,
    GO_EB_WITHERBARK_VINE_WALL_2  = 231967
};

enum WitherbarkSpells
{
    SPELL_WITHERBARK_PARCHED_GASP      = 164357,
    SPELL_WITHERBARK_AGITATED_WATER    = 177733,
    SPELL_WITHERBARK_BRITTLE_BARK      = 164275,
    SPELL_AQUEOUS_GLOBULE_BURST        = 164538,
    SPELL_UNCHECKED_GROWTH_VISUAL      = 164296,
    SPELL_UNCHECKED_GROWTH_VINES       = 164302
};

enum AncientProtectorsSpells
{
    SPELL_GOLA_WATER_BOLT              = 168092,
    SPELL_GOLA_REVITALIZING_WATERS     = 168082,
    SPELL_GOLA_RAPID_TIDES             = 168105,
    SPELL_TELU_NATURES_WRATH           = 168040,
    SPELL_TELU_BRAMBLE_PATCH           = 177497,
    SPELL_TELU_BRIARSKIN               = 168041,
    SPELL_DULHU_RENDING_CHARGE         = 168186,
    SPELL_DULHU_NOXIOUS_ERUPTION       = 175997,
    SPELL_DULHU_GRASPING_VINE          = 168375,
    SPELL_DULHU_SLASH                  = 168383
};

enum XeritacSpells
{
    SPELL_XERITAC_TOXIC_BOLT           = 169375,
    SPELL_XERITAC_VENOMOUS_STING       = 169376,
    SPELL_XERITAC_GAS_VOLLEY           = 169382,
    SPELL_VENOM_SPRAYER_SPRAY          = 172727,
    SPELL_PALE_ONE_SWIPE               = 169371
};

enum ArchmageSolSpells
{
    SPELL_SOL_PARASITIC_GROWTH         = 168885,
    SPELL_SOL_PARASITIC_INJECTION      = 166891,
    SPELL_SOL_AFFINITY_FIRE            = 166475,
    SPELL_SOL_AFFINITY_FROST           = 166476,
    SPELL_SOL_AFFINITY_ARCANE          = 166477,
    SPELL_SOL_FIREBALL                 = 168894,
    SPELL_SOL_FIREBLOOM                = 166562,
    SPELL_SOL_FROSTBOLT                = 169824,
    SPELL_SOL_ARCANE_BURST             = 166466,
    SPELL_FROZEN_RAIN_SPAWN            = 166723
};

enum YalnuSpells
{
    SPELL_YALNU_COLOSSAL_BLOW          = 169179,
    SPELL_YALNU_ENTANGLEMENT           = 169251,
    SPELL_YALNU_ENTANGLEMENT_PLAYER    = 170124,
    SPELL_YALNU_GENESIS                = 169613,
    SPELL_KIRIN_TOR_BLINK              = 142193,
    SPELL_KIRIN_TOR_FIREBALL           = 168666,
    SPELL_KIRIN_TOR_FLAMESTRIKE        = 169094,
    SPELL_KIRIN_TOR_FROSTBOLT          = 170028,
    SPELL_KIRIN_TOR_ICE_COMET          = 170032,
    SPELL_KIRIN_TOR_ARCANE_BLAST       = 170035,
    SPELL_KIRIN_TOR_ARCANE_ORB         = 170040
};
