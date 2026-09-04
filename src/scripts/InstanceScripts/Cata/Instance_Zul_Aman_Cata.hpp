/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Zul'Aman (Cataclysm heroic revamp) - six bosses. Identities verified against wowhead.
// Nalorakk, Halazzi, Hexlord Malacrass and Daakara are entirely database-driven in the
// reference (no AI scripting data) - there is nothing to
// port, so their abilities below are built from wowhead's Encounter Journal / strategy guide
// text instead. Each carries an elaborate multi-form transformation mechanic (Nalorakk's
// troll/bear alternation is kept, since it's a simple timed swap; Halazzi's split into two
// separate beings, Malacrass' random 4-of-8 minion pool, and Daakara's random shapeshift
// phases are all dropped in favor of a continuous rotation from the base form's kit). Since no
// real Cata spell id could be recovered for the reused generic-sounding abilities, several
// reuse a classic-era spell of the same name/effect as a stand-in (noted per spell).

uint32_t const ZulAmanCataEncounterCount = 6;

enum ZulAmanCataData
{
    DATA_ALKILZON          = 0,
    DATA_NALORAKK          = 1,
    DATA_JANALAI           = 2,
    DATA_HALAZZI           = 3,
    DATA_HEXLORD_MALACRASS = 4,
    DATA_DAAKARA           = 5
};

enum ZulAmanCataCreatures
{
    BOSS_AKILZON           = 23574,
    BOSS_NALORAKK          = 23576,
    BOSS_JANALAI           = 23578,
    BOSS_HALAZZI           = 23577,
    BOSS_HEXLORD_MALACRASS = 24239,
    BOSS_DAAKARA           = 23863,

    NPC_AMANISHI_GUARDIAN  = 23597,
    NPC_AMANISHI_SCOUT     = 23586,
    NPC_AMANI_ELDER_LYNX   = 24530

    // The Zandalari Juggernaut/Hierophant/Archon trash (52956/52958/52962) also spawns in
    // this instance; their CreatureAIScript is registered once, in Instance_Zul_Gurub_Cata.cpp,
    // and applies here automatically since our creature script registry is keyed by
    // entry id, not by map.
};

enum AmanishiGuardianSpells
{
    SPELL_AMANISHI_GUARDIAN_REND = 43246
};

enum ZulAmanCataTrashSpells
{
    SPELL_AMANISHI_SCOUT_CURSE  = 42177,
    SPELL_AMANISHI_SCOUT_AMBUSH = 16496,
    SPELL_AMANISHI_SCOUT_SHOOT  = 43205,

    SPELL_ELDER_LYNX_ENRAGE     = 34970,
    SPELL_ELDER_LYNX_CLAW       = 43357,
    SPELL_ELDER_LYNX_BITE       = 43356
};

// Per wowhead: the four animal-god avatars (Akil'zon, Nalorakk, Jan'alai, Halazzi) can be
// fought in any order; Hex Lord Malacrass then Daakara follow.
enum ZulAmanCataGameObjects
{
    GO_ZULAMAN_WIND_DOOR   = 186858,
    GO_LYNX_TEMPLE_EXIT    = 186303,
    GO_HEXLORD_ENTRANCE    = 186305,
    GO_HEXLORD_WOODEN_DOOR = 186306,
    GO_MASSIVE_GATE        = 186728
};

enum AkilzonSpells
{
    SPELL_AKILZON_STATIC_DISRUPTION = 43622,
    SPELL_AKILZON_CALL_LIGHTNING    = 43661,
    SPELL_AKILZON_GUST_OF_WIND      = 43621,
    SPELL_AKILZON_ELECTRICAL_STORM  = 43648
};

enum NalorakkSpells
{
    // Troll form
    SPELL_NALORAKK_BRUTAL_STRIKE    = 40598,
    SPELL_NALORAKK_SURGE            = 40601,
    // Bear form
    SPELL_NALORAKK_BEAR_FORM        = 42181,
    SPELL_NALORAKK_REND_FLESH       = 48724,
    SPELL_NALORAKK_LACERATING_SLASH = 40598,  // stand-in: reuses Brutal Strike's id (Physical bleed)
    SPELL_NALORAKK_DEAFENING_ROAR   = 41130
};

enum HalazziSpells
{
    SPELL_HALAZZI_ENRAGE          = 32964,
    SPELL_HALAZZI_WATER_TOTEM     = 43263,
    SPELL_HALAZZI_LIGHTNING_TOTEM = 43436,
    SPELL_HALAZZI_EARTH_SHOCK     = 8042,   // stand-in: reuses classic "Earth Shock" rank 1
    SPELL_HALAZZI_FLAME_SHOCK     = 8050,   // stand-in: reuses classic "Flame Shock" rank 1
    SPELL_HALAZZI_SHRED_ARMOR     = 41623
};

enum HexlordMalacrassSpells
{
    SPELL_MALACRASS_SPIRIT_BOLTS = 40467,
    SPELL_MALACRASS_SIPHON_SOUL  = 40997,
    SPELL_MALACRASS_DRAIN_POWER  = 40903
};

enum DaakaraSpells
{
    SPELL_DAAKARA_WHIRLWIND      = 15576,
    SPELL_DAAKARA_GRIEVOUS_THROW = 97639
};

enum JanalaiSpells
{
    SPELL_JANALAI_FLAME_BREATH = 43140,
    SPELL_JANALAI_FRENZY       = 44779,
    SPELL_JANALAI_FIRE_BOMB    = 42621
};
