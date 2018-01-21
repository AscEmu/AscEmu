/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum SpellDidHitResult
{
    SPELL_DID_HIT_SUCCESS = 0,
    SPELL_DID_HIT_MISS    = 1,
    SPELL_DID_HIT_RESIST  = 2,
    SPELL_DID_HIT_DODGE   = 3,
    SPELL_DID_HIT_PARRY   = 4,
    SPELL_DID_HIT_BLOCK   = 5,
    SPELL_DID_HIT_EVADE   = 6,
    SPELL_DID_HIT_IMMUNE  = 7,
    SPELL_DID_HIT_IMMUNE2 = 8,
    SPELL_DID_HIT_DEFLECT = 9,
    SPELL_DID_HIT_ABSORB  = 10,
    SPELL_DID_HIT_REFLECT = 11,
    NUM_SPELL_DID_HIT_RESULTS
};
