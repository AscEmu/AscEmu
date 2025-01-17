/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

struct DamageProc
{
    uint32_t m_spellId;
    uint32_t m_damage;
    // uint64_t m_caster;           // log is: some reflects x arcane/nature damage to 'attacker' no matter who casted
    uint32_t m_school;
    uint32_t m_flags;
    void* owner;                    // mark the owner of this proc to know which one to delete
};

enum SpellAttributes : uint32_t
{
    ATTRIBUTES_NULL                                 = 0x00000000,
    ATTRIBUTES_UNK2                                 = 0x00000001,
    ATTRIBUTES_RANGED                               = 0x00000002,   // Ranged attack (Arcane shot, Serpent sting etc...)
    ATTRIBUTES_ON_NEXT_ATTACK                       = 0x00000004,
    ATTRIBUTES_UNK5                                 = 0x00000008,
    ATTRIBUTES_ABILITY                              = 0x00000010,
    ATTRIBUTES_TRADESPELL                           = 0x00000020,   // Tradeskill recipies
    ATTRIBUTES_PASSIVE                              = 0x00000040,
    ATTRIBUTES_NO_VISUAL_AURA                       = 0x00000080,   // Not visible in spellbook or aura bar. Client handles this by itself.
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

enum SpellAttributesEx : uint32_t
{
    ATTRIBUTESEX_NULL                               = 0x00000000,   // 0
    ATTRIBUTESEX_DISMISS_CURRENT_PET                = 0x00000001,   // 1 Dismisses current pet
    ATTRIBUTESEX_DRAIN_WHOLE_POWER                  = 0x00000002,   // 2 Uses all power / health
    ATTRIBUTESEX_CHANNELED_1                        = 0x00000004,   // 3 Channeled
    ATTRIBUTESEX_CANT_BE_REFLECTED                  = 0x00000008,   // 4
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
    ATTRIBUTESEX_DISPEL_AURAS_ON_IMMUNITY           = 0x00008000,   // 16 Removes and grants immunity to a mechanic (Blink (stun), Divine Shield, Ice Block etc)
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
    ATTRIBUTESEX_AUTOCASTED_AT_SPELL_LEARN          = 0x80000000    // 32 Spells with this attribute and learn effect should be auto casted when learnt
};

enum SpellAttributesExB : uint32_t
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
    ATTRIBUTESEXB_TAME_BEAST                        = 0x00000400,   // 11 Hunter's Tame Beast and its pre-quest spells
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

enum SpellAttributesExC : uint32_t
{
    ATTRIBUTESEXC_NULL                              = 0x00000000,   // 0
    ATTRIBUTESEXC_UNK2                              = 0x00000001,   // 1
    ATTRIBUTESEXC_UNK3                              = 0x00000002,   // 2
    ATTRIBUTESEXC_UNK4                              = 0x00000004,   // 3
    ATTRIBUTESEXC_UNK5                              = 0x00000008,   // 4
    ATTRIBUTESEXC_IGNORE_RESURRECTION_TIMER         = 0x00000010,   // 5 ignor resurrection
    ATTRIBUTESEXC_UNK7                              = 0x00000020,   // 6
    ATTRIBUTESEXC_UNK8                              = 0x00000040,   // 7
    ATTRIBUTESEXC_APPLY_OWN_STACK_FOR_EACH_CASTER   = 0x00000080,   // 8 Applies separate aura stack for each caster
    ATTRIBUTESEXC_TARGET_ONLY_PLAYERS               = 0x00000100,   // 9 Requires player target
    ATTRIBUTESEXC_UNK11                             = 0x00000200,   // 10
    ATTRIBUTESEXC_REQUIRES_MAIN_HAND_WEAPON         = 0x00000400,   // 11 Requires main hand weapon
    ATTRIBUTESEXC_BG_ONLY                           = 0x00000800,   // 12
    ATTRIBUTESEXC_TARGET_ONLY_GHOSTS                = 0x00001000,   // 13 Requires ghost target
    ATTRIBUTESEXC_UNK15                             = 0x00002000,   // 14
    ATTRIBUTESEXC_UNK16                             = 0x00004000,   // 15
    ATTRIBUTESEXC_PLAYER_RANGED_SPELLS              = 0x00008000,   // 16
    ATTRIBUTESEXC_UNK18                             = 0x00010000,   // 17
    ATTRIBUTESEXC_UNK19                             = 0x00020000,   // 18
    ATTRIBUTESEXC_UNK20                             = 0x00040000,   // 19 probably these spells cannot be missed or resisted
    ATTRIBUTESEXC_UNK21                             = 0x00080000,   // 20 e.g. Totemic mastery
    ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD = 0x00100000,   // 21
    ATTRIBUTESEXC_UNK23                             = 0x00200000,   // 22
    ATTRIBUTESEXC_REQUIRES_WAND                     = 0x00400000,   // 23 Requires wand
    ATTRIBUTESEXC_UNK25                             = 0x00800000,   // 24
    ATTRIBUTESEXC_REQUIRES_OFFHAND_WEAPON           = 0x01000000,   // 25 Requires offhand weapon
    ATTRIBUTESEXC_NO_HEALING_BONUS                  = 0x02000000,   // 26
    ATTRIBUTESEXC_CAN_PROC_ON_TRIGGERED             = 0x04000000,   // 27
    ATTRIBUTESEXC_DRAIN_SOUL                        = 0x08000000,   // 28 just drain soul has this flag
    ATTRIBUTESEXC_HIGH_PRIORITY                     = 0x10000000,   // 29 Takes highest priority to cast/do effect
    ATTRIBUTESEXC_NO_DONE_BONUS                     = 0x20000000,   // 30 TODO: used for checking spellpower/damage mods
    ATTRIBUTESEXC_NO_DISPLAY_RANGE                  = 0x40000000,   // 31 tooltip dont show range
    ATTRIBUTESEXC_UNK33                             = 0x80000000    // 32
};

enum SpellAttributesExD : uint32_t
{
    ATTRIBUTESEXD_NULL                              = 0x00000000,   // 0
    ATTRIBUTESEXD_UNK1                              = 0x00000001,   // 1
    ATTRIBUTESEXD_PROCCHANCE_COMBOBASED             = 0x00000002,   // 2
    ATTRIBUTESEXD_UNK2                              = 0x00000004,   // 3
    ATTRIBUTESEXD_UNK3                              = 0x00000008,   // 4
    ATTRIBUTESEXD_UNK4                              = 0x00000010,   // 5
    ATTRIBUTESEXD_UNK5                              = 0x00000020,   // 6
    ATTRIBUTESEXD_NOT_STEALABLE                     = 0x00000040,   // 7
    ATTRIBUTESEXD_TRIGGERED                         = 0x00000080,   // 8 spells forced to be triggered
    ATTRIBUTESEXD_UNK6                              = 0x00000100,   // 9
    ATTRIBUTESEXD_TRIGGER_ACTIVATE                  = 0x00000200,   // 10 trigger activate (Deep Freeze...)
    ATTRIBUTESEXD_SHIV                              = 0x00000400,   // 11 Only rogue's Shiv has this attribute in 3.3.5a
    ATTRIBUTESEXD_UNK8                              = 0x00000800,   // 12
    ATTRIBUTESEXD_UNK9                              = 0x00001000,   // 13
    ATTRIBUTESEXD_UNK10                             = 0x00002000,   // 14
    ATTRIBUTESEXD_NOT_BREAK_AURAS                   = 0x00004000,   // 15 not breake auras by damage from this spell
    ATTRIBUTESEXD_UNK11                             = 0x00008000,   // 16
    ATTRIBUTESEXD_NOT_IN_ARENAS                     = 0x00010000,   // 17 Cannot be used in arenas
    ATTRIBUTESEXD_CAN_BE_USED_IN_ARENAS             = 0x00020000,   // 18
    ATTRIBUTESEXD_UNK13                             = 0x00040000,   // 19
    ATTRIBUTESEXD_UNK14                             = 0x00080000,   // 20
    ATTRIBUTESEXD_UNK15                             = 0x00100000,   // 21
    ATTRIBUTESEXD_UNK16                             = 0x00200000,   // 22 pala aura, dk presence, dudu form, warrior stance, shadowform, hunter track
    ATTRIBUTESEXD_UNK17                             = 0x00400000,   // 23
    ATTRIBUTESEXD_UNK18                             = 0x00800000,   // 24
    ATTRIBUTESEXD_UNK19                             = 0x01000000,   // 25
    ATTRIBUTESEXD_NOT_USED                          = 0x02000000,   // 26
    ATTRIBUTESEXD_ONLY_IN_OUTLANDS                  = 0x04000000,   // 27 Flying mounts maybe?
    ATTRIBUTESEXD_UNK20                             = 0x08000000,   // 28
    ATTRIBUTESEXD_UNK21                             = 0x10000000,   // 29
    ATTRIBUTESEXD_UNK22                             = 0x20000000,   // 30
    ATTRIBUTESEXD_UNK23                             = 0x40000000,   // 31
    ATTRIBUTESEXD_UNK24                             = 0x80000000    // 32
};

enum SpellAttributesExE : uint32_t
{
    ATTRIBUTESEXE_NULL                              = 0x00000000,   // 0
    ATTRIBUTESEXE_CAN_MOVE_WHILE_CHANNELING         = 0x00000001,   // 1 Can move while channeling
    ATTRIBUTESEXE_REAGENT_REMOVAL                   = 0x00000002,   // 2 if player has UNIT_FLAG_NO_REAGANT_COST, spells with this attribute won't use reagents
    ATTRIBUTESEXE_UNK4                              = 0x00000004,   // 3
    ATTRIBUTESEXE_USABLE_WHILE_STUNNED              = 0x00000008,   // 4 usable while stunned
    ATTRIBUTESEXE_UNK6                              = 0x00000010,   // 5
    ATTRIBUTESEXE_SINGLE_TARGET_AURA                = 0x00000020,   // 6 only one target can be applied
    ATTRIBUTESEXE_UNK8                              = 0x00000040,   // 7
    ATTRIBUTESEXE_UNK9                              = 0x00000080,   // 8
    ATTRIBUTESEXE_UNK10                             = 0x00000100,   // 9
    ATTRIBUTESEXE_UNK11                             = 0x00000200,   // 10 periodic aura apply
    ATTRIBUTESEXE_HIDE_DURATION                     = 0x00000400,   // 11 no duration for client
    ATTRIBUTESEXE_UNK13                             = 0x00000800,   // 12
    ATTRIBUTESEXE_UNK14                             = 0x00001000,   // 13
    ATTRIBUTESEXE_HASTE_AFFECTS_DURATION            = 0x00002000,   // 14
    ATTRIBUTESEXE_UNK16                             = 0x00004000,   // 15
    ATTRIBUTESEXE_UNK17                             = 0x00008000,   // 16
    ATTRIBUTESEXE_ITEM_CLASS_CHECK                  = 0x00010000,   // 17 TODO: this allows spells with EquippedItemClass to affect spells from other items if the required item is equipped
    ATTRIBUTESEXE_USABLE_WHILE_FEARED               = 0x00020000,   // 18 usable while feared
    ATTRIBUTESEXE_USABLE_WHILE_CONFUSED             = 0x00040000,   // 19 usable while confused
    ATTRIBUTESEXE_UNK21                             = 0x00080000,   // 20
    ATTRIBUTESEXE_UNK22                             = 0x00100000,   // 21
    ATTRIBUTESEXE_UNK23                             = 0x00200000,   // 22
    ATTRIBUTESEXE_UNK24                             = 0x00400000,   // 23
    ATTRIBUTESEXE_UNK25                             = 0x00800000,   // 24
    ATTRIBUTESEXE_UNK26                             = 0x01000000,   // 25
    ATTRIBUTESEXE_UNK27                             = 0x02000000,   // 26
    ATTRIBUTESEXE_SKIP_LINE_OF_SIGHT_CHECK          = 0x04000000,   // 27 Used for spells which explode around target
    ATTRIBUTESEXE_HIDE_AURA_ON_SELF_CAST            = 0x08000000,   // 28 Client handles this by itself
    ATTRIBUTESEXE_HIDE_AURA_ON_NON_SELF_CAST        = 0x10000000,   // 29 Client handles this by itself
    ATTRIBUTESEXE_UNK31                             = 0x20000000,   // 30
    ATTRIBUTESEXE_UNK32                             = 0x40000000,   // 31
    ATTRIBUTESEXE_UNK33                             = 0x80000000    // 32
};

enum SpellAttributesExF : uint32_t
{
    ATTRIBUTESEXF_NULL                              = 0x00000000,   // 0
    ATTRIBUTESEXF_UNK2                              = 0x00000001,   // 1 cooldown in tooltyp (not displayed)
    ATTRIBUTESEXF_UNUSED1                           = 0x00000002,   // 2 only arena
    ATTRIBUTESEXF_IGNORE_CASTER_STATE_AND_AURAS     = 0x00000004,   // 3
    ATTRIBUTESEXF_UNK5                              = 0x00000008,   // 4
    ATTRIBUTESEXF_UNK6                              = 0x00000010,   // 5
    ATTRIBUTESEXF_UNK7                              = 0x00000020,   // 6
    ATTRIBUTESEXF_UNK8                              = 0x00000040,   // 7
    ATTRIBUTESEXF_UNK9                              = 0x00000080,   // 8
    ATTRIBUTESEXF_UNK10                             = 0x00000100,   // 9
    ATTRIBUTESEXF_UNK11                             = 0x00000200,   // 10
    ATTRIBUTESEXF_UNK12                             = 0x00000400,   // 11
    ATTRIBUTESEXF_NOT_IN_RAIDS_OR_HEROIC_DUNGEONS   = 0x00000800,   // 12 Cannot be casted in raids or heroic dungeons
    ATTRIBUTESEXF_UNUSED4                           = 0x00001000,   // 13 castable on vehicle
    ATTRIBUTESEXF_CAN_TARGET_INVISIBLE              = 0x00002000,   // 14
    ATTRIBUTESEXF_UNUSED5                           = 0x00004000,   // 15
    ATTRIBUTESEXF_UNUSED6                           = 0x00008000,   // 16 54368, 67892
    ATTRIBUTESEXF_UNUSED7                           = 0x00010000,   // 17
    ATTRIBUTESEXF_UNK19                             = 0x00020000,   // 18 Mountspells?
    ATTRIBUTESEXF_CAST_BY_CHARMER                   = 0x00040000,   // 19
    ATTRIBUTESEXF_UNK21                             = 0x00080000,   // 20
    ATTRIBUTESEXF_UNK22                             = 0x00100000,   // 21
    ATTRIBUTESEXF_UNK23                             = 0x00200000,   // 22
    ATTRIBUTESEXF_UNK24                             = 0x00400000,   // 23
    ATTRIBUTESEXF_UNK25                             = 0x00800000,   // 24
    ATTRIBUTESEXF_UNK26                             = 0x01000000,   // 25
    ATTRIBUTESEXF_UNK27                             = 0x02000000,   // 26
    ATTRIBUTESEXF_UNK28                             = 0x04000000,   // 27
    ATTRIBUTESEXF_UNK29                             = 0x08000000,   // 28
    ATTRIBUTESEXF_UNK30                             = 0x10000000,   // 29
    ATTRIBUTESEXF_UNK31                             = 0x20000000,   // 30
    ATTRIBUTESEXF_UNK32                             = 0x40000000,   // 31
    ATTRIBUTESEXF_UNK33                             = 0x80000000    // 32
};

enum SpellAttributesExG : uint32_t
{
    ATTRIBUTESEXG_NULL                              = 0x00000000,   // 0
    ATTRIBUTESEXG_UNK1                              = 0x00000001,   // 1
    ATTRIBUTESEXG_UNK2                              = 0x00000002,   // 2
    ATTRIBUTESEXG_UNK3                              = 0x00000004,   // 3
    ATTRIBUTESEXG_IS_CHEAT_SPELL                    = 0x00000008,   // 4
    ATTRIBUTESEXG_UNK5                              = 0x00000010,   // 5
    ATTRIBUTESEXG_UNK6                              = 0x00000020,   // 6 shaman player totem summon?
    ATTRIBUTESEXG_UNK7                              = 0x00000040,   // 7
    ATTRIBUTESEXG_UNK8                              = 0x00000080,   // 8
    ATTRIBUTESEXG_UNK9                              = 0x00000100,   // 9
    ATTRIBUTESEXG_UNK10                             = 0x00000200,   // 10
    ATTRIBUTESEXG_UNK11                             = 0x00000400,   // 11
    ATTRIBUTESEXG_INTERRUPT_NPC                     = 0x00000800,   // 12 non player character casts interruption
    ATTRIBUTESEXG_UNK13                             = 0x00001000,   // 13
    ATTRIBUTESEXG_UNK14                             = 0x00002000,   // 14
    ATTRIBUTESEXG_UNK15                             = 0x00004000,   // 15
    ATTRIBUTESEXG_UNK16                             = 0x00008000,   // 16
    ATTRIBUTESEXG_UNK17                             = 0x00010000,   // 17
    ATTRIBUTESEXG_UNK18                             = 0x00020000,   // 18
    ATTRIBUTESEXG_UNK19                             = 0x00040000,   // 19
    ATTRIBUTESEXG_UNK20                             = 0x00080000,   // 20
    ATTRIBUTESEXG_UNK21                             = 0x00100000,   // 21
    ATTRIBUTESEXG_UNK22                             = 0x00200000,   // 22
    ATTRIBUTESEXG_IGNORE_COLD_WEATHER_FLYING        = 0x00400000,   // 23
    ATTRIBUTESEXG_UNK24                             = 0x00800000,   // 24
    ATTRIBUTESEXG_UNK25                             = 0x01000000,   // 25
    ATTRIBUTESEXG_UNK26                             = 0x02000000,   // 26
    ATTRIBUTESEXG_UNK27                             = 0x04000000,   // 27
    ATTRIBUTESEXG_UNK28                             = 0x08000000,   // 28
    ATTRIBUTESEXG_UNK29                             = 0x10000000,   // 29
    ATTRIBUTESEXG_UNK30                             = 0x20000000,   // 30
    ATTRIBUTESEXG_UNK31                             = 0x40000000,   // 31
    ATTRIBUTESEXG_UNK32                             = 0x80000000    // 32
};

enum SpellAttributesExH : uint32_t
{
    ATTRIBUTESEXH_NULL                              = 0x00000000,   // 0
    ATTRIBUTESEXH_UNK0                              = 0x00000001,   // 1
    ATTRIBUTESEXH_UNK1                              = 0x00000002,   // 2
    ATTRIBUTESEXH_UNK2                              = 0x00000004,   // 3
    ATTRIBUTESEXH_UNK3                              = 0x00000008,   // 4
    ATTRIBUTESEXH_UNK4                              = 0x00000010,   // 5
    ATTRIBUTESEXH_UNK5                              = 0x00000020,   // 6
    ATTRIBUTESEXH_UNK6                              = 0x00000040,   // 7
    ATTRIBUTESEXH_UNK7                              = 0x00000080,   // 8
    ATTRIBUTESEXH_UNK8                              = 0x00000100,   // 9
    ATTRIBUTESEXH_UNK9                              = 0x00000200,   // 10
    ATTRIBUTESEXH_UNK10                             = 0x00000400,   // 11
    ATTRIBUTESEXH_UNK11                             = 0x00000800,   // 12
    ATTRIBUTESEXH_SEND_AURA_EFFECT_AMOUNT           = 0x00001000,   // 13 sends aura effect values in SMSG_SEND_AURA_UPDATE
    ATTRIBUTESEXH_UNK13                             = 0x00002000,   // 14
    ATTRIBUTESEXH_UNK14                             = 0x00004000,   // 15
    ATTRIBUTESEXH_UNK15                             = 0x00008000,   // 16
    ATTRIBUTESEXH_UNK16                             = 0x00010000,   // 17
    ATTRIBUTESEXH_UNK17                             = 0x00020000,   // 18
    ATTRIBUTESEXH_UNK18                             = 0x00040000,   // 19
    ATTRIBUTESEXH_UNK19                             = 0x00080000,   // 20
    ATTRIBUTESEXH_UNK20                             = 0x00100000,   // 21
    ATTRIBUTESEXH_UNK21                             = 0x00200000,   // 22
    ATTRIBUTESEXH_UNK22                             = 0x00400000,   // 23
    ATTRIBUTESEXH_UNK23                             = 0x00800000,   // 24
    ATTRIBUTESEXH_UNK24                             = 0x01000000,   // 25
    ATTRIBUTESEXH_UNK25                             = 0x02000000,   // 26
    ATTRIBUTESEXH_UNK26                             = 0x04000000,   // 27
    ATTRIBUTESEXH_UNK27                             = 0x08000000,   // 28
    ATTRIBUTESEXH_UNK28                             = 0x10000000,   // 29
    ATTRIBUTESEXH_UNK29                             = 0x20000000,   // 30
    ATTRIBUTESEXH_UNK30                             = 0x40000000,   // 31
    ATTRIBUTESEXH_UNK31                             = 0x80000000    // 32
};

enum SpellAttributesExI : uint32_t
{
    ATTRIBUTESEXI_NULL                              = 0x00000000,   // 0
    ATTRIBUTESEXI_UNK0                              = 0x00000001,   // 1
    ATTRIBUTESEXI_UNK1                              = 0x00000002,   // 2
    ATTRIBUTESEXI_UNK2                              = 0x00000004,   // 3
    ATTRIBUTESEXI_UNK3                              = 0x00000008,   // 4
    ATTRIBUTESEXI_UNK4                              = 0x00000010,   // 5
    ATTRIBUTESEXI_UNK5                              = 0x00000020,   // 6
    ATTRIBUTESEXI_UNK6                              = 0x00000040,   // 7
    ATTRIBUTESEXI_UNK7                              = 0x00000080,   // 8
    ATTRIBUTESEXI_UNK8                              = 0x00000100,   // 9
    ATTRIBUTESEXI_UNK9                              = 0x00000200,   // 10
    ATTRIBUTESEXI_UNK10                             = 0x00000400,   // 11
    ATTRIBUTESEXI_UNK11                             = 0x00000800,   // 12
    ATTRIBUTESEXI_UNK12                             = 0x00001000,   // 13
    ATTRIBUTESEXI_UNK13                             = 0x00002000,   // 14
    ATTRIBUTESEXI_UNK14                             = 0x00004000,   // 15
    ATTRIBUTESEXI_UNK15                             = 0x00008000,   // 16
    ATTRIBUTESEXI_UNK16                             = 0x00010000,   // 17
    ATTRIBUTESEXI_UNK17                             = 0x00020000,   // 18
    ATTRIBUTESEXI_UNK18                             = 0x00040000,   // 19
    ATTRIBUTESEXI_UNK19                             = 0x00080000,   // 20
    ATTRIBUTESEXI_UNK20                             = 0x00100000,   // 21
    ATTRIBUTESEXI_UNK21                             = 0x00200000,   // 22
    ATTRIBUTESEXI_UNK22                             = 0x00400000,   // 23
    ATTRIBUTESEXI_UNK23                             = 0x00800000,   // 24
    ATTRIBUTESEXI_UNK24                             = 0x01000000,   // 25
    ATTRIBUTESEXI_UNK25                             = 0x02000000,   // 26
    ATTRIBUTESEXI_UNK26                             = 0x04000000,   // 27
    ATTRIBUTESEXI_UNK27                             = 0x08000000,   // 28
    ATTRIBUTESEXI_UNK28                             = 0x10000000,   // 29
    ATTRIBUTESEXI_UNK29                             = 0x20000000,   // 30
    ATTRIBUTESEXI_UNK30                             = 0x40000000,   // 31
    ATTRIBUTESEXI_UNK31                             = 0x80000000    // 32
};

enum SpellAttributesExJ : uint32_t
{
    ATTRIBUTESEXJ_NULL                              = 0x00000000,   // 0
    ATTRIBUTESEXJ_UNK0                              = 0x00000001,   // 1
    ATTRIBUTESEXJ_UNK1                              = 0x00000002,   // 2
    ATTRIBUTESEXJ_UNK2                              = 0x00000004,   // 3
    ATTRIBUTESEXJ_UNK3                              = 0x00000008,   // 4
    ATTRIBUTESEXJ_UNK4                              = 0x00000010,   // 5
    ATTRIBUTESEXJ_UNK5                              = 0x00000020,   // 6
    ATTRIBUTESEXJ_UNK6                              = 0x00000040,   // 7
    ATTRIBUTESEXJ_UNK7                              = 0x00000080,   // 8
    ATTRIBUTESEXJ_UNK8                              = 0x00000100,   // 9
    ATTRIBUTESEXJ_UNK9                              = 0x00000200,   // 10
    ATTRIBUTESEXJ_UNK10                             = 0x00000400,   // 11
    ATTRIBUTESEXJ_UNK11                             = 0x00000800,   // 12
    ATTRIBUTESEXJ_UNK12                             = 0x00001000,   // 13
    ATTRIBUTESEXJ_UNK13                             = 0x00002000,   // 14
    ATTRIBUTESEXJ_UNK14                             = 0x00004000,   // 15
    ATTRIBUTESEXJ_UNK15                             = 0x00008000,   // 16
    ATTRIBUTESEXJ_UNK16                             = 0x00010000,   // 17
    ATTRIBUTESEXJ_UNK17                             = 0x00020000,   // 18
    ATTRIBUTESEXJ_UNK18                             = 0x00040000,   // 19
    ATTRIBUTESEXJ_UNK19                             = 0x00080000,   // 20
    ATTRIBUTESEXJ_UNK20                             = 0x00100000,   // 21
    ATTRIBUTESEXJ_UNK21                             = 0x00200000,   // 22
    ATTRIBUTESEXJ_UNK22                             = 0x00400000,   // 23
    ATTRIBUTESEXJ_UNK23                             = 0x00800000,   // 24
    ATTRIBUTESEXJ_UNK24                             = 0x01000000,   // 25
    ATTRIBUTESEXJ_UNK25                             = 0x02000000,   // 26
    ATTRIBUTESEXJ_UNK26                             = 0x04000000,   // 27
    ATTRIBUTESEXJ_UNK27                             = 0x08000000,   // 28
    ATTRIBUTESEXJ_UNK28                             = 0x10000000,   // 29
    ATTRIBUTESEXJ_UNK29                             = 0x20000000,   // 30
    ATTRIBUTESEXJ_UNK30                             = 0x40000000,   // 31
    ATTRIBUTESEXJ_UNK31                             = 0x80000000    // 32
};

enum SpellRangeTypeMask : uint8_t
{
    SPELL_RANGE_TYPE_MASK_MELEE                     = 1,
    SPELL_RANGE_TYPE_MASK_RANGED                    = 2
};

enum ProcEvents : uint8_t
{
    PROC_EVENT_DO_CASTER_PROCS_ONLY                 = 0, // Handles only procs which proc on self
    PROC_EVENT_DO_TARGET_PROCS_ONLY                 = 1, // Handles only procs which proc on target
    PROC_EVENT_DO_ALL                               = 2  // Handles all procs
};

enum SpellVisualIds : uint32_t
{
    SPELL_VISUAL_FOOD                               = 406,
    SPELL_VISUAL_DRINK                              = 438,
};

// todo: move this to each dbc structure file
#define MAX_SPELL_ID 121820
