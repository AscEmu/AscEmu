/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // Doctor Theolen KrastinovAI
    CN_DOCTOR_THEOLEN_KRASTINOV     = 11261,

    // Instructor MaliciaAI
    CN_INSTRUCTOR_MALICIA           = 10505,

    // The RavenianAI
    CN_THE_RAVENIAN                 = 10507,

    // Lady Illucia BarovAI
    CN_LADY_ILLUCIA_BAROV           = 10502,

    // Ras ForstwhisperAI
    CN_RAS_FORSTWHISPER             = 10508,

    // Jandice BarovAI
    CN_JANDICE_BAROV                = 10503,

    // KormokAI
    CN_KORMOK                       = 14491,

    // VectusAI
    CN_VECTUS                       = 10432,

    // Lord Alexei BarovAI
    CN_LORD_ALEXEI_BAROV            = 10504,

    // Lorekeeper PolkeltAI
    CN_LOREKEEPER_POLKELT           = 10901,

    // Darkmaster GandlingAI
    CN_DARKMASTER_GANDLING          = 1853,

};

enum CreatureSpells
{
    // Doctor Theolen KrastinovAI
    SP_DR_THEOL_REND            = 18106,
    SP_DR_THEOL_KRASTINOVCLEAVE = 15584,
    SP_DR_THEOL_FRENZY          = 28371,

    // Instructor MaliciaAI
    SP_MALICIA_CALL_OF_GRAVE    = 17831,
    SP_MALICIA_CORRUPTION       = 11672,
    SP_MALICIA_FLASH_HEAL       = 17138,    //10917
    SP_MALICIA_RENEW            = 10929,
    SP_MALICIA_HEAL             = 15586,    // not sure

    // The RavenianAI
    SP_RAVENIAN_TRAMPLE         = 15550,
    SP_RAVENIAN_RAVENIANCLEAVE  = 20691,
    SP_RAVENIAN_SUNDERINCLEAVE  = 25174,
    SP_RAVENIAN_KNOCKAWAY       = 10101,

    // Lady Illucia BarovAI
    SP_ILLUCIA_CURSE_OF_AGONY   = 18671,
    SP_ILLUCIA_SHADOW_SHOCK     = 20603,
    SP_ILLUCIA_SILENCE          = 15487,
    SP_ILLUCIA_FEAR             = 26580,    //26661
    SP_ILLUCIA_DOMINATE_MIND    = 20740,

    // Ras ForstwhisperAI
    SP_RAS_FORTH_FROSTBOLT      = 21369,
    SP_RAS_FORTH_ICE_ARMOR      = 18100,
    SP_RAS_FORTH_FREEZE         = 18763,
    SP_RAS_FORTH_FEAR           = 26070,
    SP_RAS_FORTH_CHILL_NOVA     = 18099,
    SP_RAS_FORTH_FROSTB_VOLLEY  = 22643,    //8398 

    // Jandice BarovAI
    SP_JANDICE_CURSE_OF_BLOOD   = 24673,
    SP_JANDICE_SUMMON_ILLUSION  = 17773,
    SP_JANDICE_BANISH           = 39674,    // not sure //8994

    // KormokAI
    SP_KORMOK_SHADOW_B_VOLLEY   = 20741,
    SP_KORMOK_BONE_SHIELD       = 27688,
    SP_KORMOK_SUM_RISEY_LACKEY  = 17618,    // not sure

    // VectusAI
    SP_VECTUS_FIRE_SHIELD       = 19627,
    SP_VECTUS_BLAST_WAVE        = 13021,
    SP_VECTUS_FRENZY            = 28371,

    // Lord Alexei BarovAI
    SP_ALEXEI_UNHOLY_AURA       = 17467,
    SP_ALEXEI_IMMOLATE          = 20294,
    SP_ALEXEI_VEIL_OF_SHADOW    = 17820,

    // Lorekeeper PolkeltAI
    SP_LORE_VOLATILE_INFECTION  = 24928,    // wrong id // can cause crashes as it is casted on caster too
    SP_LORE_DARK_PLAGUE         = 18270,    // sure it should be here?
    SP_LORE_CORROSIVE_ACID      = 19463,    // Added Corrosive Acid Spit; 16359 or 20667 or 19463 or 19463    // 23313
    SP_LORE_NOXIOUS_CATALYST    = 18151,

    // Darkmaster GandlingAI
    SP_GANDLING_ARCANE_MISSILES = 25346,
    SP_GANDLING_COT_DARKMASTER  = 18702,    // Curse of the Darkmaster
    SP_GANDLING_SHADOW_SHIELD   = 22417,

};

enum CreatureSay
{

};
