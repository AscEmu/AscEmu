/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TheDawnbreakerEncounterCount = 3;

enum TheDawnbreakerData
{
    DATA_SPEAKER_SHADOWCROWN    = 0,    // 2837
    DATA_ANUBIKKAJ              = 1,    // 2838
    DATA_DAWNBREAKER_RASHANAN   = 2     // 2839
};

enum TheDawnbreakerCreatures
{
    BOSS_SPEAKER_SHADOWCROWN        = 211087,

    BOSS_ANUBIKKAJ                  = 211089,
    // lieutenants empowering Anub'ikkaj while alive
    NPC_ASCENDANT_VISCOXRIA         = 211261,
    NPC_IXKRETEN_THE_UNBREAKABLE    = 211262,
    NPC_DEATHSCREAMER_IKENTAK       = 211263,

    BOSS_DAWNBREAKER_RASHANAN       = 213937
};

enum SpeakerShadowcrownSpells
{
    SPELL_SHADOWCROWN_DARKNESS_COMES        = 451026,
    SPELL_SHADOWCROWN_OBSIDIAN_BLAST        = 425264,
    SPELL_SHADOWCROWN_COLLAPSING_DARKNESS   = 426712,
    SPELL_SHADOWCROWN_BURNING_SHADOWS       = 426734,
    SPELL_SHADOWCROWN_SHADOW_BOLT           = 428086
};

enum AnubikkajSpells
{
    SPELL_ANUBIKKAJ_EMPOWERED_MIGHT         = 427192,
    SPELL_ANUBIKKAJ_DARK_ORB                = 426860,
    SPELL_ANUBIKKAJ_TERRIFYING_SLAM         = 427001,
    SPELL_ANUBIKKAJ_SHADOWY_DECAY           = 426787
};

enum DawnbreakerRashananSpells
{
    SPELL_RASHANAN_ARATHI_BOMBS             = 434655,
    SPELL_RASHANAN_ROLLING_ACID             = 434407,
    SPELL_RASHANAN_EROSIVE_SPRAY            = 448888,
    SPELL_RASHANAN_ACIDIC_ERUPTION          = 449734,
    SPELL_RASHANAN_SPINNERETS_STRANDS       = 434089,
    SPELL_RASHANAN_TACKY_BURST              = 435793
};
