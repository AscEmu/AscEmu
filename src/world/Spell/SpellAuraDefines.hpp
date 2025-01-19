/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <set>

enum AURA_INTERNAL_USAGE_FLAGS
{
    // if all 3 mods are resisted then we can send client as a fully resisted spell.
    // don't change the value of these !
    MOD_0_RESISTED      = 1,
    MOD_1_RESISTED      = 2,
    MOD_2_RESISTED      = 4
};

enum AuraTickFlags
{
    FLAG_PERIODIC_NONE              = 0,
    FLAG_PERIODIC_DAMAGE            = 2,
    FLAG_PERIODIC_TRIGGER_SPELL     = 4,
    FLAG_PERIODIC_HEAL              = 8,
    FLAG_PERIODIC_LEECH             = 16,
    FLAG_PERIODIC_ENERGIZE          = 32
};

struct ProcTriggerSpellOnSpell
{
    uint32_t origId;
    uint32_t spellId;
    uint64_t caster;
    uint32_t procChance;
    uint32_t procFlags;
    uint32_t RemainingCharges;
    uint32_t LastTrigger;
    void* owner;                            // mark the owner of this proc to know which one to delete
};

struct DamageSplitTarget
{
    uint64_t m_target;                      // we store them
    uint32_t m_spellId;
    float m_pctDamageSplit;                 // % of taken damage to transfer (i.e. Soul Link)
    uint32_t m_flatDamageSplit;             // flat damage to transfer (i.e. Blessing of Sacrifice)
    uint8_t damage_type;                    // bitwise 0-127 thingy
    void* creator;
};

typedef std::set<uint64_t> AreaAuraList;

// 4-bit flag
enum AuraUpdateFlags : uint8_t
{
    AFLAG_EMPTY                 = 0x00,
    AFLAG_EFFECT_1              = 0x01,
    AFLAG_EFFECT_2              = 0x02,
    AFLAG_EFFECT_3              = 0x04,
    AFLAG_IS_CASTER             = 0x08,
    AFLAG_SET                   = 0x09,
    AFLAG_CANCELLABLE           = 0x10,
    AFLAG_DURATION              = 0x20,
#if VERSION_STRING < Cata
    AFLAG_HIDE                  = 0x40,     // Seems to hide the aura and tell client the aura was removed
#else
    AFLAG_SEND_EFFECT_AMOUNT    = 0x40,     // used with AFLAG_EFFECT_0/1/2
#endif
    AFLAG_NEGATIVE              = 0x80,

    AFLAG_MASK_ALL              = 0xFF
};
