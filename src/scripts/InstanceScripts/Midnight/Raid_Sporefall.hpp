/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const SporefallEncounterCount = 1;

enum SporefallData
{
    DATA_ROTMIRE    = 0     // 3159
};

enum SporefallCreatures
{
    BOSS_ROTMIRE                    = 238693,

    // fungal minions grown by Awaken Fungi
    NPC_ROTMIRE_SPORECAP            = 238699,
    NPC_ROTMIRE_SHROOMLING          = 238696,
    NPC_ROTMIRE_FUNGLING            = 239020
};

enum SporefallSharedSpells
{
    SPELL_SPOREFALL_BERSERK         = 26662
};

enum SporefallRotmireSpells
{
    SPELL_ROTMIRE_AWAKEN_FUNGI      = 1221622,
    SPELL_ROTMIRE_FUNGAL_BLOOM      = 1221637,
    SPELL_ROTMIRE_BURSTING_SHROOM   = 1221965,
    SPELL_ROTMIRE_FUNGAL_FRENZY     = 1221644,
    SPELL_ROTMIRE_FESTERING_VINES   = 1222088,
    SPELL_ROTMIRE_BURSTING_PUSTULES = 1221787,
    SPELL_ROTMIRE_PUTRID_FIST       = 1221781,

    SPELL_SPORECAP_POISON_BURST     = 1221714,
    SPELL_SPORECAP_BLIGHTSHOT       = 1221717
};
