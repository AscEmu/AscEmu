/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"

#include <cstdint>

enum PlayerSkills : uint16_t
{
#if VERSION_STRING <= Cata
    SKILL_FROST                         = 6,
    SKILL_FIRE                          = 8,
    SKILL_ARMS                          = 26,
    SKILL_COMBAT                        = 38,
    SKILL_SUBTLETY                      = 39,
#endif
#if VERSION_STRING <= TBC
    SKILL_POISONS                       = 40,
#endif
    SKILL_SWORDS                        = 43,
    SKILL_AXES                          = 44,
    SKILL_BOWS                          = 45,
    SKILL_GUNS                          = 46,
#if VERSION_STRING <= Cata
    SKILL_BEAST_MASTERY                 = 50,
    SKILL_SURVIVAL                      = 51,
#endif
    SKILL_MACES                         = 54,
    SKILL_2H_SWORDS                     = 55,
#if VERSION_STRING <= Cata
    SKILL_HOLY                          = 56,
    SKILL_SHADOW_MAGIC                  = 78,
#endif
    SKILL_DEFENSE                       = 95,
    SKILL_LANG_COMMON                   = 98,
    SKILL_RACIAL_DWARVEN                = 101,
    SKILL_LANG_ORCISH                   = 109,
    SKILL_LANG_DWARVEN                  = 111,
    SKILL_LANG_DARNASSIAN               = 113,
    SKILL_LANG_TAURAHE                  = 115,
    SKILL_DUAL_WIELD                    = 118,
    SKILL_RACIAL_TAUREN                 = 124,
    SKILL_RACIAL_ORC                    = 125,
    SKILL_RACIAL_NIGHT_ELF              = 126,
    SKILL_FIRST_AID                     = 129,
#if VERSION_STRING <= Cata
    SKILL_FERAL_COMBAT                  = 134,
#endif
    SKILL_STAVES                        = 136,
    SKILL_LANG_THALASSIAN               = 137,
    SKILL_LANG_DRACONIC                 = 138,
    SKILL_LANG_DEMON_TONGUE             = 139,
    SKILL_LANG_TITAN                    = 140,
    SKILL_LANG_OLD_TONGUE               = 141,
    SKILL_SURVIVAL2                     = 142,
    SKILL_HORSE_RIDING                  = 148,
    SKILL_WOLF_RIDING                   = 149,
    SKILL_TIGER_RIDING                  = 150,
    SKILL_RAM_RIDING                    = 152,
    SKILL_SWIMMING                      = 155,
    SKILL_2H_MACES                      = 160,
    SKILL_UNARMED                       = 162,
#if VERSION_STRING <= Cata
    SKILL_MARKSMANSHIP                  = 163,
#endif
    SKILL_BLACKSMITHING                 = 164,
    SKILL_LEATHERWORKING                = 165,
    SKILL_ALCHEMY                       = 171,
    SKILL_2H_AXES                       = 172,
    SKILL_DAGGERS                       = 173,
#if VERSION_STRING <= Cata
    SKILL_THROWN                        = 176,
#endif
    SKILL_HERBALISM                     = 182,
    SKILL_GENERIC_DND                   = 183,
#if VERSION_STRING <= Cata
    SKILL_RETRIBUTION                   = 184,
#endif
    SKILL_COOKING                       = 185,
    SKILL_MINING                        = 186,
    SKILL_PET_IMP                       = 188,
    SKILL_PET_FELHUNTER                 = 189,
    SKILL_TAILORING                     = 197,
    SKILL_ENGINEERING                   = 202,
    SKILL_PET_SPIDER                    = 203,
    SKILL_PET_VOIDWALKER                = 204,
    SKILL_PET_SUCCUBUS                  = 205,
    SKILL_PET_INFERNAL                  = 206,
    SKILL_PET_DOOMGUARD                 = 207,
    SKILL_PET_WOLF                      = 208,
    SKILL_PET_CAT                       = 209,
    SKILL_PET_BEAR                      = 210,
    SKILL_PET_BOAR                      = 211,
    SKILL_PET_CROCOLISK                 = 212,
    SKILL_PET_CARRION_BIRD              = 213,
    SKILL_PET_CRAB                      = 214,
    SKILL_PET_GORILLA                   = 215,
    SKILL_PET_RAPTOR                    = 217,
    SKILL_PET_TALLSTRIDER               = 218,
    SKILL_RACIAL_UNDEAD                 = 220,
    SKILL_CROSSBOWS                     = 226,
    SKILL_WANDS                         = 228,
    SKILL_POLEARMS                      = 229,
    SKILL_PET_SCORPID                   = 236,
#if VERSION_STRING <= Cata
    SKILL_ARCANE                        = 237,
#endif
    SKILL_PET_TURTLE                    = 251,
#if VERSION_STRING <= Cata
    SKILL_ASSASSINATION                 = 253,
    SKILL_FURY                          = 256,
    SKILL_PROTECTION                    = 257,
    SKILL_PROTECTION2                   = 267,
#endif
    SKILL_PET_GENERIC_HUNTER            = 270,
    SKILL_PLATE_MAIL                    = 293,
    SKILL_LANG_GNOMISH                  = 313,
    SKILL_LANG_TROLL                    = 315,
    SKILL_ENCHANTING                    = 333,
#if VERSION_STRING <= Cata
    SKILL_DEMONOLOGY                    = 354,
    SKILL_AFFLICTION                    = 355,
#endif
    SKILL_FISHING                       = 356,
#if VERSION_STRING <= Cata
    SKILL_ENHANCEMENT                   = 373,
    SKILL_RESTORATION                   = 374,
    SKILL_ELEMENTAL_COMBAT              = 375,
#endif
    SKILL_SKINNING                      = 393,
    SKILL_MAIL                          = 413,
    SKILL_LEATHER                       = 414,
    SKILL_CLOTH                         = 415,
    SKILL_SHIELD                        = 433,
    SKILL_FIST_WEAPONS                  = 473,
    SKILL_RAPTOR_RIDING                 = 533,
    SKILL_MECHANOSTRIDER_PILOTING       = 553,
    SKILL_UNDEAD_HORSEMANSHIP           = 554,
#if VERSION_STRING <= Cata
    SKILL_RESTORATION2                  = 573,
    SKILL_BALANCE                       = 574,
    SKILL_DESTRUCTION                   = 593,
    SKILL_HOLY2                         = 594,
    SKILL_DISCIPLINE                    = 613,
#endif
#if VERSION_STRING <= WotLK
    SKILL_LOCKPICKING                   = 633,
#endif
    SKILL_PET_BAT                       = 653,
    SKILL_PET_HYENA                     = 654,
    SKILL_PET_BIRD_OF_PREY              = 655,
    SKILL_PET_WIND_SERPENT              = 656,
    SKILL_LANG_GUTTERSPEAK              = 673,
    SKILL_KODO_RIDING                   = 713,
    SKILL_RACIAL_TROLL                  = 733,
    SKILL_RACIAL_GNOME                  = 753,
    SKILL_RACIAL_HUMAN                  = 754,
    SKILL_PET_EVENT_REMOTECONTROL       = 758,
    SKILL_RIDING                        = 762,
// TBC
#if VERSION_STRING >= TBC
    SKILL_JEWELCRAFTING                 = 755,
    SKILL_RACIAL_BLOOD_ELF              = 756,
    SKILL_LANG_DRAENEI                  = 759,
    SKILL_RACIAL_DRAENEI                = 760,
    SKILL_PET_FELGUARD                  = 761,
    SKILL_PET_DRAGONHAWK                = 763,
    SKILL_PET_NETHER_RAY                = 764,
    SKILL_PET_SPOREBAT                  = 765,
    SKILL_PET_WARP_STALKER              = 766,
    SKILL_PET_RAVAGER                   = 767,
    SKILL_PET_SERPENT                   = 768,
    SKILL_INTERNAL                      = 769,
#endif
// WoTLK
#if VERSION_STRING >= WotLK
#if VERSION_STRING <= Cata
    SKILL_BLOOD                         = 770,
    SKILL_FROST2                        = 771,
    SKILL_UNHOLY                        = 772,
#endif
    SKILL_INSCRIPTION                   = 773,
    SKILL_PET_MOTH                      = 775,
#if VERSION_STRING <= Cata
    SKILL_RUNEFORGING                   = 776,
#endif
    SKILL_MOUNTS                        = 777,
    SKILL_COMPANIONS                    = 778,
    SKILL_PET_EXOTIC_CHIMAERA           = 780,
    SKILL_PET_EXOTIC_DEVLISAUR          = 781,
    SKILL_PET_GHOUL                     = 782,
    SKILL_PET_EXOTIC_SILITHID           = 783,
    SKILL_PET_EXOTIC_WORM               = 784,
    SKILL_PET_WASP                      = 785,
    SKILL_PET_EXOTIC_RHINO              = 786,
    SKILL_PET_EXOTIC_CORE_HOUND         = 787,
    SKILL_PET_EXOTIC_SPIRIT_BEAST       = 788,
#endif
// Cata
#if VERSION_STRING >= Cata
    SKILL_RACIAL_WORGEN                 = 789,
    SKILL_RACIAL_GOLBLIN                = 790,
    SKILL_LANG_GILNEAN                  = 791,
    SKILL_LANG_GOBLIN                   = 792,
    SKILL_ARCHAEOLOGY                   = 794,
    SKILL_GENERAL_HUNTER                = 795,
    SKILL_GENERAL_DEATH_KNIGHT          = 796,
#if VERSION_STRING == Cata
    SKILL_GENERAL_ROGUE                 = 797,
#endif
    SKILL_GENERAL_DRUID                 = 798,
#if VERSION_STRING == Cata
    SKILL_GENERAL_MAGE                  = 799,
#endif
    SKILL_GENERAL_PALADIN               = 800,
#if VERSION_STRING == Cata
    SKILL_GENERAL_SHAMAN                = 801,
    SKILL_GENERAL_WARLOCK               = 802,
    SKILL_GENERAL_WARRIOR               = 803,
#endif
    SKILL_GENERAL_PRIEST                = 804,
    SKILL_PET_WATER_ELEMENTAL           = 805,
    SKILL_PET_FOX                       = 808,
    SKILL_ALL_GLYPHS                    = 810,
    SKILL_PET_DOG                       = 811,
    SKILL_PET_MONKEY                    = 815,
    SKILL_PET_SHALE_SPIDER              = 817,
    SKILL_PET_BEETLE                    = 818,
    SKILL_ALL_GUILD_PERKS               = 821,
    SKILL_PET_HYDRA                     = 824,
#endif
// Mop
#if VERSION_STRING >= Mop
    SKILL_GENERAL_MONK                  = 829,
    SKILL_GENERAL_WARRIOR               = 840,
    SKILL_GENERAL_WARLOCK               = 849,
    SKILL_PANDAREN_RACIAL               = 899,
    SKILL_GENERAL_MAGE                  = 904,
    SKILL_LANG_PANDAREN_NEUTRAL         = 905,
    SKILL_LANG_PANDAREN_ALLIANCE        = 906,
    SKILL_LANG_PANDAREN_HORDE           = 907,
    SKILL_GENERAL_ROGUE                 = 921,
    SKILL_GENERAL_SHAMAN                = 924,
    SKILL_FEL_IMP                       = 927,
    SKILL_VOIDLORD                      = 928,
    SKILL_SHIVARRA                      = 929,
    SKILL_OBSERVER                      = 930,
    SKILL_WRATHGUARD                    = 931,
    SKILL_ALL_SPECIALIZATIONS           = 934,
    SKILL_RUNEFORGING                   = 960,
    SKILL_PET_PRIMALFIRE_ELEMENTAL      = 962,
    SKILL_PET_PRIMALEARTH_ELEMENTAL     = 963,
    SKILL_WAYOFTHE_GRILL                = 975,
    SKILL_WAYOFTHE_WOK                  = 976,
    SKILL_WAYOFTHE_POT                  = 977,
    SKILL_WAYOFTHE_STEAMER              = 978,
    SKILL_WAYOFTHE_OVEN                 = 979,
    SKILL_WAYOFTHE_BREW                 = 980,
    SKILL_APPRENTICE_COOKING            = 981,
    SKILL_JOURNEYMAN_COOKBOOK           = 982,
    SKILL_PORCUPINE                     = 983,
    SKILL_CRANE                         = 984,
    SKILL_WATER_STRIDER                 = 985,
    SKILL_PET_QUILEN                    = 986,
    SKILL_PET_GOAT                      = 987,
    SKILL_BASILISK                      = 988,
    SKILL_NO_PLAYERS                    = 999,
    SKILL_DIREHORN                      = 1305,
#endif
};

enum SkillTypes : uint8_t
{
    SKILL_TYPE_UNK                      = 5,
    SKILL_TYPE_WEAPON                   = 6,  // Weapon skills
    SKILL_TYPE_CLASS                    = 7,  // Class skills (i.e frost, fire and arcane for mages, and pet family types for hunters)
    SKILL_TYPE_ARMOR                    = 8,  // Armor skills
    SKILL_TYPE_SECONDARY                = 9,  // Secondary skills (riding, fishing etc)
    SKILL_TYPE_LANGUAGE                 = 10, // Language skills
    SKILL_TYPE_PROFESSION               = 11, // Profession skills
    SKILL_TYPE_NA                       = 12
};
