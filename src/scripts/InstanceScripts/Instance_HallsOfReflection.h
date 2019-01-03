/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    //JainaAI
    CN_JAINA    = 37221,
    CN_UTHER    = 37225,
    CN_LICH     = 37226,

    //Marwyn
    CN_MARWYN   = 38113,

    //Falric
    CN_FALRIC   = 38112,

    //Alliance Spawns
    CN_JAINA_PROUDMOORE = 37221,
    CN_ARCHMAGE_KORELN = 37582,
    //Horde Spawns
    CN_SYLVANAS_WINDRUNNER = 37223
};

enum CreatureSpells
{
    //Marwyn
        // Normal Mode Spells
        N_SPELL_OBLITERATE      = 72360,
        N_SPELL_WELL            = 72362,
        N_SPELL_CORRUPTFLESH    = 72363,
        N_SPELL_SHARED          = 72368,
        // Heroic Mode Spells
        H_SPELL_OBLITERATE      = 72434,
        H_SPELL_WELL            = 72362,
        H_SPELL_CORRUPTFLESH    = 72436,
        H_SPELL_SHARED          = 72369,

    //Falric
        // Normal Mode Spells
        N_SPELL_QSTRIKE     = 72422,
        N_SPELL_IMPEND      = 72426,
        N_SPELL_HORROR      = 72435,
        // Heroic Mode Spells
        H_SPELL_QSTRIKE     = 72453,
        H_SPELL_IMPEND      = 72426,
        H_SPELL_HORROR      = 72452,
};

enum CreatureSay
{
    //JainaAI
    SAY_JAINA_01    = 4090,     // Stand back!Touch that blade and your soul will be scarred for all eternity!I...
    SAY_JAINA_02    = 4091,     /// \todo not used Frostmourne: the blade that destroyed our kingdom...
    SAY_JAINA_03    = 4092,     /// \todo not used What is that!Up ahead!Could it be ? Heroes, at my side!
    SAY_JAINA_04    = 4093,     /// \todo not used The chill of this place... I can feel my blood freezing.
    SAY_JAINA_05    = 4094,     // Uther!Dear Uther!I... I'm so sorry.
    SAY_JAINA_06    = 4095,     // Arthas is here ? Maybe I...
    SAY_JAINA_07    = 4096,     // But Uther, if there's any hope of reaching Arthas. I... I must try.
    SAY_JAINA_08    = 4097,     // Tell me how, Uther ? How do I destroy my prince ? My...
    SAY_JAINA_09    = 4098,     // You're right, Uther. Forgive me. I... I don't know what got a hold of me.We will..
    SAY_JAINA_10    = 4099,     // Who could bear such a burden ?
    SAY_JAINA_11    = 4100,     // Then maybe there is still hope...
    SAY_JAINA_12    = 4101,     /// \todo not used You won't deny me this, Arthas! I must know... I must find out... 

    //Uther
    SAY_UTHER_01    = 4117,     // Jaina!Could it truly be you ?
    SAY_UTHER_02    = 4118,     // Jaina, you haven't much time. The Lich King sees what the sword sees. He will be here shortly. 
    SAY_UTHER_03    = 4119,     // No, girl.Arthas is not here.Arthas is merely a presence within the Lich King's mind. A dwindling presence...
    SAY_UTHER_04    = 4120,     // Jaina, listen to me.You must destroy the Lich King.You cannot reason with him.He will kill you and your..
    SAY_UTHER_05    = 4121,     // Snap out of it, girl.You must destroy the Lich King at the place where he merged with Ner'zhul - ...
    SAY_UTHER_06    = 4122,     // There is... something else that you should know about the Lich King.Control over the Scourge must ...
    SAY_UTHER_07    = 4123,     // A grand sacrifice by a noble soul...
    SAY_UTHER_08    = 4124,     // I do not know, Jaina.I suspect that the piece of Arthas that might be left inside the Lich King ...
    SAY_UTHER_09    = 4125,     // No, Jaina!ARRRRRRGHHHH... He... He is coming.You... You must...
    SAY_UTHER_10    = 4126,     /// \todo not used Careful, girl.I've heard talk of that cursed blade saving us before. Look around ...
    SAY_UTHER_11    = 4127,     /// \todo not used You haven't much time. The Lich King sees what the sword sees. He will be here shortly. 
    SAY_UTHER_12    = 4128,     /// \todo not used You cannot defeat the Lich King.Not here.You would be a fool to try.He will kill those ...
    SAY_UTHER_13    = 4129,     /// \todo not used Perhaps, but know this: there must always be a Lich King.Even if you were to strike down ...
    SAY_UTHER_14    = 4130,     /// \todo not used I do not know, Banshee Queen.I suspect that the piece of Arthas that might be left inside ...
    SAY_UTHER_15    = 4131,     /// \todo not used Alas, the only way to defeat the Lich King is to destroy him at the place where he was created.
    SAY_UTHER_16    = 4132,     /// \todo not used Aye.ARRRRRRGHHHH... He... He is coming.You... You must...

    //Lich
    SAY_LICH_01     = 4102,     // SILENCE, PALADIN!
    SAY_LICH_02     = 4103,     // So you wish to commune with the dead ? You shall have your wish.
    SAY_LICH_03     = 4104,     // Falric.Marwyn.Bring their corpses to my chamber when you are through.
    SAY_LICH_04     = 6592,     /// \todo not used Foolish girl.You seek that which I killed long ago.He is merely a ghost now.A faint echo in my mind...
    SAY_LICH_05     = 6593      /// \todo not used I will not make the same mistake again, Sylvanas.This time there will be no escape.You will all ...

};
