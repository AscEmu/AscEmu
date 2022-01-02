/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

typedef enum
{
    EFF_TARGET_NONE                                               = 0,
    EFF_TARGET_SELF                                               = 1,
    EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS     = 3,
    EFF_TARGET_PET                                                = 5,
    EFF_TARGET_SINGLE_ENEMY                                       = 6,
    EFF_TARGET_SCRIPTED_TARGET                                    = 7,
    EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS           = 8,
    EFF_TARGET_HEARTSTONE_LOCATION                                = 9,
    EFF_TARGET_ALL_ENEMY_IN_AREA                                  = 15,
    EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT                          = 16,
    EFF_TARGET_TELEPORT_LOCATION                                  = 17,
    EFF_TARGET_LOCATION_TO_SUMMON                                 = 18,
    EFF_TARGET_ALL_PARTY_AROUND_CASTER                            = 20,
    EFF_TARGET_SINGLE_FRIEND                                      = 21,
    EFF_TARGET_ALL_ENEMIES_AROUND_CASTER                          = 22,
    EFF_TARGET_GAMEOBJECT                                         = 23,
    EFF_TARGET_IN_FRONT_OF_CASTER                                 = 24,
    EFF_TARGET_DUEL                                               = 25, // Don't know the real name!!!
    EFF_TARGET_GAMEOBJECT_ITEM                                    = 26,
    EFF_TARGET_PET_MASTER                                         = 27,
    EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED                        = 28,
    EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED                        = 29,
    EFF_TARGET_ALL_FRIENDLY_IN_AREA                               = 30,
    EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME = 31,
    EFF_TARGET_MINION                                             = 32,
    EFF_TARGET_ALL_PARTY_IN_AREA                                  = 33,
    EFF_TARGET_SINGLE_PARTY                                       = 35,
    EFF_TARGET_PET_SUMMON_LOCATION                                = 36,
    EFF_TARGET_ALL_PARTY                                          = 37,
    EFF_TARGET_SCRIPTED_OR_SINGLE_TARGET                          = 38,
    EFF_TARGET_SELF_FISHING                                       = 39,
    EFF_TARGET_SCRIPTED_GAMEOBJECT                                = 40,
    EFF_TARGET_TOTEM_EARTH                                        = 41,
    EFF_TARGET_TOTEM_WATER                                        = 42,
    EFF_TARGET_TOTEM_AIR                                          = 43,
    EFF_TARGET_TOTEM_FIRE                                         = 44,
    EFF_TARGET_CHAIN                                              = 45,
    EFF_TARGET_SCIPTED_OBJECT_LOCATION                            = 46,
    EFF_TARGET_DYNAMIC_OBJECT                                     = 47, // not sure exactly where is used
    EFF_TARGET_MULTIPLE_SUMMON_LOCATION                           = 48,
    EFF_TARGET_MULTIPLE_SUMMON_PET_LOCATION                       = 49,
    EFF_TARGET_SUMMON_LOCATION                                    = 50,
    EFF_TARGET_CALIRI_EGS                                         = 51,
    EFF_TARGET_LOCATION_NEAR_CASTER                               = 52,
    EFF_TARGET_CURRENT_SELECTION                                  = 53,
    EFF_TARGET_TARGET_AT_ORIENTATION_TO_CASTER                    = 54,
    EFF_TARGET_LOCATION_INFRONT_CASTER                            = 55,
    EFF_TARGET_ALL_RAID                                           = 56,
    EFF_TARGET_PARTY_MEMBER                                       = 57,
    EFF_TARGET_TARGET_FOR_VISUAL_EFFECT                           = 59,
    EFF_TARGET_SCRIPTED_TARGET2                                   = 60,
    EFF_TARGET_AREAEFFECT_PARTY_AND_CLASS                         = 61,
    EFF_TARGET_PRIEST_CHAMPION                                    = 62, // wtf ?
    EFF_TARGET_NATURE_SUMMON_LOCATION                             = 63,
    EFF_TARGET_BEHIND_TARGET_LOCATION                             = 65,
    EFF_TARGET_MULTIPLE_GUARDIAN_SUMMON_LOCATION                  = 72,
    EFF_TARGET_NETHETDRAKE_SUMMON_LOCATION                        = 73,
    EFF_TARGET_SCRIPTED_LOCATION                                  = 74,
    EFF_TARGET_LOCATION_INFRONT_CASTER_AT_RANGE                   = 75,
    EFF_TARGET_ENEMIES_IN_AREA_CHANNELED_WITH_EXCEPTIONS          = 76,
    EFF_TARGET_SELECTED_ENEMY_CHANNELED                           = 77,
    EFF_TARGET_SELECTED_ENEMY_DEADLY_POISON                       = 86,
    EFF_TARGET_NON_COMBAT_PET                                     = 90,
    // these are custom, feel free to move them further if targeting gets extended
    EFF_TARGET_CUSTOM_PARTY_INJURED_SINGLE                        = 99,
    EFF_TARGET_CUSTOM_PARTY_INJURED_MULTI                         = 100,
    EFF_TARGET_CONE_IN_FRONT                                      = 104,
    EFF_TARGET_LIST_LENGTH_MARKER                                 = 111,
} SpellEffectTarget;
