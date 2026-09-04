/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Throne of the Tides - four bosses, identities and mechanics verified against wowhead.

uint32_t const ThroneOfTidesEncounterCount = 4;

enum ThroneOfTidesData
{
    DATA_LADY_NAZJAR       = 0,
    DATA_COMMANDER_ULTHOK  = 1,
    DATA_MINDBENDER_GURSHA = 2,
    DATA_OZUMAT            = 3
};

enum ThroneOfTidesCreatures
{
    BOSS_LADY_NAZJAR        = 40586,
    BOSS_COMMANDER_ULTHOK   = 40765,
    BOSS_MINDBENDER_GURSHA  = 40788,
    BOSS_OZUMAT             = 44566,

    NPC_NAZJAR_SENTINEL     = 40577,
    NPC_NAZJAR_INVADER      = 39616,
    NPC_NAZJAR_SPIRITMENDER = 41096,
    NPC_TAINTED_SENTRY      = 40925,
    NPC_UNSTABLE_CORRUPTION = 40923
};

enum TrashSpells
{
    SPELL_SENTINEL_RANDOM            = 77218,
    SPELL_SENTINEL_VICTIM            = 76721,

    SPELL_INVADER_VICTIM             = 76807,
    SPELL_INVADER_LOW_HP             = 76813,

    SPELL_SPIRITMENDER_VICTIM        = 76815,
    SPELL_SPIRITMENDER_RANDOM        = 76820,
    SPELL_SPIRITMENDER_LOW_HP        = 76813,

    SPELL_TAINTED_SENTRY_SWELL       = 76634,
    SPELL_TAINTED_SENTRY_DEATH       = 76625,

    SPELL_UNSTABLE_CORRUPTION_GROWTH = 76362,
    SPELL_UNSTABLE_CORRUPTION_DEATH  = 76363
};

// Mindbender Ghur'sha is entirely database-driven in the reference (no AI scripting data) -
// there is nothing to port. Wowhead's own Encounter
// Journal data for this NPC id turned out to be the modern Timewalking-revamped version of
// the fight (its ability ids are in the 400000+ range, far outside any Cata-era id and
// unusable against a Cata client) rather than the original 4.0 kit, so - as with the other
// fully-unscripted bosses this pass - each ability below reuses a classic-era spell of the
// same name/effect as a stand-in. The original fight also involves a second body, Erunak
// Stonespeaker (possessed by Ghur'sha until 25% health), which isn't ported: we only
// spawn Ghur'sha himself here, so he runs the combined ability pool of both fight halves
// continuously from a single body instead.
enum MindbenderGurshaSpells
{
    SPELL_GURSHA_EARTHFURY         = 8385,   // stand-in: reuses "Volcanic Eruption" (ground AoE)
    SPELL_GURSHA_STORMFLURRY_TOTEM = 8599,   // stand-in: reuses "Enrage" for the totem's self-buff effect
    SPELL_GURSHA_FLAME_SHOCK       = 8050,   // real "Flame Shock" rank 1 (instant + DoT)
    SPELL_GURSHA_TERRIFYING_VISION = 5246,   // stand-in: reuses "Intimidating Shout" (fear + damage)
    SPELL_GURSHA_MIND_ROT          = 15537   // stand-in: reuses "Mind Flay" rank-equivalent (raid Shadow tick)
};

enum ThroneOfTidesGameObjects
{
    GO_ABYSSAL_MAW_DOOR_1 = 204338,
    GO_ABYSSAL_MAW_DOOR_2 = 204339,
    GO_ABYSSAL_MAW_DOOR_4 = 204341
};

enum LadyNazjarSpells
{
    SPELL_NAZJAR_SUMMON_GEYSER  = 75722,
    SPELL_NAZJAR_FUNGAL_SPORES  = 76001,
    SPELL_NAZJAR_SHOCK_BLAST_10 = 76008,
    SPELL_NAZJAR_SHOCK_BLAST_25 = 91477
};

enum UlthokSpells
{
    SPELL_ULTHOK_ENRAGE           = 76100,
    SPELL_ULTHOK_CURSE_OF_FATIGUE = 76094,
    SPELL_ULTHOK_DARK_FISSURE     = 76047,
    SPELL_ULTHOK_SQUEEZE_10       = 76026,
    SPELL_ULTHOK_SQUEEZE_25       = 91484
};

enum OzumatSpells
{
    // Ozumat itself is an elaborate scripted/gossip-triggered escort-defense event in the
    // original design, not a stand-up tab-target fight - simplified here into a straightforward
    // boss encounter using its own listed abilities.
    SPELL_OZUMAT_GLOBE_IMPACT_PERIODIC = 83126,
    SPELL_OZUMAT_BLIGHT_OF_OZUMAT      = 83585
};
