/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Firelands - seven bosses. Identities and spell ids verified against wowhead; Beth'tilac and
// Shannox in particular have no hand-scripted mechanics in the Cata reference implementation
// at all (empty AIName/ScriptName, no AI scripting data rows), so their abilities here come
// straight from wowhead rather than a ported reference script.

uint32_t const FirelandsEncounterCount = 7;

enum FirelandsData
{
    DATA_BETHTILAC          = 0,
    DATA_LORD_RHYOLITH      = 1,
    DATA_SHANNOX            = 2,
    DATA_ALYSRAZOR          = 3,
    DATA_BALEROC            = 4,
    DATA_MAJORDOMO_STAGHELM = 5,
    DATA_RAGNAROS           = 6
};

// Per wowhead's Firelands progression: Beth'tilac and Shannox (either order) unlock Lord
// Rhyolith and Alysrazor (either order); those two unlock Baleroc; Baleroc unlocks Majordomo
// Staghelm; Majordomo Staghelm unlocks Ragnaros.
enum FirelandsGameObjects
{
    GO_BALEROC_DOOR         = 209066,
    GO_STAGHELM_FIRE_WALL_1 = 208873,
    GO_STAGHELM_FIRE_WALL_2 = 208906,
    GO_SULFURON_KEEP_DOOR   = 209073

    // GO_STICKY_WEB (208877) is part of Beth'tilac's own web-climb fight mechanic, not a
    // progression gate, and isn't tracked. The Firelands Bridge / Bridge Forming Door objects
    // (209255, 209277) are a multi-piece visual bridge assembly tied to reaching Alysrazor's
    // flight arena rather than a simple open/close door, and aren't tracked either.
};

enum FirelandsCreatures
{
    BOSS_BETHTILAC            = 52498,
    BOSS_LORD_RHYOLITH        = 52558,
    BOSS_SHANNOX              = 53691,
    BOSS_ALYSRAZOR            = 52530,
    BOSS_BALEROC              = 53494,
    BOSS_MAJORDOMO_STAGHELM   = 52571,
    BOSS_RAGNAROS             = 52409,

    NPC_MOLTEN_LORD           = 53115,
    NPC_FLAMEWAKER_ANIMATOR   = 53187,
    NPC_FLAMEWAKER_SUBJUGATOR = 53188,
    NPC_CINDERWEB_DRONE       = 53635,
    NPC_FLAMEWAKER_CAUTERIZER = 53639,
    NPC_CINDERWEB_SPINNER     = 53642
};

enum FirelandsTrashSpells
{
    SPELL_MOLTEN_LORD_STRIKE              = 99532,
    SPELL_MOLTEN_LORD_STOMP               = 99555,
    SPELL_MOLTEN_LORD_ERUPTION            = 99530,
    SPELL_MOLTEN_LORD_MOLTEN_ARMOR        = 101617,

    // Reference casts this on summon-linked animation cues that we have no equivalent
    // trigger for; ported here as a simple periodic self-cast instead.
    SPELL_FLAMEWAKER_ANIMATOR_ANIMATE     = 99045,

    SPELL_FLAMEWAKER_SUBJUGATOR_EMERGENCY = 100527,

    SPELL_CINDERWEB_DRONE_POISON_SPIT     = 99463,
    SPELL_CINDERWEB_DRONE_WEB_SPRAY       = 99974,
    SPELL_CINDERWEB_DRONE_SHELL_ARMOR     = 100634,

    SPELL_FLAMEWAKER_CAUTERIZER_CAUTERIZE = 100060,

    SPELL_CINDERWEB_SPINNER_SPIT          = 99647
};

enum BethtilacSpells
{
    SPELL_BETHTILAC_EMBER_FLARE            = 98934,
    SPELL_BETHTILAC_VENOM_RAIN             = 99333,
    SPELL_BETHTILAC_SMOLDERING_DEVASTATION = 99052
};

enum ShannoxSpells
{
    SPELL_SHANNOX_IMMOLATION_TRAP = 47784,
    SPELL_SHANNOX_ARCING_SLASH    = 99931,
    SPELL_SHANNOX_HURL_SPEAR      = 100002
};

enum RhyolithSpells
{
    SPELL_RHYOLITH_CONCUSSIVE_STOMP = 97282,
    SPELL_RHYOLITH_MOLTEN_SPEW      = 98043
};

enum AlysrazorSpells
{
    SPELL_ALYSRAZOR_FIRE_IT_UP         = 100093,
    SPELL_ALYSRAZOR_FIEROCLAST_BARRAGE = 100095
};

enum BalerocSpells
{
    SPELL_BALEROC_TORMENT           = 99254,
    SPELL_BALEROC_SHARDS_OF_TORMENT = 99259,
    SPELL_BALEROC_BLAZE_OF_GLORY    = 99252,
    SPELL_BALEROC_BERSERK           = 26662
};

enum MajordomoStaghelmSpells
{
    SPELL_STAGHELM_FLAME_SCYTHE  = 98474,
    SPELL_STAGHELM_FIERY_CYCLONE = 98443,
    SPELL_STAGHELM_BURNING_ORBS  = 98451
};

enum RagnarosSpells
{
    SPELL_RAGNAROS_WRATH_OF_RAGNAROS = 98259,
    SPELL_RAGNAROS_MAGMA_BLAST       = 98313,
    SPELL_RAGNAROS_SULFURAS_SMASH    = 98710,
    SPELL_RAGNAROS_LIVING_METEOR     = 99267
};
