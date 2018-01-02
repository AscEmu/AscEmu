/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // Bosses
    CN_SVALA_SORROWGRAVE = 26668,
    CN_GORTOK_PALEHOOF = 26687,
    CN_SKADI_THE_RUTHLESS = 26693,
    CN_KING_YMIRON = 26861
};

enum CreatureSpells
{
    //SvalaSorrowgrave Spells
    CALL_FLAMES = 48258,
    RITUAL_OF_THE_SWORD = 48276,
    RITUAL_PREPARATION = 48267,
    RITUAL_STRIKE = 48277,      //59930?
    N_SINISTER_STRIKE = 15667,
    H_SINISTER_STRIKE = 59409,

    //GortokPalehoof Spells
    ARCING_SMASH = 48260,
    N_IMPALE = 48261,
    H_IMPALE = 59268,
    N_WITHERING_ROAR = 48256,
    H_WITHERING_ROAR = 59267,

    //SkadiTheRuthless Spells
    N_CRUSH = 50234,
    H_CRUSH = 59330,
    N_POISONED_SPEAR = 50255,
    H_POISONED_SPEAR = 59331,
    N_WHIRLWIND = 50228,
    H_WHIRLWIND = 59322,

    //KingYmiron Spells
    BANE = 48294,       // 59301?
    DARK_SLASH = 48292,
    FETID_ROT = 48291,      //59300?
    SCREAMS_OF_THE_DEAD = 51750,
    N_SPIRIT_BURST = 48529,
    H_SPIRIT_BURST = 59305,
    N_SPIRIT_STRIKE = 48423,
    H_SPIRIT_STRIKE = 59304
};

enum CreatureSay
{
    //SvalaSorrowgrave Say
    /*
    635	The sensation is... beyond my imagining.I am yours to command, my king.
    636	I will be happy to slaughter them in your name!Come, enemies of the Scourge!I will show you the might of the Lich King!
    637	I will vanquish your soul!
    638	You were a fool to challenge the power of the Lich King!
    639	Your will is done, my king!
    640	Another soul for my master!
    641	Nooo!I did not come this far... to...
    642	Your death approaches....
    643	Go now to my master!
    644	Your end is inevitable.
    645	Yor - guul mak!
    646	Any last words ?
    */

    //GortokPalehoof Say
    SAY_GROTOK_PALEHOOF_01 = 4491,      // What this place? I will destroy you!
    SAY_GROTOK_PALEHOOF_02 = 4492,      // You die!That what master wants.
    SAY_GROTOK_PALEHOOF_03 = 4493,      // An easy task!

    //SkadiTheRuthless Say
    SAY_SKADI_RUTHLESS_START = 4494,    // What mongrels dare intrude here ? Look alive, my brothers!A feast for the one that brings me their heads!
    SAY_SKADI_RUTHLESS_KILL_01 = 4495,  // Not so brash now, are you?
    SAY_SKADI_RUTHLESS_KILL_02 = 4496,  // I'll mount your skull from the highest tower!
    SAY_SKADI_RUTHLESS_DIE = 4498,      // ARGH! You call that... an attack? I'll...  show... aghhhh...
    /*
    4497 Skadi the Ruthless is within range of the harpoon launchers!
    4499 Skadi the Ruthless is within range of the harpoon launchers!
    4500 You motherless knaves! Your corpses will make fine morsels for my new drake!
    4501 Sear them to the bone!
    4502 Go now! Leave nothing but ash in your wake!
    4503 Cleanse our sacred halls with flame!
    */

    //KingYmiron Say
    SAY_KING_YMIRON_START = 4504,       //You invade my home and then dare to challenge me?  I will tear the hearts from your chests and offer them as gifts to the death god! Rualg nja gaborr!
    SAY_KING_YMIRON_KILL_01 = 4505,     //Your death is only the beginning.
    SAY_KING_YMIRON_KILL_02 = 4506,     //You have failed your people!
    SAY_KING_YMIRON_KILL_03 = 4507,     //There is a reason I am king!
    SAY_KING_YMIRON_KILL_04 = 4508,     //Breathe no more!
    SAY_KING_YMIRON_DIE = 4509,         //What... awaits me... now?
    /*4510 Bjorn of the Black Storm! Honor me now with your presence!
    4511 Haldor of the Rocky Cliffs! Grant me your strength!
    4512 Ranulf of the Screaming Abyss!  Snuff these maggots with darkest night!
    4513 Tor of the Brutal Siege, bestow your might upon me!
    */
};

enum GameobjectEntry
{
    GO_GORTOK_SPHERE = 188593,
    GO_MIRROR = 191745,
    GO_SKADI_DOOR = 192173,
    GO_YMIRON_DOOR = 192174
};
