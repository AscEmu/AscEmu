/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const RubyLifePoolsEncounterCount = 3;

enum RubyLifePoolsData
{
    DATA_KOKIA_BLAZEHOOF        = 0,  // 2606
    DATA_MELIDRUSSA_CHILLWORN   = 1,  // 2609
    // Kyrakka and Erkhart Stormvein
    DATA_KYRAKKA_AND_ERKHART    = 2   // 2623
};

enum RubyLifePoolsCreatures
{
    BOSS_KOKIA_BLAZEHOOF      = 189232,
    NPC_BLAZEBOUND_FIRE_STORM = 189886,

    BOSS_MELIDRUSSA_CHILLWORN = 188252,
    NPC_INFUSED_WHELP         = 187894,

    BOSS_KYRAKKA              = 199790,
    // second Kyrakka template of the same encounter
    BOSS_KYRAKKA_ALT          = 190484,
    BOSS_ERKHART_STORMVEIN    = 190485
};

enum RubyLifePoolsGameObjects
{
    // way on after Kokia
    GO_KOKIA_FIRE_WALL = 377194
};

enum RubyLifePoolsActions
{
    ACTION_KYRAKKA_LAND = 1,
    ACTION_ERKHART_REMOUNT
};

enum RubyLifePoolsKokiaSpells
{
    SPELL_KOKIA_SEARING_BLOWS          = 372858,
    SPELL_KOKIA_MOLTEN_BOULDER         = 372107,
    SPELL_KOKIA_RITUAL_OF_BLAZEBINDING = 372863,

    SPELL_BLAZEBOUND_BLAZE_VOLLEY      = 373017,
    SPELL_BLAZEBOUND_INFERNO           = 384823,
    SPELL_BLAZEBOUND_BURNOUT           = 373087
};

enum RubyLifePoolsMelidrussaSpells
{
    SPELL_MELIDRUSSA_FRIGID_SHARD  = 372808,
    SPELL_MELIDRUSSA_CHILLSTORM    = 372851,
    SPELL_MELIDRUSSA_HAILBURST     = 396044,
    SPELL_MELIDRUSSA_AWAKEN_WHELPS = 373046,

    SPELL_WHELP_COLD_CLAWS         = 1305233
};

enum RubyLifePoolsKyrakkaSpells
{
    SPELL_KYRAKKA_INFERNO_SPIT              = 381862,
    SPELL_KYRAKKA_ROARING_FIREBREATH        = 381525,

    SPELL_ERKHART_STORMSLAM                 = 381512,
    SPELL_ERKHART_WINDS_OF_CHANGE           = 381517,
    SPELL_ERKHART_INTERRUPTING_CLOUDBURST   = 381516
};
