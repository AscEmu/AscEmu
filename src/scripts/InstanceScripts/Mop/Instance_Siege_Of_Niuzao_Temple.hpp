/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Siege of Niuzao Temple - four bosses, fought in order per wowhead's dungeon guide: Vizier
// Jin'bak -> Commander Vo'jak -> General Pa'valak -> Wing Leader Ner'onok. No prior scripting
// or dialogue data exists for this dungeon - all kits
// below are built from wowhead's MoP Classic data instead.

uint32_t const SiegeOfNiuzaoTempleEncounterCount = 4;

enum SiegeOfNiuzaoTempleData
{
    DATA_JINBAK  = 0,
    DATA_VOJAK   = 1,
    DATA_PAVALAK = 2,
    DATA_NERONOK = 3
};

enum SiegeOfNiuzaoTempleCreatures
{
    BOSS_JINBAK             = 61567,
    BOSS_VOJAK              = 61634,
    BOSS_PAVALAK            = 61485,
    BOSS_NERONOK            = 62205,

    // Trash - this dungeon has no placed spawn data in either reference database, but these
    // two real, named creature templates sit in a dense "Sra'thik" mantid-military cluster
    // directly adjacent to Pa'valak's own confirmed entry (61485) and match this dungeon's
    // "mantid siege" theme closely enough to be a reasonably confident (if not
    // spawn-confirmed) match.
    NPC_SRATHIK_WARCALLER   = 61502,
    NPC_SRATHIK_FLESHRENDER = 61514
};

enum SrathikWarcallerSpells
{
    SPELL_WARCALLER_SHADOW_BOLT = 32860,  // stand-in: reuses the real "Shadow Bolt" spell
    SPELL_WARCALLER_CURSE       = 30910   // stand-in: reuses a generic curse debuff
};

enum SrathikFleshrenderSpells
{
    SPELL_FLESHRENDER_CLEAVE = 845,    // stand-in: reuses "Cleave" rank 1
    SPELL_FLESHRENDER_ENRAGE = 8599    // stand-in: reuses the generic "Enrage" self-buff
};

// Jin'bak's real fight revolves around a growing, players-must-manage Sap Puddle fed by
// summoned Sap Globules; the puddle-size/globule-merge mechanic isn't replicated, only his
// direct-damage/summon casts are.
enum JinbakSpells
{
    SPELL_JINBAK_SAP_PUDDLE  = 120593,
    SPELL_JINBAK_SAP_RESIDUE = 119941,
    SPELL_JINBAK_DETONATE    = 120001
};

enum VojakSpells
{
    SPELL_VOJAK_CAUSTIC_TAR     = 120778,
    SPELL_VOJAK_BOMBARD         = 120200,
    SPELL_VOJAK_DASHING_STRIKE  = 120789,
    SPELL_VOJAK_THOUSAND_BLADES = 120759
};

enum PavalakSpells
{
    SPELL_PAVALAK_BLADE_RUSH      = 124283,
    SPELL_PAVALAK_TEMPEST         = 119875,
    SPELL_PAVALAK_SIEGE_EXPLOSIVE = 119388,
    SPELL_PAVALAK_SERRATED_BLADE  = 119840
};

// Sourced directly from Wing Leader Ner'onok's own wowhead NPC page.
enum NeronokSpells
{
    SPELL_NERONOK_HURL_BRICK      = 121762,
    SPELL_NERONOK_CAUSTIC_PITCH   = 121442,
    SPELL_NERONOK_QUICK_DRY_RESIN = 121447,
    SPELL_NERONOK_GUSTING_WINDS   = 121284
};
