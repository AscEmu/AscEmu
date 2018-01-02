/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // Ethereal Darkcaster AI
    CN_ETHEREAL_DARKCASTER      = 18331,

    // Ethereal Priest AI
    CN_ETHEREAL_PRIEST          = 18317,

    // EtherealTheurgistAI
    CN_ETHEREAL_THEURGIST       = 18315,

    // EtherealSorcererAI
    CN_ETHEREAL_SORCERER        = 18313,

    // NexusStalkerAI
    CN_NEXUS_STALKER            = 18314,

    // Nexus Terror AI
    CN_NEXUS_TERROR             = 19307,
    // Mana Leech AI
    CN_MANA_LEECH               = 19306,
    // EtherealSpellbinderAI
    CN_ETHEREAL_SPELLBINDER     = 18312,    // couldn't find accurate source of it's spells
    // EtherealWraithAI
    CN_ETHEREAL_WRAITH          = 18394,

    // BossAIs
    // PandemoniusAI
    CN_PANDEMONIUS              = 18341,

    // TavarokAI
    CN_TAVAROK                  = 18343,

    // Nexus-Prince ShaffarAI
    CN_NEXUS_PRINCE_SHAFFAR     = 18344,

    // YorAI
    CN_YOR                      = 22930
};

enum CreatureSpells
{
    // Ethereal Darkcaster AI
    MANA_BURN                   = 29310,    // can be wrong
    SHADOW_WORD_PAIN            = 41355,

    // Ethereal Priest AI
    HEAL                        = 39378,
    POWER_WORD_SHIELD           = 29408,    // no idea if this is correct one [also can be: 41373 or other]

    // EtherealTheurgistAI
    POLYMORPH                   = 36840,    // so many poly to choose from
    BLAST_WAVE                  = 30092,    // not sure, also can be: 39001 (maybe too powerful?), 36278, 30600, 25049 (eventually)

    // EtherealSorcererAI
    ARCANE_MISSILES             = 29956,    //29955 can be also: 39414 (too much dmg?), 29956 (just arcane missiles without add. effects) // doesnt'w work for now

    // NexusStalkerAI
    GOUGE                       = 29425,    // not sure, maybe should be : 36862, 
    POISON                      = 34969,    // no idea if this is correct one
    STEALTH                     = 32615,    // Can be also: 31621, 31526, 30991 and others // maybe I should set it's proc chance to 0.0 and add spell casting OnCombatStart...

    // Nexus Terror AI
    PSYCHIC_SCREAM              = 34322,

    // Mana Leech AI
    ARCANE_EXPLOSION            = 39348,    // not sure about id

    // EtherealSpellbinderAI
    CORRUPTION                  = 30938,
    IMMOLATE                    = 38806,    // Not sure to any id/spell, because of lack of information source
    UNSTABLE_AFFLICTION         = 35183,    // if it casts this spell it can also be: 34439, 30938
    SUMMON_ETHEREAL_WRAITH      = 32316,    // added, but still more core support needed
                                            // On WoWWiki is description that Spellbinder summons are Mana Wraiths, but there is no
                                            // spell connected with such creature.

    // EtherealWraithAI
    SHADOW_BOLT_VOLLEY          = 36736,    // no idea if this is correct spell, but description seems to match with this what should be used
                                            // maybe normal shadow bolt like those: 29927, 30055 should be used

    // PandemoniusAI
    VOID_BLAST                  = 32325,    //38760
    DARK_SHELL                  = 32358,    //38759

    // TavarokAI
    EARTHQUAKE                  = 33919,    // affects also caster if it is close enough (and does it really works?)
    CRYSTAL_PRISON              = 32361,    // deals 10 dmg instead of 10% player hp - core problem
    ARCING_SMASH                = 39144,    // 8374; probably wrong id (maybe: 40457 ?)

    // Nexus-Prince ShaffarAI
    FIREBALL                    = 32363,
    FROSTBOLT                   = 32364,    // Also can be: 40430 or 32370
    FROST_NOVA                  = 32365,    // Also worth to try: 30094
    BLINK                       = 34605,    // 36109 - forward 20yard, 36718 - 5yard, behind target, 29883 - random target // still doesn't tp boss
    SUMMON_ETEREAL_BECON        = 32371,    // not sure about spawning way

    // YorAI
    DOUBLE_BREATH               = 38369,    // couldn't have found anything more powerful with that name
    STOMP                       = 34716     // not sure even more about this one
};

enum CreatureSay
{
    // PandemoniusAI
    SAY_PANDEMONIUS_01      = 4583,     // I will feed... on your soul.
    SAY_PANDEMONIUS_02      = 4584,     // So... full of life.
    SAY_PANDEMONIUS_03      = 4585,     // Do not... resist.
    SAY_PANDEMONIUS_04      = 4586,     // Yes... I am... empowered.
    SAY_PANDEMONIUS_05      = 4587,     // More... I must have more.
    SAY_PANDEMONIUS_06      = 4588,     // To the Void... once more.

    BROADCAST_PANDEMONIUS   = 4589,     /// \todo unused % s shifts into the void...

    // Nexus-Prince ShaffarAI
    SAY_NEXUSPRINCE_01      = 4575,     /// \todo unused What is this ? You must forgive me, but I was not expecting company...
    SAY_NEXUSPRINCE_02      = 4576,     // We have not been properly introduced.
    SAY_NEXUSPRINCE_03      = 4577,     // An epic battle, how exciting!
    SAY_NEXUSPRINCE_04      = 4578,     // I've longed for a good adventure!
    SAY_NEXUSPRINCE_05      = 4579,     // It has been... entertaining.
    SAY_NEXUSPRINCE_06      = 4580,     // And now we part company.
    SAY_NEXUSPRINCE_07      = 4581,     /// \todo unused I have such fascinating things to show you.
    SAY_NEXUSPRINCE_08      = 4582      // I must bid you... farewell.
};
