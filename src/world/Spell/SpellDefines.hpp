/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _SPELL_DEFINES_HPP
#define _SPELL_DEFINES_HPP


enum SpellAttributes
{
    ATTRIBUTES_NULL                                 = 0x00000000,
    ATTRIBUTES_UNK2                                 = 0x00000001,
    ATTRIBUTES_RANGED                               = 0x00000002,   // related to ranged??
    ATTRIBUTES_ON_NEXT_ATTACK                       = 0x00000004,
    ATTRIBUTES_UNK5                                 = 0x00000008,   // ATTRIBUTES_UNUSED0
    ATTRIBUTES_ABILITY                              = 0x00000010,
    ATTRIBUTES_TRADESPELL                           = 0x00000020,   // Tradeskill recipies
    ATTRIBUTES_PASSIVE                              = 0x00000040,
    ATTRIBUTES_NO_VISUAL_AURA                       = 0x00000080,   // not visible in spellbook or aura bar
    ATTRIBUTES_NO_CAST                              = 0x00000100,   //seems to be afflicts pet
    ATTRIBUTES_TARGET_MAINHAND                      = 0x00000200,   // automatically select item from mainhand
    ATTRIBUTES_ON_NEXT_SWING_2                      = 0x00000400,   //completely the same as ATTRIBUTE_ON_NEXT_ATTACK for class spells. So difference somewhere in mob abilities.
    ATTRIBUTES_UNK13                                = 0x00000800,
    ATTRIBUTES_DAY_ONLY                             = 0x00001000,
    ATTRIBUTES_NIGHT_ONLY                           = 0x00002000,
    ATTRIBUTES_ONLY_INDOORS                         = 0x00004000,
    ATTRIBUTES_ONLY_OUTDOORS                        = 0x00008000,
    ATTRIBUTES_NOT_SHAPESHIFT                       = 0x00010000,
    ATTRIBUTES_REQ_STEALTH                          = 0x00020000,
    ATTRIBUTES_UNK20                                = 0x00040000,   //it's not : must be behind
    ATTRIBUTES_LEVEL_DAMAGE_CALCULATION             = 0x00080000,
    ATTRIBUTES_STOP_ATTACK                          = 0x00100000,   //switch off auto attack on use. Maim,Gouge,Disengage,Polymorph etc
    ATTRIBUTES_CANT_BE_DPB                          = 0x00200000,   //can't be dodged, blocked, parried
    ATTRIBUTES_UNK24                                = 0x00400000,   // related to ranged
    ATTRIBUTES_DEAD_CASTABLE                        = 0x00800000,   //castable while dead
    ATTRIBUTES_MOUNT_CASTABLE                       = 0x01000000,   //castable on mounts
    ATTRIBUTES_TRIGGER_COOLDOWN                     = 0x02000000,   //also requires atributes ex = 32 ?
    ATTRIBUTES_NEGATIVE                             = 0x04000000,   // most negative spells have this attribute
    ATTRIBUTES_CASTABLE_WHILE_SITTING               = 0x08000000,
    ATTRIBUTES_REQ_OOC                              = 0x10000000,   // ATTRIBUTES_REQ_OUT_OF_COMBAT
    ATTRIBUTES_IGNORE_INVULNERABILITY               = 0x20000000,   // debuffs that can't be removed by any spell and spells that can't be resisted in any case
    ATTRIBUTES_UNK32                                = 0x40000000,   // seems like IS_DIMINISHING but some spells not there (f.e. Gouge)
    ATTRIBUTES_CANT_CANCEL                          = 0x80000000    // seems like aura is not removeable by CMSG_CANCEL_AURA
};

enum SpellAttributesEx
{
    ATTRIBUTESEX_NULL                               = 0x00000000,   // 0
    ATTRIBUTESEX_UNK2                               = 0x00000001,   // 1 pet summonings
    ATTRIBUTESEX_DRAIN_WHOLE_POWER                  = 0x00000002,   // 2 Uses all power / health
    ATTRIBUTESEX_CHANNELED_1                        = 0x00000004,   // 3 Channeled
    ATTRIBUTESEX_UNK5                               = 0x00000008,   // 4
    ATTRIBUTESEX_IGNORE_IN_FRONT                    = 0x00000010,   // 5 ignore verification isInFront() in unit::strike
    ATTRIBUTESEX_NOT_BREAK_STEALTH                  = 0x00000020,   // 6 does not break stealth
    ATTRIBUTESEX_CHANNELED_2                        = 0x00000040,   // 7 Channeled - [POSSIBLY: dynamite, grenades from engineering etc..]
    ATTRIBUTESEX_NEGATIVE                           = 0x00000080,   // 8
    ATTRIBUTESEX_REQ_OOC_TARGET                     = 0x00000100,   // 9 Spell req target should not be in combat
    ATTRIBUTESEX_UNK11                              = 0x00000200,   // 10
    ATTRIBUTESEX_NO_INITIAL_AGGRO                   = 0x00000400,   // 11 guessed
    ATTRIBUTESEX_UNK13                              = 0x00000800,   // 12
    ATTRIBUTESEX_UNK14                              = 0x00001000,   // 13 related to pickpocket
    ATTRIBUTESEX_UNK15                              = 0x00002000,   // 14 related to remote control
    ATTRIBUTESEX_UNK16                              = 0x00004000,   // 15
    ATTRIBUTESEX_DISPEL_AURAS_ON_IMMUNITY           = 0x00008000,   // 16 remove auras on immunity - something like "grant immunity"
    ATTRIBUTESEX_UNAFFECTED_BY_SCHOOL_IMMUNE        = 0x00010000,   // 17 unaffected by school immunity - something like "grant immunity" too
    ATTRIBUTESEX_REMAIN_OOC                         = 0x00020000,   // 18
    ATTRIBUTESEX_UNK20                              = 0x00040000,   // 19
    ATTRIBUTESEX_UNK21                              = 0x00080000,   // 20
    ATTRIBUTESEX_REQ_COMBO_POINTS1                  = 0x00100000,   // 21 related to "Finishing move" and "Instantly overpowers"
    ATTRIBUTESEX_UNK23                              = 0x00200000,   // 22
    ATTRIBUTESEX_REQ_COMBO_POINTS2                  = 0x00400000,   // 23 only related to "Finishing move"
    ATTRIBUTESEX_UNK25                              = 0x00800000,   // 24 related to spells like "ClearAllBuffs"
    ATTRIBUTESEX_UNK26                              = 0x01000000,   // 25 req fishing pole? FISHING SPELLS
    ATTRIBUTESEX_UNK27                              = 0x02000000,   // 26 related to "Detect" spell
    ATTRIBUTESEX_UNK28                              = 0x04000000,   // 27
    ATTRIBUTESEX_UNK29                              = 0x08000000,   // 28
    ATTRIBUTESEX_UNK30                              = 0x10000000,   // 29
    ATTRIBUTESEX_UNK31                              = 0x20000000,   // 30
    ATTRIBUTESEX_UNK32                              = 0x40000000,   // 31 Overpower
    ATTRIBUTESEX_UNK33                              = 0x80000000    // 32
};

enum SpellAttributesExB
{
    ATTRIBUTESEXB_NULL                              = 0x00000000,   // 0
    ATTRIBUTESEXB_UNK2                              = 0x00000001,   // 1
    ATTRIBUTESEXB_UNK3                              = 0x00000002,   // 2 Can be used while stealthed
    ATTRIBUTESEXB_UNK4                              = 0x00000004,   // 3 request pet maybe
    ATTRIBUTESEXB_UNK5                              = 0x00000008,   // 4 something todo with temp enchanted items
    ATTRIBUTESEXB_PARTY_EFFECTING_AURA              = 0x00000010,   // 5 Party affecting aura's
    ATTRIBUTESEXB_ACTIVATE_AUTO_SHOT                = 0x00000020,   // 6 spell that enable's auto shoot
    ATTRIBUTESEXB_UNK8                              = 0x00000040,   // 7 Polymorph spells
    ATTRIBUTESEXB_UNK9                              = 0x00000080,   // 8
    ATTRIBUTESEXB_UNUSED1                           = 0x00000100,   // 9 not set in 3.0.3
    ATTRIBUTESEXB_UNK11                             = 0x00000200,   // 10 used by 2 spells, 30421 | Nether Portal - Perseverence and  30466 | Nether Portal - Perseverence
    ATTRIBUTESEXB_TAME_X                            = 0x00000400,   // 11 tame [creature]
    ATTRIBUTESEXB_FUNNEL                            = 0x00000800,   // 12 only funnel spells
    ATTRIBUTESEXB_UNK14                             = 0x00001000,   // 13 swipe / Cleave spells
    ATTRIBUTESEXB_ENCHANT_OWN_ONLY                  = 0x00002000,   // 14 no trade window targets, BoE items get soulbound to you
    ATTRIBUTESEXB_SPELL_PLAYER_EVENT                = 0x00004000,   // 15 Player event's like logging in, finishing quests, triggering cinematic, being adored, Heartbroken etc
    ATTRIBUTESEXB_UNUSED3                           = 0x00008000,   // 16 not set in 3.0.3
    ATTRIBUTESEXB_CONTROL_UNIT                      = 0x00010000,   // 17 PvP Controller, RC, Creature taming, Taming Lesson
    ATTRIBUTESEXB_REQ_RANGED_WEAPON                 = 0x00020000,   // 18 used by hunters shot and stings... Possibly triggers autoshot?
    ATTRIBUTESEXB_REQ_DEAD_PET                      = 0x00040000,   // 19
    ATTRIBUTESEXB_NOT_NEED_SHAPESHIFT               = 0x00080000,   // 20 does not necessarily need shapeshift
    ATTRIBUTESEXB_REQ_BEHIND_TARGET                 = 0x00100000,   // 21
    ATTRIBUTESEXB_UNK23                             = 0x00200000,   // 22
    ATTRIBUTESEXB_UNK24                             = 0x00400000,   // 23
    ATTRIBUTESEXB_UNK25                             = 0x00800000,   // 24
    ATTRIBUTESEXB_UNK26                             = 0x01000000,   // 25
    ATTRIBUTESEXB_UNK27                             = 0x02000000,   // 26
    ATTRIBUTESEXB_UNK28                             = 0x04000000,   // 27
    ATTRIBUTESEXB_UNK29                             = 0x08000000,   // 28 fishing spells and enchanting weapons
    ATTRIBUTESEXB_UNK30                             = 0x10000000,   // 29 some secondairy spell triggers, especialy for lightning shield alike spells
    ATTRIBUTESEXB_CANT_CRIT                         = 0x20000000,   // 30 spell can't crit
    ATTRIBUTESEXB_UNK32                             = 0x40000000,   // 31
    ATTRIBUTESEXB_UNK33                             = 0x80000000    // 32
};

enum SpellAttributesExC
{
    ATTRIBUTESEXC_NULL                              = 0x00000000,
    ATTRIBUTESEXC_UNK2                              = 0x00000001,
    ATTRIBUTESEXC_UNK3                              = 0x00000002,
    ATTRIBUTESEXC_UNK4                              = 0x00000004,
    ATTRIBUTESEXC_UNK5                              = 0x00000008,
    ATTRIBUTESEXC_UNK6                              = 0x00000010,   // ignor resurrection
    ATTRIBUTESEXC_UNK7                              = 0x00000020,
    ATTRIBUTESEXC_UNK8                              = 0x00000040,
    ATTRIBUTESEXC_UNK9                              = 0x00000080,
    ATTRIBUTESEXC_UNK10                             = 0x00000100,
    ATTRIBUTESEXC_UNK11                             = 0x00000200,
    ATTRIBUTESEXC_UNK12                             = 0x00000400,
    ATTRIBUTESEXC_BG_ONLY                           = 0x00000800,
    ATTRIBUTESEXC_UNK14                             = 0x00001000,
    ATTRIBUTESEXC_UNK15                             = 0x00002000,
    ATTRIBUTESEXC_UNK16                             = 0x00004000,
    ATTRIBUTESEXC_PLAYER_RANGED_SPELLS              = 0x00008000,
    ATTRIBUTESEXC_UNK18                             = 0x00010000,
    ATTRIBUTESEXC_UNK19                             = 0x00020000,
    ATTRIBUTESEXC_UNK20                             = 0x00040000,
    ATTRIBUTESEXC_UNK21                             = 0x00080000,   // e.g. Totemic mastery
    ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD = 0x00100000,
    ATTRIBUTESEXC_UNK23                             = 0x00200000,
    ATTRIBUTESEXC_PLAYER_REQUIRED_WAND              = 0x00400000,   // required wand
    ATTRIBUTESEXC_UNK25                             = 0x00800000,
    ATTRIBUTESEXC_TYPE_OFFHAND                      = 0x01000000,   // required offhand
    ATTRIBUTESEXC_NO_HEALING_BONUS                  = 0x02000000,
    ATTRIBUTESEXC_CAN_PROC_ON_TRIGGERED             = 0x04000000,
    ATTRIBUTESEXC_DRAIN_SOUL                        = 0x08000000,   // just drain soul has this flag
    ATTRIBUTESEXC_UNK30                             = 0x10000000,
    ATTRIBUTESEXC_NO_DONE_BONUS                     = 0x20000000,   ///\todo used for checking spellpower/damage mods
    ATTRIBUTESEXC_NO_DISPLAY_RANGE                  = 0x40000000,   // tooltip dont show range
    ATTRIBUTESEXC_UNK33                             = 0x80000000
};

enum SpellAttributesExD
{
    ATTRIBUTESEXD_NULL                              = 0x00000000,
    ATTRIBUTESEXD_UNK1                              = 0x00000001,
    ATTRIBUTESEXD_PROCCHANCE_COMBOBASED             = 0x00000002,
    ATTRIBUTESEXD_UNK2                              = 0x00000004,
    ATTRIBUTESEXD_UNK3                              = 0x00000008,
    ATTRIBUTESEXD_UNK4                              = 0x00000010,
    ATTRIBUTESEXD_UNK5                              = 0x00000020,
    ATTRIBUTESEXD_NOT_STEALABLE                     = 0x00000040,
    ATTRIBUTESEXD_TRIGGERED                         = 0x00000080,   // spells forced to be triggered
    ATTRIBUTESEXD_UNK6                              = 0x00000100,
    ATTRIBUTESEXD_TRIGGER_ACTIVATE                  = 0x00000200,   // trigger activate (Deep Freeze...)
    ATTRIBUTESEXD_UNK7                              = 0x00000400,
    ATTRIBUTESEXD_UNK8                              = 0x00000800,
    ATTRIBUTESEXD_UNK9                              = 0x00001000,
    ATTRIBUTESEXD_UNK10                             = 0x00002000,
    ATTRIBUTESEXD_NOT_BREAK_AURAS                   = 0x00004000,   // not breake auras by damage from this spell
    ATTRIBUTESEXD_UNK11                             = 0x00008000,
    ATTRIBUTESEXD_NOT_IN_ARENA                      = 0x00010000,   // can not be used in arenas
    ATTRIBUTESEXD_UNK12                             = 0x00020000,   // can be used in arenas
    ATTRIBUTESEXD_UNK13                             = 0x00040000,
    ATTRIBUTESEXD_UNK14                             = 0x00080000,
    ATTRIBUTESEXD_UNK15                             = 0x00100000,
    ATTRIBUTESEXD_UNK16                             = 0x00200000,   // pala aura, dk presence, dudu form, warrior stance, shadowform, hunter track
    ATTRIBUTESEXD_UNK17                             = 0x00400000,
    ATTRIBUTESEXD_UNK18                             = 0x00800000,
    ATTRIBUTESEXD_UNK19                             = 0x01000000,
    ATTRIBUTESEXD_NOT_USED                          = 0x02000000,
    ATTRIBUTESEXD_ONLY_IN_OUTLANDS                  = 0x04000000,   // can be used only in outland
    ATTRIBUTESEXD_UNK20                             = 0x08000000,
    ATTRIBUTESEXD_UNK21                             = 0x10000000,
    ATTRIBUTESEXD_UNK22                             = 0x20000000,
    ATTRIBUTESEXD_UNK23                             = 0x40000000,
    ATTRIBUTESEXD_UNK24                             = 0x80000000
};

enum SpellAttributesExE
{
    ATTRIBUTESEXE_NULL                              = 0x00000000,
    ATTRIBUTESEXE_UNK2                              = 0x00000001,
    ATTRIBUTESEXE_REAGENT_REMOVAL                   = 0x00000002,
    ATTRIBUTESEXE_UNK4                              = 0x00000004,
    ATTRIBUTESEXE_USABLE_WHILE_STUNNED              = 0x00000008,   // usable while stunned
    ATTRIBUTESEXE_UNK6                              = 0x00000010,
    ATTRIBUTESEXE_SINGLE_TARGET_AURA                = 0x00000020,   // only one target can be applied
    ATTRIBUTESEXE_UNK8                              = 0x00000040,
    ATTRIBUTESEXE_UNK9                              = 0x00000080,
    ATTRIBUTESEXE_UNK10                             = 0x00000100,
    ATTRIBUTESEXE_UNK11                             = 0x00000200,   // periodic aura apply
    ATTRIBUTESEXE_HIDE_DURATION                     = 0x00000400,   // no duration for client
    ATTRIBUTESEXE_UNK13                             = 0x00000800,
    ATTRIBUTESEXE_UNK14                             = 0x00001000,
    ATTRIBUTESEXE_UNK15                             = 0x00002000,   // haste effect duration
    ATTRIBUTESEXE_UNK16                             = 0x00004000,
    ATTRIBUTESEXE_UNK17                             = 0x00008000,
    ATTRIBUTESEXE_ITEM_CLASS_CHECK                  = 0x00010000,   ///\todo this allows spells with EquippedItemClass to affect spells from other items if the required item is equipped
    ATTRIBUTESEXE_USABLE_WHILE_FEARED               = 0x00020000,   // usable while feared
    ATTRIBUTESEXE_USABLE_WHILE_CONFUSED             = 0x00040000,   // usable while confused
    ATTRIBUTESEXE_UNK21                             = 0x00080000,
    ATTRIBUTESEXE_UNK22                             = 0x00100000,
    ATTRIBUTESEXE_UNK23                             = 0x00200000,
    ATTRIBUTESEXE_UNK24                             = 0x00400000,
    ATTRIBUTESEXE_UNK25                             = 0x00800000,
    ATTRIBUTESEXE_UNK26                             = 0x01000000,
    ATTRIBUTESEXE_UNK27                             = 0x02000000,
    ATTRIBUTESEXE_UNK28                             = 0x04000000,
    ATTRIBUTESEXE_UNK29                             = 0x08000000,
    ATTRIBUTESEXE_UNK30                             = 0x10000000,
    ATTRIBUTESEXE_UNK31                             = 0x20000000,
    ATTRIBUTESEXE_UNK32                             = 0x40000000,
    ATTRIBUTESEXE_UNK33                             = 0x80000000
};

enum SpellAttributesExF
{
    ATTRIBUTESEXF_NULL                              = 0x00000000,
    ATTRIBUTESEXF_UNK2                              = 0x00000001,   // cooldown in tooltyp (not displayed)
    ATTRIBUTESEXF_UNUSED1                           = 0x00000002,   // only arena
    ATTRIBUTESEXF_UNK4                              = 0x00000004,
    ATTRIBUTESEXF_UNK5                              = 0x00000008,
    ATTRIBUTESEXF_UNK6                              = 0x00000010,
    ATTRIBUTESEXF_UNK7                              = 0x00000020,
    ATTRIBUTESEXF_UNK8                              = 0x00000040,
    ATTRIBUTESEXF_UNK9                              = 0x00000080,
    ATTRIBUTESEXF_UNK10                             = 0x00000100,
    ATTRIBUTESEXF_UNK11                             = 0x00000200,
    ATTRIBUTESEXF_UNK12                             = 0x00000400,
    ATTRIBUTESEXF_UNK13                             = 0x00000800,   // not in raid/instances
    ATTRIBUTESEXF_UNUSED4                           = 0x00001000,   // castable on vehicle
    ATTRIBUTESEXF_UNK15                             = 0x00002000,
    ATTRIBUTESEXF_UNUSED5                           = 0x00004000,
    ATTRIBUTESEXF_UNUSED6                           = 0x00008000,   // 54368, 67892
    ATTRIBUTESEXF_UNUSED7                           = 0x00010000,
    ATTRIBUTESEXF_UNK19                             = 0x00020000,   // Mountspells?
    ATTRIBUTESEXF_CAST_BY_CHARMER                   = 0x00040000,
    ATTRIBUTESEXF_UNK21                             = 0x00080000,
    ATTRIBUTESEXF_UNK22                             = 0x00100000,
    ATTRIBUTESEXF_UNK23                             = 0x00200000,
    ATTRIBUTESEXF_UNK24                             = 0x00400000,
    ATTRIBUTESEXF_UNK25                             = 0x00800000,
    ATTRIBUTESEXF_UNK26                             = 0x01000000,
    ATTRIBUTESEXF_UNK27                             = 0x02000000,
    ATTRIBUTESEXF_UNK28                             = 0x04000000,
    ATTRIBUTESEXF_UNK29                             = 0x08000000,
    ATTRIBUTESEXF_UNK30                             = 0x10000000,
    ATTRIBUTESEXF_UNK31                             = 0x20000000,
    ATTRIBUTESEXF_UNK32                             = 0x40000000,
    ATTRIBUTESEXF_UNK33                             = 0x80000000
};

enum SpellAttributesExG
{
    ATTRIBUTESEXG_NULL                              = 0x00000000,
    ATTRIBUTESEXG_UNK1                              = 0x00000001,
    ATTRIBUTESEXG_UNK2                              = 0x00000002,
    ATTRIBUTESEXG_UNK3                              = 0x00000004,
    ATTRIBUTESEXG_IS_CHEAT_SPELL                    = 0x00000008,
    ATTRIBUTESEXG_UNK5                              = 0x00000010,
    ATTRIBUTESEXG_UNK6                              = 0x00000020,   // shaman player totem summon?
    ATTRIBUTESEXG_UNK7                              = 0x00000040,
    ATTRIBUTESEXG_UNK8                              = 0x00000080,
    ATTRIBUTESEXG_UNK9                              = 0x00000100,
    ATTRIBUTESEXG_UNK10                             = 0x00000200,
    ATTRIBUTESEXG_UNK11                             = 0x00000400,
    ATTRIBUTESEXG_INTERRUPT_NPC                     = 0x00000800,   // non player character casts interruption
    ATTRIBUTESEXG_UNK13                             = 0x00001000,
    ATTRIBUTESEXG_UNK14                             = 0x00002000,
    ATTRIBUTESEXG_UNK15                             = 0x00004000,
    ATTRIBUTESEXG_UNK16                             = 0x00008000,
    ATTRIBUTESEXG_UNK17                             = 0x00010000,
    ATTRIBUTESEXG_UNK18                             = 0x00020000,
    ATTRIBUTESEXG_UNK19                             = 0x00040000,
    ATTRIBUTESEXG_UNK20                             = 0x00080000,
    ATTRIBUTESEXG_UNK21                             = 0x00100000,
    ATTRIBUTESEXG_UNK22                             = 0x00200000,
    ATTRIBUTESEXG_UNK23                             = 0x00400000,
    ATTRIBUTESEXG_UNK24                             = 0x00800000,
    ATTRIBUTESEXG_UNK25                             = 0x01000000,
    ATTRIBUTESEXG_UNK26                             = 0x02000000,
    ATTRIBUTESEXG_UNK27                             = 0x04000000,
    ATTRIBUTESEXG_UNK28                             = 0x08000000,
    ATTRIBUTESEXG_UNK29                             = 0x10000000,
    ATTRIBUTESEXG_UNK30                             = 0x20000000,
    ATTRIBUTESEXG_UNK31                             = 0x40000000,
    ATTRIBUTESEXG_UNK32                             = 0x80000000
};

enum SpellAttributesExH
{
    ATTRIBUTESEXH_UNK0                              = 0x00000001,   // 0
    ATTRIBUTESEXH_UNK1                              = 0x00000002,   // 1 
    ATTRIBUTESEXH_UNK2                              = 0x00000004,   // 2 
    ATTRIBUTESEXH_UNK3                              = 0x00000008,   // 3
    ATTRIBUTESEXH_UNK4                              = 0x00000010,   // 4
    ATTRIBUTESEXH_UNK5                              = 0x00000020,   // 5
    ATTRIBUTESEXH_UNK6                              = 0x00000040,   // 6 
    ATTRIBUTESEXH_UNK7                              = 0x00000080,   // 7
    ATTRIBUTESEXH_UNK8                              = 0x00000100,   // 8 
    ATTRIBUTESEXH_UNK9                              = 0x00000200,   // 9 
    ATTRIBUTESEXH_UNK10                             = 0x00000400,   // 10 
    ATTRIBUTESEXH_UNK11                             = 0x00000800,   // 11 
    ATTRIBUTESEXH_UNK12                             = 0x00001000,   // 12
    ATTRIBUTESEXH_UNK13                             = 0x00002000,   // 13
    ATTRIBUTESEXH_UNK14                             = 0x00004000,   // 14 
    ATTRIBUTESEXH_UNK15                             = 0x00008000,   // 15 
    ATTRIBUTESEXH_UNK16                             = 0x00010000,   // 16
    ATTRIBUTESEXH_UNK17                             = 0x00020000,   // 17
    ATTRIBUTESEXH_UNK18                             = 0x00040000,   // 18 
    ATTRIBUTESEXH_UNK19                             = 0x00080000,   // 19 
    ATTRIBUTESEXH_UNK20                             = 0x00100000,   // 20
    ATTRIBUTESEXH_UNK21                             = 0x00200000,   // 21 
    ATTRIBUTESEXH_UNK22                             = 0x00400000,   // 22 
    ATTRIBUTESEXH_UNK23                             = 0x00800000,   // 23 
    ATTRIBUTESEXH_UNK24                             = 0x01000000,   // 24 
    ATTRIBUTESEXH_UNK25                             = 0x02000000,   // 25 
    ATTRIBUTESEXH_UNK26                             = 0x04000000,   // 26 
    ATTRIBUTESEXH_UNK27                             = 0x08000000,   // 27
    ATTRIBUTESEXH_UNK28                             = 0x10000000,   // 28
    ATTRIBUTESEXH_UNK29                             = 0x20000000,   // 29
    ATTRIBUTESEXH_UNK30                             = 0x40000000,   // 30
    ATTRIBUTESEXH_UNK31                             = 0x80000000    // 31
};

enum SpellAttributesExI
{
    ATTRIBUTESEXI_UNK0                              = 0x00000001,   // 0
    ATTRIBUTESEXI_UNK1                              = 0x00000002,   // 1
    ATTRIBUTESEXI_UNK2                              = 0x00000004,   // 2
    ATTRIBUTESEXI_UNK3                              = 0x00000008,   // 3
    ATTRIBUTESEXI_UNK4                              = 0x00000010,   // 4
    ATTRIBUTESEXI_UNK5                              = 0x00000020,   // 5
    ATTRIBUTESEXI_UNK6                              = 0x00000040,   // 6
    ATTRIBUTESEXI_UNK7                              = 0x00000080,   // 7
    ATTRIBUTESEXI_UNK8                              = 0x00000100,   // 8
    ATTRIBUTESEXI_UNK9                              = 0x00000200,   // 9
    ATTRIBUTESEXI_UNK10                             = 0x00000400,   // 10
    ATTRIBUTESEXI_UNK11                             = 0x00000800,   // 11
    ATTRIBUTESEXI_UNK12                             = 0x00001000,   // 12
    ATTRIBUTESEXI_UNK13                             = 0x00002000,   // 13
    ATTRIBUTESEXI_UNK14                             = 0x00004000,   // 14
    ATTRIBUTESEXI_UNK15                             = 0x00008000,   // 15
    ATTRIBUTESEXI_UNK16                             = 0x00010000,   // 16
    ATTRIBUTESEXI_UNK17                             = 0x00020000,   // 17
    ATTRIBUTESEXI_UNK18                             = 0x00040000,   // 18
    ATTRIBUTESEXI_UNK19                             = 0x00080000,   // 19
    ATTRIBUTESEXI_UNK20                             = 0x00100000,   // 20
    ATTRIBUTESEXI_UNK21                             = 0x00200000,   // 21
    ATTRIBUTESEXI_UNK22                             = 0x00400000,   // 22
    ATTRIBUTESEXI_UNK23                             = 0x00800000,   // 23
    ATTRIBUTESEXI_UNK24                             = 0x01000000,   // 24
    ATTRIBUTESEXI_UNK25                             = 0x02000000,   // 25
    ATTRIBUTESEXI_UNK26                             = 0x04000000,   // 26
    ATTRIBUTESEXI_UNK27                             = 0x08000000,   // 27
    ATTRIBUTESEXI_UNK28                             = 0x10000000,   // 28
    ATTRIBUTESEXI_UNK29                             = 0x20000000,   // 29
    ATTRIBUTESEXI_UNK30                             = 0x40000000,   // 30
    ATTRIBUTESEXI_UNK31                             = 0x80000000    // 31
};

enum SpellAttributesExJ
{
    ATTRIBUTESEXJ_UNK0                              = 0x00000001,   // 0
    ATTRIBUTESEXJ_UNK1                              = 0x00000002,   // 1
    ATTRIBUTESEXJ_UNK2                              = 0x00000004,   // 2
    ATTRIBUTESEXJ_UNK3                              = 0x00000008,   // 3
    ATTRIBUTESEXJ_UNK4                              = 0x00000010,   // 4
    ATTRIBUTESEXJ_UNK5                              = 0x00000020,   // 5
    ATTRIBUTESEXJ_UNK6                              = 0x00000040,   // 6
    ATTRIBUTESEXJ_UNK7                              = 0x00000080,   // 7
    ATTRIBUTESEXJ_UNK8                              = 0x00000100,   // 8
    ATTRIBUTESEXJ_UNK9                              = 0x00000200,   // 9
    ATTRIBUTESEXJ_UNK10                             = 0x00000400,   // 10
    ATTRIBUTESEXJ_UNK11                             = 0x00000800,   // 11
    ATTRIBUTESEXJ_UNK12                             = 0x00001000,   // 12
    ATTRIBUTESEXJ_UNK13                             = 0x00002000,   // 13
    ATTRIBUTESEXJ_UNK14                             = 0x00004000,   // 14
    ATTRIBUTESEXJ_UNK15                             = 0x00008000,   // 15
    ATTRIBUTESEXJ_UNK16                             = 0x00010000,   // 16
    ATTRIBUTESEXJ_UNK17                             = 0x00020000,   // 17
    ATTRIBUTESEXJ_UNK18                             = 0x00040000,   // 18
    ATTRIBUTESEXJ_UNK19                             = 0x00080000,   // 19
    ATTRIBUTESEXJ_UNK20                             = 0x00100000,   // 20
    ATTRIBUTESEXJ_UNK21                             = 0x00200000,   // 21
    ATTRIBUTESEXJ_UNK22                             = 0x00400000,   // 22
    ATTRIBUTESEXJ_UNK23                             = 0x00800000,   // 23
    ATTRIBUTESEXJ_UNK24                             = 0x01000000,   // 24
    ATTRIBUTESEXJ_UNK25                             = 0x02000000,   // 25
    ATTRIBUTESEXJ_UNK26                             = 0x04000000,   // 26
    ATTRIBUTESEXJ_UNK27                             = 0x08000000,   // 27
    ATTRIBUTESEXJ_UNK28                             = 0x10000000,   // 28
    ATTRIBUTESEXJ_UNK29                             = 0x20000000,   // 29
    ATTRIBUTESEXJ_UNK30                             = 0x40000000,   // 30
    ATTRIBUTESEXJ_UNK31                             = 0x80000000    // 31
};

#pragma pack(push,1)

#define MAX_SPELL_EFFECTS 3
#define MAX_SPELL_TOTEMS 2
#define MAX_SPELL_TOTEM_CATEGORIES 2
#define MAX_SPELL_REAGENTS 8
#define MAX_SPELL_ID 121820

class Player;

struct SERVER_DECL SpellInfo
{
    // Applied
    uint32 Id;
    uint32 Attributes;
    uint32 AttributesEx;
    uint32 AttributesExB;
    uint32 AttributesExC;
    uint32 AttributesExD;
    uint32 AttributesExE;
    uint32 AttributesExF;
    uint32 AttributesExG;
    uint32 AttributesExH;
    uint32 AttributesExI;
    uint32 AttributesExJ;
    uint32 CastingTimeIndex;
    uint32 DurationIndex;
    uint32 powerType;
    uint32 rangeIndex;
    float speed;
    uint32 SpellVisual;
    uint32 field114;
    uint32 spellIconID;
    uint32 activeIconID;
    std::string Name;
    std::string Rank;
    std::string Description;
    std::string BuffDescription;
    uint32 School;
    uint32 RuneCostID;
    //uint32 SpellMissileID;
    //uint32 spellDescriptionVariableID;
    uint32 SpellDifficultyID;

    //dbc links
    uint32 SpellScalingId;                              // SpellScaling.dbc
    uint32 SpellAuraOptionsId;                          // SpellAuraOptions.dbc
    uint32 SpellAuraRestrictionsId;                     // SpellAuraRestrictions.dbc
    uint32 SpellCastingRequirementsId;                  // SpellCastingRequirements.dbc
    uint32 SpellCategoriesId;                           // SpellCategories.dbc
    uint32 SpellClassOptionsId;                         // SpellClassOptions.dbc
    uint32 SpellCooldownsId;                            // SpellCooldowns.dbc
    uint32 SpellEquippedItemsId;                        // SpellEquippedItems.dbc
    uint32 SpellInterruptsId;                           // SpellInterrupts.dbc
    uint32 SpellLevelsId;                               // SpellLevels.dbc
    uint32 SpellPowerId;                                // SpellPower.dbc
    uint32 SpellReagentsId;                             // SpellReagents.dbc
    uint32 SpellShapeshiftId;                           // SpellShapeshift.dbc
    uint32 SpellTargetRestrictionsId;                   // SpellTargetRestrictions.dbc
    uint32 SpellTotemsId;                               // SpellTotems.dbc

    // data from SpellScaling.dbc
    // data from SpellAuraOptions.dbc
    uint32 maxstack;
    uint32 procChance;
    uint32 procCharges;
    uint32 procFlags;

    // data from SpellAuraRestrictions.dbc
    uint32 CasterAuraState;
    uint32 TargetAuraState;
    uint32 CasterAuraStateNot;
    uint32 TargetAuraStateNot;
    uint32 casterAuraSpell;
    uint32 targetAuraSpell;
    uint32 casterAuraSpellNot;
    uint32 targetAuraSpellNot;

    // data from SpellCastingRequirements.dbc
    uint32 FacingCasterFlags;
    int32 RequiresAreaId;
    uint32 RequiresSpellFocus;

    // data from SpellCategories.dbc
    uint32 Category;
    uint32 DispelType;
    uint32 Spell_Dmg_Type;
    uint32 MechanicsType;
    uint32 PreventionType;
    uint32 StartRecoveryCategory;

    // data from SpellClassOptions.dbc
    uint32 SpellGroupType[3];
    uint32 SpellFamilyName;

    // data from SpellCooldowns.dbc
    uint32 CategoryRecoveryTime;
    uint32 RecoveryTime;
    uint32 StartRecoveryTime;

    // data from SpellEquippedItems.dbc
    int32 EquippedItemClass;
    int32 EquippedItemInventoryTypeMask;
    int32 EquippedItemSubClass;

    // data from SpellInterrupts.dbc
    uint32 AuraInterruptFlags;
    uint32 ChannelInterruptFlags;
    uint32 InterruptFlags;

    // data from SpellLevels.dbc
    uint32 baseLevel;
    uint32 maxLevel;
    uint32 spellLevel;

    // data from SpellPower.dbc
    uint32 manaCost;
    uint32 manaCostPerlevel;
    uint32 ManaCostPercentage;
    uint32 manaPerSecond;
    uint32 manaPerSecondPerLevel;

    // data from SpellReagents.dbc
    uint32 Reagent[MAX_SPELL_REAGENTS];
    uint32 ReagentCount[MAX_SPELL_REAGENTS];

    // data from SpellShapeshift.dbc
    uint32 RequiredShapeShift;
    uint32 ShapeshiftExclude;

    // data from SpellTargetRestrictions.dbc
    uint32 MaxTargets;
    uint32 MaxTargetLevel;
    uint32 TargetCreatureType;
    uint32 Targets;

    // data from SpellTotems.dbc
    uint32 TotemCategory[MAX_SPELL_TOTEM_CATEGORIES];
    uint32 Totem[MAX_SPELL_TOTEMS];
    
    // data from SpellEffect.dbc
    uint32 Effect[MAX_SPELL_EFFECTS];
    float EffectMultipleValue[MAX_SPELL_EFFECTS];
    uint32 EffectApplyAuraName[MAX_SPELL_EFFECTS];
    uint32 EffectAmplitude[MAX_SPELL_EFFECTS];
    int32 EffectBasePoints[MAX_SPELL_EFFECTS];
    float EffectBonusMultiplier[MAX_SPELL_EFFECTS];
    float dmg_multiplier[MAX_SPELL_EFFECTS];
    uint32 EffectChainTarget[MAX_SPELL_EFFECTS];
    int32 EffectDieSides[MAX_SPELL_EFFECTS];
    uint32 EffectItemType[MAX_SPELL_EFFECTS];
    uint32 EffectMechanic[MAX_SPELL_EFFECTS];
    int32 EffectMiscValue[MAX_SPELL_EFFECTS];
    int32 EffectMiscValueB[MAX_SPELL_EFFECTS];
    float EffectPointsPerComboPoint[MAX_SPELL_EFFECTS];
    uint32 EffectRadiusIndex[MAX_SPELL_EFFECTS];
    uint32 EffectRadiusMaxIndex[MAX_SPELL_EFFECTS];
    float EffectRealPointsPerLevel[MAX_SPELL_EFFECTS];
    uint32 EffectSpellClassMask[MAX_SPELL_EFFECTS];
    uint32 EffectTriggerSpell[MAX_SPELL_EFFECTS];
    uint32 EffectImplicitTargetA[MAX_SPELL_EFFECTS];
    uint32 EffectImplicitTargetB[MAX_SPELL_EFFECTS];
    uint32 EffectSpellId[MAX_SPELL_EFFECTS];
    uint32 EffectIndex[MAX_SPELL_EFFECTS];

    //custom values
    uint32 custom_DiminishStatus;
    uint32 custom_proc_interval;
    uint32 custom_BGR_one_buff_on_target;
    uint32 custom_BGR_one_buff_from_caster_on_self;
    uint32 custom_c_is_flags;
    uint32 custom_RankNumber;
    uint32 custom_NameHash;
    uint32 custom_ThreatForSpell;
    float custom_ThreatForSpellCoef;
    uint32 custom_ProcOnNameHash[MAX_SPELL_EFFECTS];
    uint32 custom_spell_coef_flags;
    float custom_base_range_or_radius_sqr;
    float cone_width;
    float casttime_coef;
    float fixed_dddhcoef;
    float fixed_hotdotcoef;
    float Dspell_coef_override;
    float OTspell_coef_override;
    int ai_target_type;
    bool custom_self_cast_only;
    bool custom_apply_on_shapeshift_change;
    bool custom_always_apply;
    bool custom_is_melee_spell;
    bool custom_is_ranged_spell;
    bool CheckLocation(uint32 map_id, uint32 zone_id, uint32 area_id, Player* player = NULL);
    uint32 custom_SchoolMask;
    uint32 CustomFlags;
    uint32 EffectCustomFlag[MAX_SPELL_EFFECTS];

    void* (*SpellFactoryFunc);
    void* (*AuraFactoryFunc);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note bool HasEffect   - Tells if the Spell has a certain effect
    bool HasEffect(uint32 effect)
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (Effect[i] == effect)
                return true;

        return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note bool HasCustomFlagForEffect - Tells if the Spell has this flag for this effect
    bool HasCustomFlagForEffect(uint32 effect, uint32 flag)
    {
        if (effect >= MAX_SPELL_EFFECTS)
            return false;

        if ((EffectCustomFlag[effect] & flag) != 0)
            return true;
        else
            return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note bool AppliesAura  - Tells if the Spell applies this Aura
    bool AppliesAura(uint32 aura)
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {

            if ((Effect[i] == 6 ||        /// SPELL_EFFECT_APPLY_GROUP_AREA_AURA
                 Effect[i] == 27 ||    /// SPELL_EFFECT_PERSISTENT_AREA_AURA
                 Effect[i] == 35 ||    /// SPELL_EFFECT_APPLY_GROUP_AREA_AURA
                 Effect[i] == 65 ||    /// SPELL_EFFECT_APPLY_RAID_AREA_AURA
                 Effect[i] == 119 ||   /// SPELL_EFFECT_APPLY_PET_AREA_AURA
                 Effect[i] == 128 ||   /// SPELL_EFFECT_APPLY_FRIEND_AREA_AURA
                 Effect[i] == 129 ||   /// SPELL_EFFECT_APPLY_ENEMY_AREA_AURA
                 Effect[i] == 143) &&  /// SPELL_EFFECT_APPLY_OWNER_AREA_AURA
                EffectApplyAuraName[i] == aura)
                return true;
        }

        return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note uint32 GetAAEffectId()  - Returns the Effect Id of the Area Aura effect if the spell has one.
    uint32 GetAAEffectId()
    {

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {

            if (Effect[i] == 35 ||        /// SPELL_EFFECT_APPLY_GROUP_AREA_AURA
                Effect[i] == 65 ||    /// SPELL_EFFECT_APPLY_RAID_AREA_AURA
                Effect[i] == 119 ||   /// SPELL_EFFECT_APPLY_PET_AREA_AURA
                Effect[i] == 128 ||   /// SPELL_EFFECT_APPLY_FRIEND_AREA_AURA
                Effect[i] == 129 ||   /// SPELL_EFFECT_APPLY_ENEMY_AREA_AURA
                Effect[i] == 143)     /// SPELL_EFFECT_APPLY_OWNER_AREA_AURA
                return Effect[i];
        }

        return 0;
    }

    SpellInfo()
    {
        Id = 0;
        Category = 0;
        DispelType = 0;
        MechanicsType = 0;
        Attributes = 0;
        AttributesEx = 0;
        AttributesExB = 0;
        AttributesExC = 0;
        AttributesExD = 0;
        AttributesExE = 0;
        AttributesExF = 0;
        AttributesExG = 0;
        AttributesExH = 0;
        AttributesExI = 0;
        AttributesExJ = 0;
        CastingTimeIndex = 0;
        DurationIndex = 0;
        powerType = 0;
        rangeIndex = 0;
        speed = 0;
        SpellVisual = 0;
        field114 = 0;
        spellIconID = 0;
        activeIconID = 0;
        Name = "";
        Rank = "";
        Description = "";
        BuffDescription = "";
        School = 0;
        RuneCostID = 0;
        SpellDifficultyID = 0;

        //dbc links
        SpellScalingId = 0;
        SpellAuraOptionsId = 0;
        SpellAuraRestrictionsId = 0;
        SpellCastingRequirementsId = 0;
        SpellCategoriesId = 0;
        SpellClassOptionsId = 0;
        SpellCooldownsId = 0;
        SpellEquippedItemsId = 0;
        SpellInterruptsId = 0;
        SpellLevelsId = 0;
        SpellPowerId = 0;
        SpellReagentsId = 0;
        SpellShapeshiftId = 0;
        SpellTargetRestrictionsId = 0;
        SpellTotemsId = 0;

        // data from SpellScaling.dbc
        // data from SpellAuraOptions.dbc
        maxstack = 0;
        procChance = 0;
        procCharges = 0;
        procFlags = 0;

        // data from SpellAuraRestrictions.dbc
        CasterAuraState = 0;
        TargetAuraState = 0;
        CasterAuraStateNot = 0;
        TargetAuraStateNot = 0;
        casterAuraSpell = 0;
        targetAuraSpell = 0;
        casterAuraSpellNot = 0;
        targetAuraSpellNot = 0;

        // data from SpellCastingRequirements.dbc
        FacingCasterFlags = 0;
        RequiresAreaId = 0;
        RequiresSpellFocus = 0;

        // data from SpellCategories.dbc
        Category = 0;
        DispelType = 0;
        Spell_Dmg_Type = 0;
        MechanicsType = 0;
        PreventionType = 0;
        StartRecoveryCategory = 0;

        // data from SpellClassOptions.dbc
        SpellFamilyName = 0;
        for (uint8 i = 0; i < 3; ++i)
            SpellGroupType[i] = 0;

        // data from SpellCooldowns.dbc
        CategoryRecoveryTime = 0;
        RecoveryTime = 0;
        StartRecoveryTime = 0;

        // data from SpellEquippedItems.dbc
        EquippedItemClass = 0;
        EquippedItemInventoryTypeMask = 0;
        EquippedItemSubClass = 0;

        // data from SpellInterrupts.dbc
        AuraInterruptFlags = 0;
        ChannelInterruptFlags = 0;
        InterruptFlags = 0;

        // data from SpellLevels.dbc
        baseLevel = 0;
        maxLevel = 0;
        spellLevel = 0;

        // data from SpellPower.dbc
        manaCost = 0;
        manaCostPerlevel = 0;
        ManaCostPercentage = 0;
        manaPerSecond = 0;
        manaPerSecondPerLevel = 0;

        // data from SpellReagents.dbc
        for (uint8 i = 0; i < MAX_SPELL_REAGENTS; ++i)
        {
            Reagent[i] = 0;
            ReagentCount[i] = 0;
        }

        // data from SpellShapeshift.dbc
        RequiredShapeShift = 0;
        ShapeshiftExclude = 0;

        // data from SpellTargetRestrictions.dbc
        MaxTargets = 0;
        MaxTargetLevel = 0;
        TargetCreatureType = 0;
        Targets = 0;

        // data from SpellTotems.dbc
        for (uint8 i = 0; i < MAX_SPELL_TOTEMS; ++i)
        {
            TotemCategory[i] = 0;
            Totem[i] = 0;
        }

        // data from SpellEffect.dbc
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            Effect[i] = 0;
            EffectMultipleValue[i] = 0;
            EffectApplyAuraName[i] = 0;
            EffectAmplitude[i] = 0;
            EffectBasePoints[i] = 0;
            EffectBonusMultiplier[i] = 0;
            dmg_multiplier[i] = 0;
            EffectChainTarget[i] = 0;
            EffectDieSides[i] = 0;
            EffectItemType[i] = 0;
            EffectMechanic[i] = 0;
            EffectMiscValue[i] = 0;
            EffectMiscValueB[i] = 0;
            EffectPointsPerComboPoint[i] = 0;
            EffectRadiusIndex[i] = 0;
            EffectRadiusMaxIndex[i] = 0;
            EffectRealPointsPerLevel[i] = 0;
            EffectSpellClassMask[i] = 0;
            EffectTriggerSpell[i] = 0;
            EffectImplicitTargetA[i] = 0;
            EffectImplicitTargetB[i] = 0;
            EffectSpellId[i] = 0;
            EffectIndex[i] = 0;
        }

        // custom values
        custom_DiminishStatus = 0;
        custom_proc_interval = 0;
        custom_BGR_one_buff_on_target = 0;
        custom_BGR_one_buff_from_caster_on_self = 0;
        custom_c_is_flags = 0;
        custom_RankNumber = 0;
        custom_NameHash = 0;
        custom_ThreatForSpell = 0;
        custom_ThreatForSpellCoef = 0;
        custom_spell_coef_flags = 0;
        custom_base_range_or_radius_sqr = 0;
        cone_width = 0;
        casttime_coef = 0;
        fixed_dddhcoef = 0;
        fixed_hotdotcoef = 0;
        Dspell_coef_override = 0;
        OTspell_coef_override = 0;
        ai_target_type = 0;

        custom_self_cast_only = false;
        custom_apply_on_shapeshift_change = false;
        custom_always_apply = false;
        custom_is_melee_spell = false;
        custom_is_ranged_spell = false;
        custom_SchoolMask = 0;
        CustomFlags = 0;

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            custom_ProcOnNameHash[i] = 0;
            EffectCustomFlag[i] = 0;
        }

        SpellFactoryFunc = nullptr;
        AuraFactoryFunc = nullptr;
    }
};

#pragma pack(pop)

#endif  //_SPELL_DEFINES_HPP
