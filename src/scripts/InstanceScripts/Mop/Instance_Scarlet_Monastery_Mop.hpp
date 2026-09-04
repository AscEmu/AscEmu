/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Scarlet Monastery (Mists revamp) - three encounters, fought in order per wowhead's dungeon
// guide: Thalnos the Soulrender -> Brother Korloff -> Commander Durand & High Inquisitor
// Whitemane (a duo fight sharing one boss-state slot). No prior scripting or dialogue data
// exists for this dungeon - all kits below are built from
// wowhead's MoP Classic data instead. Whitemane reuses her original vanilla NPC entry (3977),
// same as in the live game.

uint32_t const ScarletMonasteryMopEncounterCount = 3;

enum ScarletMonasteryMopData
{
    DATA_THALNOS              = 0,
    DATA_KORLOFF              = 1,
    DATA_DURAND_AND_WHITEMANE = 2
};

enum ScarletMonasteryMopCreatures
{
    BOSS_THALNOS             = 59789,
    BOSS_KORLOFF             = 59223,
    BOSS_DURAND              = 60040,
    BOSS_WHITEMANE           = 3977,

    // Trash - all three confirmed against an expanded reference database's own
    // placed spawns on this instance's map. None of their NPC pages carried ability data, so
    // their kits are thematically-reasonable rotations built from generic, verified-safe
    // spells rather than confirmed tooltips.
    NPC_SCARLET_FLAMETHROWER = 59705,
    NPC_FRENZIED_SPIRIT      = 60033,
    NPC_SCARLET_JUDICATOR    = 58605
};

enum ScarletFlamethrowerSpells
{
    SPELL_FLAMETHROWER_FIREBALL    = 133,   // stand-in: reuses "Fireball" rank 1
    SPELL_FLAMETHROWER_FLAMESTRIKE = 2120   // stand-in: reuses "Flamestrike" rank 1
};

enum FrenziedSpiritSpells
{
    SPELL_FRENZIED_SPIRIT_SHADOW_BOLT = 32860,  // stand-in: reuses the real "Shadow Bolt" spell
    SPELL_FRENZIED_SPIRIT_ENRAGE      = 8599    // stand-in: reuses the generic "Enrage" self-buff
};

enum ScarletJudicatorSpells
{
    SPELL_JUDICATOR_HOLY_SMITE        = 20695,  // stand-in: reuses the real "Holy Smite" spell
    SPELL_JUDICATOR_HAMMER_OF_JUSTICE = 853     // stand-in: reuses "Hammer of Justice" rank 1
};

// Thalnos' real fight also raises Fallen Crusader corpses that an Empowering Spirit can
// reanimate into tougher Empowered Zombies if left near the boss; the summon-and-reanimate
// add chain isn't replicated, only his direct-damage kit is.
enum ThalnosSpells
{
    SPELL_THALNOS_SPIRIT_GALE           = 115289,
    SPELL_THALNOS_EVICT_SOUL            = 115297,
    SPELL_THALNOS_RAISE_FALLEN_CRUSADER = 115139
};

enum KorloffSpells
{
    SPELL_KORLOFF_BLAZING_FISTS  = 114808,
    SPELL_KORLOFF_FIRESTORM_KICK = 113764,
    SPELL_KORLOFF_RISING_FLAME   = 114410,
    SPELL_KORLOFF_SCORCHED_EARTH = 114460
};

enum DurandSpells
{
    SPELL_DURAND_FLASH_OF_STEEL = 115629,
    SPELL_DURAND_DASHING_STRIKE = 115739
};

enum WhitemaneSpells
{
    SPELL_WHITEMANE_POWER_WORD_SHIELD = 17,
    SPELL_WHITEMANE_HOLY_SMITE        = 20695,
    SPELL_WHITEMANE_MASS_RESURRECTION = 113134
};
