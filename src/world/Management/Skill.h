/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <cstdint>

enum PlayerSkills
{
    SKILL_FROST                   = 6,
    SKILL_FIRE                    = 8,
    SKILL_ARMS                    = 26,
    SKILL_COMBAT                  = 38,
    SKILL_SUBTLETY                = 39,
    SKILL_SWORDS                  = 43,
    SKILL_AXES                    = 44,
    SKILL_BOWS                    = 45,
    SKILL_GUNS                    = 46,
    SKILL_BEAST_MASTERY           = 50,
    SKILL_SURVIVAL                = 51,
    SKILL_MACES                   = 54,
    SKILL_2H_SWORDS               = 55,
    SKILL_HOLY                    = 56,
    SKILL_SHADOW_MAGIC            = 78,
    SKILL_DEFENSE                 = 95,
    SKILL_LANG_COMMON             = 98,
    SKILL_RACIAL_DWARVEN          = 101,
    SKILL_LANG_ORCISH             = 109,
    SKILL_LANG_DWARVEN            = 111,
    SKILL_LANG_DARNASSIAN         = 113,
    SKILL_LANG_TAURAHE            = 115,
    SKILL_DUAL_WIELD              = 118,
    SKILL_RACIAL_TAUREN           = 124,
    SKILL_RACIAL_ORC              = 125,
    SKILL_RACIAL_NIGHT_ELF        = 126,
    SKILL_FIRST_AID               = 129,
    SKILL_FERAL_COMBAT            = 134,
    SKILL_STAVES                  = 136,
    SKILL_LANG_THALASSIAN         = 137,
    SKILL_LANG_DRACONIC           = 138,
    SKILL_LANG_DEMON_TONGUE       = 139,
    SKILL_LANG_TITAN              = 140,
    SKILL_LANG_OLD_TONGUE         = 141,
    SKILL_SURVIVAL2               = 142,
    SKILL_HORSE_RIDING            = 148,
    SKILL_WOLF_RIDING             = 149,
    SKILL_TIGER_RIDING            = 150,
    SKILL_RAM_RIDING              = 152,
    SKILL_SWIMMING                = 155,
    SKILL_2H_MACES                = 160,
    SKILL_UNARMED                 = 162,
    SKILL_MARKSMANSHIP            = 163,
    SKILL_BLACKSMITHING           = 164,
    SKILL_LEATHERWORKING          = 165,
    SKILL_ALCHEMY                 = 171,
    SKILL_2H_AXES                 = 172,
    SKILL_DAGGERS                 = 173,
    SKILL_THROWN                  = 176,
    SKILL_HERBALISM               = 182,
    SKILL_GENERIC_DND             = 183,
    SKILL_RETRIBUTION             = 184,
    SKILL_COOKING                 = 185,
    SKILL_MINING                  = 186,
    SKILL_PET_IMP                 = 188,
    SKILL_PET_FELHUNTER           = 189,
    SKILL_TAILORING               = 197,
    SKILL_ENGINEERING             = 202,
    SKILL_PET_SPIDER              = 203,
    SKILL_PET_VOIDWALKER          = 204,
    SKILL_PET_SUCCUBUS            = 205,
    SKILL_PET_INFERNAL            = 206,
    SKILL_PET_DOOMGUARD           = 207,
    SKILL_PET_WOLF                = 208,
    SKILL_PET_CAT                 = 209,
    SKILL_PET_BEAR                = 210,
    SKILL_PET_BOAR                = 211,
    SKILL_PET_CROCOLISK           = 212,
    SKILL_PET_CARRION_BIRD        = 213,
    SKILL_PET_CRAB                = 214,
    SKILL_PET_GORILLA             = 215,
    SKILL_PET_RAPTOR              = 217,
    SKILL_PET_TALLSTRIDER         = 218,
    SKILL_RACIAL_UNDEAD           = 220,
    SKILL_CROSSBOWS               = 226,
    SKILL_WANDS                   = 228,
    SKILL_POLEARMS                = 229,
    SKILL_PET_SCORPID             = 236,
    SKILL_ARCANE                  = 237,
    SKILL_PET_TURTLE              = 251,
    SKILL_ASSASSINATION           = 253,
    SKILL_FURY                    = 256,
    SKILL_PROTECTION              = 257,
    SKILL_PROTECTION2             = 267,
    SKILL_PET_GENERIC_HUNTER      = 270,
    SKILL_PLATE_MAIL              = 293,
    SKILL_LANG_GNOMISH            = 313,
    SKILL_LANG_TROLL              = 315,
    SKILL_ENCHANTING              = 333,
    SKILL_DEMONOLOGY              = 354,
    SKILL_AFFLICTION              = 355,
    SKILL_FISHING                 = 356,
    SKILL_ENHANCEMENT             = 373,
    SKILL_RESTORATION             = 374,
    SKILL_ELEMENTAL_COMBAT        = 375,
    SKILL_SKINNING                = 393,
    SKILL_MAIL                    = 413,
    SKILL_LEATHER                 = 414,
    SKILL_CLOTH                   = 415,
    SKILL_SHIELD                  = 433,
    SKILL_FIST_WEAPONS            = 473,
    SKILL_RAPTOR_RIDING           = 533,
    SKILL_MECHANOSTRIDER_PILOTING = 553,
    SKILL_UNDEAD_HORSEMANSHIP     = 554,
    SKILL_RESTORATION2            = 573,
    SKILL_BALANCE                 = 574,
    SKILL_DESTRUCTION             = 593,
    SKILL_HOLY2                   = 594,
    SKILL_DISCIPLINE              = 613,
    SKILL_LOCKPICKING             = 633,
    SKILL_PET_BAT                 = 653,
    SKILL_PET_HYENA               = 654,
    SKILL_PET_BIRD_OF_PREY        = 655,
    SKILL_PET_WIND_SERPENT        = 656,
    SKILL_LANG_GUTTERSPEAK        = 673,
    SKILL_KODO_RIDING             = 713,
    SKILL_RACIAL_TROLL            = 733,
    SKILL_RACIAL_GNOME            = 753,
    SKILL_RACIAL_HUMAN            = 754,
    SKILL_JEWELCRAFTING           = 755,
    SKILL_RACIAL_BLOOD_ELF        = 756,
    SKILL_PET_EVENT_REMOTECONTROL = 758,
    SKILL_LANG_DRAENEI            = 759,
    SKILL_RACIAL_DRAENEI          = 760,
    SKILL_PET_FELGUARD            = 761,
    SKILL_RIDING                  = 762,
    SKILL_PET_DRAGONHAWK          = 763,
    SKILL_PET_NETHER_RAY          = 764,
    SKILL_PET_SPOREBAT            = 765,
    SKILL_PET_WARP_STALKER        = 766,
    SKILL_PET_RAVAGER             = 767,
    SKILL_PET_SERPENT             = 768,
    SKILL_INTERNAL                = 769,
    SKILL_BLOOD                   = 770,
    SKILL_FROST2                  = 771,
    SKILL_UNHOLY                  = 772,
    SKILL_INSCRIPTION             = 773,
    SKILL_PET_MOTH                = 775,
    SKILL_RUNEFORGING             = 776,
    SKILL_MOUNTS                  = 777,
    SKILL_COMPANIONS              = 778,
    SKILL_PET_EXOTIC_CHIMAERA     = 780,
    SKILL_PET_EXOTIC_DEVLISAUR    = 781,
    SKILL_PET_GHOUL               = 782,
    SKILL_PET_EXOTIC_SILITHID     = 783,
    SKILL_PET_EXOTIC_WORM         = 784,
    SKILL_PET_WASP                = 785,
    SKILL_PET_EXOTIC_RHINO        = 786,
    SKILL_PET_EXOTIC_CORE_HOUND   = 787,
#if VERSION_STRING != Cata
    SKILL_PET_EXOTIC_SPIRIT_BEAST = 788
#else
    SKILL_PET_EXOTIC_SPIRIT_BEAST = 788,
    SKILL_RACIAL_WORGEN           = 789,
    SKILL_RACIAL_GOLBLIN          = 790,
    SKILL_LANG_GILNEAN            = 791,
    SKILL_LANG_GOBLIN             = 792,
    SKILL_ARCHAEOLOGY             = 794
#endif
};

enum SkillTypes
{
    SKILL_TYPE_ATTRIBUTES           = 5,
    SKILL_TYPE_WEAPON               = 6,
    SKILL_TYPE_CLASS                = 7,
    SKILL_TYPE_ARMOR                = 8,
    SKILL_TYPE_SECONDARY            = 9,
    SKILL_TYPE_LANGUAGE             = 10,
    SKILL_TYPE_PROFESSION           = 11,
    SKILL_TYPE_NA                   = 12
};

typedef struct
{
    uint8_t itemclass;
    uint32_t subclass;
} ItemProf;

const ItemProf* GetProficiencyBySkill(uint32_t skill);
