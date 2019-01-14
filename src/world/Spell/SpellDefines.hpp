/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

struct DamageProc
{
    uint32_t m_spellId;
    uint32_t m_damage;
    //uint64 m_caster;          //log is: some reflects x arcane/nature damage to 'attacker' no matter who casted
    uint32_t m_school;
    uint32_t m_flags;
    void* owner;                //mark the owner of this proc to know which one to delete
};

struct SpellCharge
{
    uint32_t spellId;
    uint32_t count;
    uint32_t ProcFlag;
    uint32_t lastproc;
    uint32_t procdiff;
};

enum SpellAttributes
{
    ATTRIBUTES_NULL                                 = 0x00000000,
    ATTRIBUTES_UNK2                                 = 0x00000001,
    ATTRIBUTES_RANGED                               = 0x00000002,   // Ranged attack (Arcane shot, Serpent sting etc...)
    ATTRIBUTES_ON_NEXT_ATTACK                       = 0x00000004,
    ATTRIBUTES_UNK5                                 = 0x00000008,
    ATTRIBUTES_ABILITY                              = 0x00000010,
    ATTRIBUTES_TRADESPELL                           = 0x00000020,   // Tradeskill recipies
    ATTRIBUTES_PASSIVE                              = 0x00000040,
    ATTRIBUTES_NO_VISUAL_AURA                       = 0x00000080,   // not visible in spellbook or aura bar
    ATTRIBUTES_NO_CAST                              = 0x00000100,   //seems to be afflicts pet
    ATTRIBUTES_TARGET_MAINHAND                      = 0x00000200,   // automatically select item from mainhand
    ATTRIBUTES_ON_NEXT_SWING_2                      = 0x00000400,   //completely the same as ATTRIBUTE_ON_NEXT_ATTACK for class spells. So difference somewhere in mob abilities.
    ATTRIBUTES_UNK13                                = 0x00000800,
    ATTRIBUTES_DAY_ONLY                             = 0x00001000,   // No spells in 3.3.5a
    ATTRIBUTES_NIGHT_ONLY                           = 0x00002000,   // No spells in 3.3.5a
    ATTRIBUTES_ONLY_INDOORS                         = 0x00004000,   // No spells in 3.3.5a
    ATTRIBUTES_ONLY_OUTDOORS                        = 0x00008000,
    ATTRIBUTES_NOT_SHAPESHIFT                       = 0x00010000,   // Cannot be cast while shapeshifted
    ATTRIBUTES_REQ_STEALTH                          = 0x00020000,
    ATTRIBUTES_UNK20                                = 0x00040000,   //it's not : must be behind
    ATTRIBUTES_LEVEL_DAMAGE_CALCULATION             = 0x00080000,   ///\ todo: implement
    ATTRIBUTES_STOP_ATTACK                          = 0x00100000,   // Switches auto attack off (Gouge, Maim etc...)
    ATTRIBUTES_CANT_BE_DPB                          = 0x00200000,   // Cannot be dodged, parried or blocked
    ATTRIBUTES_UNK24                                = 0x00400000,   // related to ranged
    ATTRIBUTES_DEAD_CASTABLE                        = 0x00800000,   // Castable while dead
    ATTRIBUTES_MOUNT_CASTABLE                       = 0x01000000,   // Castable while mounted
    ATTRIBUTES_TRIGGER_COOLDOWN                     = 0x02000000,   //also requires atributes ex = 32 ?
    ATTRIBUTES_NEGATIVE                             = 0x04000000,   // Most negative spells have this attribute
    ATTRIBUTES_CASTABLE_WHILE_SITTING               = 0x08000000,
    ATTRIBUTES_REQ_OOC                              = 0x10000000,   // Requires caster to be out of combat
    ATTRIBUTES_IGNORE_INVULNERABILITY               = 0x20000000,   // debuffs that can't be removed by any spell and spells that can't be resisted in any case
    ATTRIBUTES_UNK32                                = 0x40000000,   ///\ todo: Heartbeat resist, not implemented
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
    ATTRIBUTESEX_REQ_FACING_TARGET                  = 0x00000200,   // 10 Requires caster to face target
    ATTRIBUTESEX_NO_INITIAL_AGGRO                   = 0x00000400,   // 11 guessed
    ATTRIBUTESEX_UNK13                              = 0x00000800,   // 12
    ATTRIBUTESEX_UNK14                              = 0x00001000,   // 13 related to pickpocket
    ATTRIBUTESEX_UNK15                              = 0x00002000,   // 14 related to remote control
    ATTRIBUTESEX_CHANNEL_FACE_TARGET                = 0x00004000,   // 15 Channeling makes you face target
    ATTRIBUTESEX_DISPEL_AURAS_ON_IMMUNITY           = 0x00008000,   // 16 remove auras on immunity - something like "grant immunity"
    ATTRIBUTESEX_UNAFFECTED_BY_SCHOOL_IMMUNE        = 0x00010000,   // 17 unaffected by school immunity - something like "grant immunity" too
    ATTRIBUTESEX_REMAIN_OOC                         = 0x00020000,   // 18
    ATTRIBUTESEX_UNK20                              = 0x00040000,   // 19
    ATTRIBUTESEX_CANT_TARGET_SELF                   = 0x00080000,   // 20
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
    ATTRIBUTESEXB_CAN_BE_CASTED_ON_DEAD_TARGET      = 0x00000001,   // 1 Can be casted on dead target
    ATTRIBUTESEXB_UNK3                              = 0x00000002,   // 2 Can be used while stealthed
    ATTRIBUTESEXB_IGNORE_LINE_OF_SIGHT              = 0x00000004,   // 3 Ignores Line of Sight
    ATTRIBUTESEXB_UNK5                              = 0x00000008,   // 4 something todo with temp enchanted items
    ATTRIBUTESEXB_PARTY_EFFECTING_AURA              = 0x00000010,   // 5 Party affecting aura's
    ATTRIBUTESEXB_AUTOREPEAT                        = 0x00000020,   // 6 Autorepeat spells (Auto Shot, Shoot wand)
    ATTRIBUTESEXB_CANT_TARGET_TAGGED                = 0x00000040,   // 7 Spells with this attribute cannot be casted on mobs tagged by another player
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
    ATTRIBUTESEXB_NOT_RESET_AUTO_ATTACKS            = 0x00020000,   // 18 Doesn't reset timers for melee or ranged autoattacks
    ATTRIBUTESEXB_REQ_DEAD_PET                      = 0x00040000,   // 19
    ATTRIBUTESEXB_NOT_NEED_SHAPESHIFT               = 0x00080000,   // 20 does not necessarily need shapeshift
    ATTRIBUTESEXB_REQ_BEHIND_TARGET                 = 0x00100000,   // 21
    ATTRIBUTESEXB_UNK23                             = 0x00200000,   // 22
    ATTRIBUTESEXB_UNK24                             = 0x00400000,   // 23
    ATTRIBUTESEXB_UNK25                             = 0x00800000,   // 24
    ATTRIBUTESEXB_UNK26                             = 0x01000000,   // 25
    ATTRIBUTESEXB_UNK27                             = 0x02000000,   // 26
    ATTRIBUTESEXB_UNAFFECTED_BY_SCHOOL_IMMUNITY     = 0x04000000,   // 27
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
    ATTRIBUTESEXC_TARGET_ONLY_PLAYERS               = 0x00000100,   // Requires player target
    ATTRIBUTESEXC_UNK11                             = 0x00000200,
    ATTRIBUTESEXC_REQUIRES_MAIN_HAND_WEAPON         = 0x00000400,   // Requires main hand weapon
    ATTRIBUTESEXC_BG_ONLY                           = 0x00000800,
    ATTRIBUTESEXC_TARGET_ONLY_GHOSTS                = 0x00001000,   // Requires ghost target
    ATTRIBUTESEXC_UNK15                             = 0x00002000,
    ATTRIBUTESEXC_UNK16                             = 0x00004000,
    ATTRIBUTESEXC_PLAYER_RANGED_SPELLS              = 0x00008000,
    ATTRIBUTESEXC_UNK18                             = 0x00010000,
    ATTRIBUTESEXC_UNK19                             = 0x00020000,
    ATTRIBUTESEXC_UNK20                             = 0x00040000,
    ATTRIBUTESEXC_UNK21                             = 0x00080000,   // e.g. Totemic mastery
    ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD = 0x00100000,
    ATTRIBUTESEXC_UNK23                             = 0x00200000,
    ATTRIBUTESEXC_REQUIRES_WAND                     = 0x00400000,   // Requires wand
    ATTRIBUTESEXC_UNK25                             = 0x00800000,
    ATTRIBUTESEXC_REQUIRES_OFFHAND_WEAPON           = 0x01000000,   // Requires offhand weapon
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
    ATTRIBUTESEXD_SHIV                              = 0x00000400,   // Only rogue's Shiv has this attribute in 3.3.5a
    ATTRIBUTESEXD_UNK8                              = 0x00000800,
    ATTRIBUTESEXD_UNK9                              = 0x00001000,
    ATTRIBUTESEXD_UNK10                             = 0x00002000,
    ATTRIBUTESEXD_NOT_BREAK_AURAS                   = 0x00004000,   // not breake auras by damage from this spell
    ATTRIBUTESEXD_UNK11                             = 0x00008000,
    ATTRIBUTESEXD_NOT_IN_ARENAS                     = 0x00010000,   // Cannot be used in arenas
    ATTRIBUTESEXD_CAN_BE_USED_IN_ARENAS             = 0x00020000,
    ATTRIBUTESEXD_UNK13                             = 0x00040000,
    ATTRIBUTESEXD_UNK14                             = 0x00080000,
    ATTRIBUTESEXD_UNK15                             = 0x00100000,
    ATTRIBUTESEXD_UNK16                             = 0x00200000,   // pala aura, dk presence, dudu form, warrior stance, shadowform, hunter track
    ATTRIBUTESEXD_UNK17                             = 0x00400000,
    ATTRIBUTESEXD_UNK18                             = 0x00800000,
    ATTRIBUTESEXD_UNK19                             = 0x01000000,
    ATTRIBUTESEXD_NOT_USED                          = 0x02000000,
    ATTRIBUTESEXD_ONLY_IN_OUTLANDS                  = 0x04000000,   // Flying mounts maybe?
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
    ATTRIBUTESEXE_REAGENT_REMOVAL                   = 0x00000002,   // if player has UNIT_FLAG_NO_REAGANT_COST, spells with this attribute won't use reagents
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
    ATTRIBUTESEXE_HASTE_AFFECTS_DURATION            = 0x00002000,
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
    ATTRIBUTESEXE_SKIP_LINE_OF_SIGHT_CHECK          = 0x04000000, // Used for spells which explode around target
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
    ATTRIBUTESEXF_NOT_IN_RAIDS_OR_HEROIC_DUNGEONS   = 0x00000800,   // Cannot be casted in raids or heroic dungeons
    ATTRIBUTESEXF_UNUSED4                           = 0x00001000,   // castable on vehicle
    ATTRIBUTESEXF_CAN_TARGET_INVISIBLE              = 0x00002000,
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
    ATTRIBUTESEXG_IGNORE_COLD_WEATHER_FLYING        = 0x00400000,
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

#if VERSION_STRING >= Cata
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
#endif

#define MAX_SPELL_EFFECTS 3
#define MAX_SPELL_TOTEMS 2
#define MAX_SPELL_TOTEM_CATEGORIES 2
#define MAX_SPELL_REAGENTS 8
#define MAX_SPELL_ID 121820
