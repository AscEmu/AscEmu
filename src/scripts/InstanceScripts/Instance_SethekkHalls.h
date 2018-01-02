/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // Avian Darkhawk AI
    CN_AVIAN_DARKHAWK           = 20686,

    // Avian Ripper AI
    CN_AVIAN_RIPPER             = 21891,

    // Avian Warhawk AI
    CN_AVIAN_WARHAWK            = 21904,    // test it more

    // Cobalt Serpent AI
    CN_COBALT_SERPENT           = 19428,

    // Time-Lost Controller AI
    CN_TIME_LOST_CONTROLLER     = 20691,

    // Time-Lost Scryer AI
    CN_TIME_LOST_SCRYER         = 20697,

    // Time-Lost Shadowmage AI
    CN_TIME_LOST_SHADOWMAGE     = 20698,

    // Sethekk Guard AI
    CN_SETHEKK_GUARD            = 18323,

    // Sethekk Initiate AI
    CN_SETHEKK_INITIATE         = 18318,

    // Sethekk Oracle AI
    CN_SETHEKK_ORACLE           = 18328,

    // Sethekk Prophet AI
    CN_SETHEKK_PROPHET          = 18325,

    // Sethekk Ravenguard AI
    CN_SETHEKK_RAVENGUARD       = 18322,

    // Sethekk Shaman AI
    CN_SETHEKK_SHAMAN           = 18326,

    // Sethekk Talon Lord AI
    CN_SETHEKK_TALON_LORD       = 18321,

    ////////////////////////////////////////////////////
    // Lakka AI
    CN_LAKKA                    = 18956,

    // Darkweaver SythAI
    CN_DARKWEAVER_SYTH          = 18472,

    // Talon King IkissAI
    CN_TALON_KING_IKISS         = 18473,

    // AnzuAI
    CN_ANZU                     = 23035,

};

enum CreatureSpells
{
    // Avian Darkhawk AI
    SP_AVIAN_DARKHAWK_CHARGE            = 36509,    // no idea if this is correct id

    // Avian Ripper AI
    SP_AVIAN_RIPPER_FLESH_RIP           = 40199,

    // Avian Warhawk AI
    SP_AVIAN_WARHAWK_CLEAVE             = 38474,    // no idea if this is right
    SP_AVIAN_WARHAWK_CHARGE             = 40602,    // same here
    SP_AVIAN_WARHAWK_BITE               = 39382,    // and here =)

    // Cobalt Serpent AI
    SP_COBALT_SERPENT_WING_BUFFET       = 41572,
    SP_COBALT_SERPENT_FROSTBOLT         = 40429,    // no idea about if these are good ids :P
    SP_COBALT_SERPENT_CHAIN_LIGHTNING   = 39945,

    // Time-Lost Controller AI
    SP_TL_CONTROLLER_SHIRNK             = 36697,    // 36697 or 35013
                                                    //SP_TIME_LOST_CONTROLLER_CONTROL_TOTEM // Can't find spell for that :O
    // Time-Lost Scryer AI
    SP_TL_SCRYER_FLASH_HEAL             = 38588,    // let's try this one
    SP_TL_SCRYER_ARCANE_MISSILES        = 35034,    // and those: 35033, 35034 // doesn't work somehow

    // Time-Lost Shadowmage AI
    SP_TL_CURSE_OF_THE_DARK_TALON       = 32682,

    // Sethekk Guard AI
    SP_SETHEKK_GUARD_THUNDERCLAP        = 36214,
    SP_SETHEKK_GUARD_SUNDER_ARMOR       = 30901,    // 1000 arm per use (to 5 uses!) O_O

    // Sethekk Initiate AI
    SP_SETHEKK_INIT_MAGIC_REFLECTION    = 20223,    // 20223 or 20619

    // Sethekk Oracle AI
    SP_SETHEKK_ORACLE_FAERIE_FIRE       = 21670,    // 20656 or 21670 or 32129 or other
    SP_SETHEKK_ORACLE_ARCANE_LIGHTNING  = 38146,    // 38146, 32690 or 38634

    // Sethekk Prophet AI
    SP_SETHEKK_PROPHET_FEAR             = 32241,    // Should it be aoe or normal? // damn it fears caster too
                                                    // Ghost spawning similar to those in Sunken Temple
    // Sethekk Ravenguard AI
    SP_SETHEKK_RAVENG_BLOODTHIRST       = 31996,    // check also spells like this: 31996 and this: 35948
    SP_SETHEKK_RAVENG_HOWLING_SCREECH   = 32651,

    // Sethekk Shaman AI
    SP_SETHEKK_SHAMAN_SUM_DARK_VORTEX   = 32663,    //SUMMON_VOIDWALKER 30208 // Shouldn't be Dark Vortex (spell id: 32663) ?

    // Sethekk Talon Lord AI
    SP_SETHEKK_TALON_OF_JUSTICE         = 32654,    // 32654 or 39229
    SP_SETHEKK_TALON_AVENGERS_SHIELD    = 32774,    // On WoWWiki is Shield of Revenge, but that should be it. Also spells that can be: 32774, 32674, 37554

    // Darkweaver SythAI
    SP_DARKW_SYNTH_FROST_SHOCK          = 37865,
    SP_DARKW_SYNTH_FLAME_SHOCK          = 34354,
    SP_DARKW_SYNTH_SHADOW_SHOCK         = 30138,
    SP_DARKW_SYNTH_ARCANE_SHOCK         = 37132,
    SP_DARKW_SYNTH_CHAIN_LIGHTNING      = 39945,

    // Darkweaver SythAI Summons
    SP_DARKW_SYNTH_SUM_FIRE_ELEMENTAL       = 33537,
    SP_DARKW_SYNTH_SUM_FROST_ELEMENTAL      = 33539,
    SP_DARKW_SYNTH_SUM_ARCANE_ELEMENTAL     = 33538,
    SP_DARKW_SYNTH_SUM_SHADOW_ELEMENTAL     = 33540,

    // Talon King IkissAI
    SP_TALRON_K_IKISS_ARCANE_VOLLEY     = 36738,    // 35059 ?
    SP_TALRON_K_IKISS_ARCANE_EXPLOSION  = 38197,    // bit too high dmg, but should work nearly in the same way
    SP_TALRON_K_IKISS_BLINK             = 1953,     // 38194; lacks of core support
    SP_TALRON_K_IKISS_POLYMORPH         = 12826,    // 38245; worth to try also: 38245, 38896
                                                    //SP_TALRON_KING_IKISS_MANA_SHIELD 38151 // also: 35064, 38151

    // AnzuAI
    SP_ANZU_SUMMON_RAVEN_GOD            = 40098,    // event just to test it!
    SP_ANZU_SPELL_BOMB                  = 40303,
    SP_ANZU_CYCLONE_OF_FEATHERS         = 40321,
    SP_ANZU_PARALYZING_SCREECH          = 40184,
    SP_ANZU_BANISH                      = 40370,    // can be: 38791, 38009, 40370, 39674, 35182, 40825 // should banish for one minute

};

enum CreatureSay
{
    // Darkweaver SythAI
    SAY_DARKW_SYNTH_01      = 4590,     // I have pets -- ca - caw!--of my own!
    SAY_DARKW_SYNTH_02      = 4591,     // Mmm... time to make my move!
    SAY_DARKW_SYNTH_03      = 4592,     // Nice pets, yes... raa - a - ak!
    SAY_DARKW_SYNTH_04      = 4593,     // Nice pets have weapons.No so -- ra - ak -- nice.
    SAY_DARKW_SYNTH_05      = 4594,     // Yes, fleeting life is, rak - a - kak!
    SAY_DARKW_SYNTH_06      = 4595,     // Mmmm... be free.Caw!
    SAY_DARKW_SYNTH_07      = 4596,     // Mmm... no more life.No more pain.A - ak!

    // Talon King IkissAI
    SAY_TALRON_K_IKISS_01   = 4597,     /// \todo not used Ra - ak!Trinkets, yes.Pretty trinkets!Ak!Power, great power.Ra - kaw!Power in trinkets!Ak - caw!
    SAY_TALRON_K_IKISS_02   = 4598,     // Mmm, you make war on Ikiss ? Aa - ak!
    SAY_TALRON_K_IKISS_03   = 4599,     // Ikiss cut you, pretty -- ak - a - ak -- slice you, yes!
    SAY_TALRON_K_IKISS_04   = 4600,     // Ak - a - ra - k!No escape for --caw -- you!
    SAY_TALRON_K_IKISS_05   = 4601,     // You die -- ka!Stay away from trinkets!
    SAY_TALRON_K_IKISS_06   = 4602,     // Mmmmmm...
    SAY_TALRON_K_IKISS_07   = 4603,     // Ikiss will not -- rak, rak -- die...
    BROADCAST_TALRON_K_IKISS_01     = 4604      /// \todo not used %s begins to channel arcane energy....
};
