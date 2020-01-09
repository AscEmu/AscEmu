/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    //MeathookAI
    CN_MEATHOOK                     = 26529,

    //SalramTheFleshcrafterAI
    CN_SALRAMM_THE_FLESHCRAFTER     = 26530,

    //ChronoLordEpochAI
    CN_CHRONO_LORD_EPOCH            = 26532,

    //InfiniteCorruptorAI
    CN_INFINITE_CORRUPTOR           = 32273,

    //MalganisAI
    CN_MALGANIS                     = 26533,

    //UtherAI
    CN_UTHER                        = 26528,

    //ArthasAI
    CN_ARTHAS                       = 26499,

    // Jaina
    CN_JAINA                        = 26497,
};

enum CreatureSpells
{

};

enum CreatureSay
{
    //MeathookAI
    SAY_MEATHOOK_01         = 3466,     // Play time!
    SAY_MEATHOOK_02         = 3467,     // Boring...
    SAY_MEATHOOK_03         = 3468,     // Why you stop moving ?
    SAY_MEATHOOK_04         = 3469,     // Get up!Me not done!
    SAY_MEATHOOK_05         = 3470,     // \todo not used New toys!
    SAY_MEATHOOK_06         = 3471,     // This not fun...

    //SalramTheFleshcrafterAI
    SAY_SALRAM_FLESH_01     = 3481,     // Ah, the entertainment has arrived!
    SAY_SALRAM_FLESH_02     = 3482,     // \todo not used You are too late, champion of Lordaeron.The dead shall have their day.
    SAY_SALRAM_FLESH_03     = 3483,     // The fun is just beginning!
    SAY_SALRAM_FLESH_04     = 3484,     // Ah!Quality materials.
    SAY_SALRAM_FLESH_05     = 3485,     // Don't worry; I'll make good use of you!
    SAY_SALRAM_FLESH_06     = 3486,     // You only advance... the master's plan...
    SAY_SALRAM_FLESH_07     = 3487,     // \todo not used BOOM!Hahahahah...
    SAY_SALRAM_FLESH_08     = 3488,     // \todo not used Blood... destruction... EXHILARATING!
    SAY_SALRAM_FLESH_09     = 3489,     // \todo not used I want a sample...
    SAY_SALRAM_FLESH_10     = 3490,     // \todo not used Such strength... it must be mine!
    SAY_SALRAM_FLESH_11     = 3491,     // \todo not used Your flesh betrays you.
    SAY_SALRAM_FLESH_12     = 3492,     // \todo not used Say hello to some friends of mine.
    SAY_SALRAM_FLESH_13     = 3493,     // \todo not used Come, citizens of Stratholme!Meet your "saviors."

    //ChronoLordEpochAI
    SAY_CHRONOLORD_EPOCH_01 = 3472,     // \todo not used Prince Arthas Menethil, on this day, a powerful darkness has taken hold of your soul.The death you are destined to visit upon others will this day be your own.
    SAY_CHRONOLORD_EPOCH_02 = 3473,     // We'll see about that, young prince.
    SAY_CHRONOLORD_EPOCH_03 = 3474,     // \todo not used Tick tock, tick tock...
    SAY_CHRONOLORD_EPOCH_04 = 3475,     // \todo not used Not quick enough!
    SAY_CHRONOLORD_EPOCH_05 = 3476,     // \todo not used Let's get this over with.
    SAY_CHRONOLORD_EPOCH_06 = 3477,     // There is no future for you.
    SAY_CHRONOLORD_EPOCH_07 = 3478,     // This is the hour of our greatest triumph!
    SAY_CHRONOLORD_EPOCH_08 = 3479,     // You were destined to fail.
    SAY_CHRONOLORD_EPOCH_09 = 3480,     // \todo not used No text... sound 13416

    //InfiniteCorruptorAI
    SAY_INFINITE_CORRUP_01  = 5377,     // How dare you interfere with our work!
    SAY_INFINITE_CORRUP_02  = 5378,     // My time... has run out...
    SAY_INFINITE_CORRUP_03  = 5379,     // \todo not used The process is finally complete!My work here is done.

    //MalganisAI
    SAY_MALGANIS_01         = 3545,     // Yes, this is the beginning.I've been waiting for you, young prince. I am Mal'Ganis.
    SAY_MALGANIS_02         = 3546,     // As you can see, your people are now mine.I will now turn this city household by household, until the flame of life has been snuffed out... forever.
    SAY_MALGANIS_03         = 3547,     // This will be a fine test, Prince Arthas.
    SAY_MALGANIS_04         = 3548,     // All too easy.
    SAY_MALGANIS_05         = 3549,     // The Dark Lord is displeased with your interference.
    SAY_MALGANIS_06         = 3550,     // It is Prince Arthas I want, not you!
    SAY_MALGANIS_07         = 3551,     // \todo not used Anakh kyree!
    SAY_MALGANIS_08         = 3552,     // \todo not used My Onslaught will wash over the Lich King's forces!
    SAY_MALGANIS_09         = 3553,     // \todo not used Your death is in vain, tiny mortal.
    SAY_MALGANIS_10         = 3554,     // \todo not used Your time has come to an end.
    SAY_MALGANIS_11         = 3555,     // \todo not used Time out.
    SAY_MALGANIS_12         = 3556,     // \todo not used You seem tired.
    SAY_MALGANIS_13         = 3557,     // \todo not used Gah!I spent too much time in that weak little shell.
    SAY_MALGANIS_14         = 3558,     // \todo not used Kirel narak!I am Mal'Ganis. I AM ETERNAL!
    SAY_MALGANIS_15         = 3559,     // \todo not used ENOUGH!I waste my time here.I must gather my strength on the homeworld.
    SAY_MALGANIS_16         = 3560,     // You'll never defeat the Lich King without my forces. I'll have my revenge... on him AND you!
    SAY_MALGANIS_17         = 3561,     // Your journey has just begun, young prince.Gather your forces and meet me in the arctic land of Northrend.It is there that we shall settle the score between us.It is there that your true destiny will unfold.

    //UtherAI
    SAY_UTHER_01            = 3533,     // Watch your tone with me, boy.You may be the prince, but I'm still your superior as a paladin!
    SAY_UTHER_02            = 3534,     // What ?
    SAY_UTHER_03            = 3535,     // How can you even consider that ? There's got to be some other way.
    SAY_UTHER_04            = 3536,     // You are not my king yet, boy!Nor would I obey that command even if you were!
    SAY_UTHER_05            = 3537,     // Treason ? Have you lost your mind, Arthas ?
    SAY_UTHER_06            = 3538,     // You've just crossed a terrible threshold, Arthas.


    //ArthasAI
    SAY_ARTHAS_01           = 3494,     // Glad you could make it, Uther.
    SAY_ARTHAS_02           = 3495,     // As if I could forget.Listen, Uther, there's something about the plague you should know...
    SAY_ARTHAS_03           = 3496,     // Oh, no.We're too late. These people have all been infected! They may look fine now, but it's just a matter of time before they turn into the undead!
    SAY_ARTHAS_04           = 3497,     // This entire city must be purged.
    SAY_ARTHAS_05           = 3498,     // Damn it, Uther!As your future king, I order you to purge this city!
    SAY_ARTHAS_06           = 3499,     // Then I must consider this an act of treason.
    SAY_ARTHAS_07           = 3500,     // Have I ? Lord Uther, by my right of succession and the sovereignty of my crown, I hereby relieve you of your command and suspend your paladins from service.
    SAY_ARTHAS_08           = 3501,     // It's done! Those of you who have the will to save this land, follow me! The rest of you... get out of my sight!
    SAY_ARTHAS_09           = 3502,     // Jaina ?
    SAY_ARTHAS_10           = 3503,     // Take position here, and I will lead a small force inside Stratholme to begin the culling.We must contain and purge the infected for the sake of all of Lordaeron!
    SAY_ARTHAS_11           = 3504,     // Everyone looks ready.Remember, these people are all infected with the plague and will die soon.We must purge Stratholme to protect the remainder of Lordaeron from the Scourge.Let's go.
    SAY_ARTHAS_12           = 3505,     // I can only help you with a clean death.
    SAY_ARTHAS_13           = 3506,     // That was just the beginning.
    SAY_ARTHAS_14           = 3507,     // I won't allow it, Mal'Ganis!Better that these people die by my hand than serve as your slaves in death!
    SAY_ARTHAS_15           = 3508,     // \todo not used Mal'Ganis will send out some of his Scourge minions to interfere with us. Those of you with the strongest steel and magic shall go forth and destroy them. I will lead the rest of my forces in purging Stratholme of the infected.
    SAY_ARTHAS_16           = 3509,     // \todo not used Champions, meet me at the Town Hall at once.We must take the fight to Mal'Ganis.
    SAY_ARTHAS_17           = 3510,     // \todo not used Follow me, I know the way through.
    SAY_ARTHAS_18           = 3511,     // \todo not used Yes, I'm glad I could get to you before the plague.
    SAY_ARTHAS_19           = 3512,     // \todo not used What is this sorcery ?
    SAY_ARTHAS_20           = 3513,     // \todo not used Mal'Ganis appears to have more than Scourge in his arsenal. We should make haste.
    SAY_ARTHAS_21           = 3514,     // \todo not used More vile sorcery!Be ready for anything!
    SAY_ARTHAS_22           = 3515,     // \todo not used Let's move on.
    SAY_ARTHAS_23           = 3516,     // \todo not used Watch your backs : they have us surrounded in this hall.
    SAY_ARTHAS_24           = 3517,     // \todo not used One less obstacle to deal with.
    SAY_ARTHAS_25           = 3518,     // \todo not used Mal'Ganis is not making this easy.
    SAY_ARTHAS_26           = 3519,     // \todo not used They're very persistent.
    SAY_ARTHAS_27           = 3520,     // \todo not used What else can he put in my way ?
    SAY_ARTHAS_28           = 3521,     // \todo not used I do what I must for Lordaeron, and neither your words nor your actions will stop me.
    SAY_ARTHAS_29           = 3522,     // \todo not used The quickest path to Mal'Ganis lies behind that bookshelf ahead.
    SAY_ARTHAS_30           = 3523,     // \todo not used This will only take a moment.
    SAY_ARTHAS_31           = 3524,     // \todo not used I'm relieved this secret passage still works.
    SAY_ARTHAS_32           = 3525,     // \todo not used Let's move through here as quickly as possible. If the undead don't kill us, the fires might.
    SAY_ARTHAS_33           = 3526,     // \todo not used Rest a moment and clear your lungs, but we must move again soon.
    SAY_ARTHAS_34           = 3527,     // \todo not used That's enough; we must move again. Mal'Ganis awaits.
    SAY_ARTHAS_35           = 3528,     // \todo not used At last some good luck.Market Row has not caught fire yet.Mal'Ganis is supposed to be in Crusaders' Square, which is just ahead.Tell me when you're ready to move forward.
    SAY_ARTHAS_36           = 3529,     // \todo not used Justice will be done.
    SAY_ARTHAS_37           = 3530,     // \todo not used We're going to finish this right now, Mal'Ganis.Just you... and me.
    SAY_ARTHAS_38           = 3531,     // \todo not used I'll hunt you to the ends of the earth if I have to! Do you hear me? To the ends of the earth!
    SAY_ARTHAS_39           = 3532,     // \todo not used You performed well this day.Anything that Mal'Ganis has left behind is yours. Take it as your reward. I must now begin plans for an expedition to Northrend.


    // Jaina
    SAY_JAINA_01            = 3539,     // Arthas!You can't just--
    SAY_JAINA_02            = 3540,     // I'm sorry, Arthas. I can't watch you do this.


};
