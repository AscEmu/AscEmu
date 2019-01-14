/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum AuraState
{
    AURASTATE_NONE                          = 0,
    AURASTATE_FLAG_DODGE_BLOCK_PARRY        = 1,
    AURASTATE_FLAG_HEALTH20                 = 2,  // Health below 20%
    AURASTATE_FLAG_BERSERK                  = 3,  // Two spells (with same name) in 3.3.5a, Birmingham Tools Test (40019 and 72556)
    AURASTATE_FLAG_FROZEN                   = 4,
    AURASTATE_FLAG_JUDGEMENT                = 5,  // Only paladin's Judgement spells require this aurastate
    AURASTATE_FLAG_PARRY                    = 7,  // Hunter's Counterattack
    AURASTATE_FLAG_LASTKILLWITHHONOR        = 10, // Warrior's Victory Rush in 3.3.5a
    AURASTATE_FLAG_PREVENT_STEALTH_INVIS    = 12, // Prevents stealth and invisibility
    AURASTATE_FLAG_HEALTH35                 = 13, // Health below 35%
    AURASTATE_FLAG_CONFLAGRATE              = 14, // Only warlock's Conflagrate require this aurastate
    AURASTATE_FLAG_SWIFTMEND                = 15, // Only druid's Swiftmend require this aurastate
    AURASTATE_FLAG_ENVENOM                  = 16, // Only rogue's Envenom require this aurastate
    AURASTATE_FLAG_ENRAGED                  = 17, // Warrior's Enraged Regeneration
    AURASTATE_FLAG_BLEED                    = 18,
    AURASTATE_FLAG_EVASIVE_CHARGE           = 22, // Two spells in 3.3.5a, Evasive Maneuvers (50240) and Death Ray (63884)
    AURASTATE_FLAG_HEALTH75                 = 23  // Health above 75%
};
