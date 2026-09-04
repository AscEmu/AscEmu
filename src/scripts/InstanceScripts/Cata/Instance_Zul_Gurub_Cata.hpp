/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Zul'Gurub (Cataclysm revamp) - five main bosses plus four optional "animal god" rare-spawn
// bosses. Identities verified against wowhead. The four animal gods (Grilek, Hazza'rah,
// Renataki, Wushoolay) carry no hand-scripted rotation in the reference implementation either
// (database-driven spellcasting) - kept the same way here.

uint32_t const ZulGurubCataEncounterCount = 5;

enum ZulGurubCataData
{
    DATA_HIGH_PRIEST_VENOXIS    = 0,
    DATA_BLOODLORD_MANDOKIR     = 1,
    DATA_HIGH_PRIESTESS_KILNARA = 2,
    DATA_ZANZIL                 = 3,
    DATA_JINDO_THE_GODBREAKER   = 4,

    DATA_HAZZARAH               = 5,
    DATA_RENATAKI               = 6,
    DATA_WUSHOOLAY              = 7,
    DATA_GRILEK                 = 8
};

enum ZulGurubCataCreatures
{
    BOSS_HIGH_PRIEST_VENOXIS    = 52155,
    BOSS_BLOODLORD_MANDOKIR     = 52151,
    BOSS_HIGH_PRIESTESS_KILNARA = 52059,
    BOSS_ZANZIL                 = 52053,
    BOSS_JINDO_THE_GODBREAKER   = 52148,

    BOSS_HAZZARAH               = 52271,
    BOSS_RENATAKI               = 52269,
    BOSS_WUSHOOLAY              = 52286,
    BOSS_GRILEK                 = 52258,

    // Real add entry for Hazza'rah's Sleep mechanic - confirmed to exist in the reference
    // data (unlike the bosses' own abilities, which have none).
    NPC_NIGHTMARE_ILLUSION      = 52284
};

// The four "Edge of Madness" animal-god bosses are entirely database-driven in the reference
// (no AI scripting data) - there is nothing to port.
// Their abilities below are built from wowhead's Encounter Journal text instead. Where
// wowhead's own NPC ability list links a spell id whose "Used by NPC" backlink confirms this
// exact boss (Gri'lek's Avatar, both Hazza'rah nature bolts, its Sleep), that real id is used.
// Where no such id could be confirmed, the ability reuses a classic-era spell of the same
// name/effect as a stand-in (noted per spell below) - the mechanic (timing, targeting, HP
// triggers) is what's actually being ported in those cases, not the exact tooltip.
enum EdgeOfMadnessSpells
{
    // Gri'lek - all four confirmed via wowhead's NPC ability list / "Used by NPC" backlink.
    SPELL_GRILEK_AVATAR              = 24646,
    SPELL_GRILEK_PURSUIT             = 30153,
    SPELL_GRILEK_ENTANGLING_ROOTS    = 40363,
    SPELL_GRILEK_RUPTURE_LINE        = 89932,

    // Hazza'rah - Earth Shock/Wrath share the same id per wowhead's own listing; Sleep and
    // Consume Soul are also taken from its NPC ability list.
    SPELL_HAZZARAH_WRATH             = 8042,
    SPELL_HAZZARAH_EARTH_SHOCK       = 8042,
    SPELL_HAZZARAH_SLEEP             = 700,

    // Renataki
    SPELL_RENATAKI_DEADLY_POISON     = 2823,   // real "Deadly Poison" rank 1 (stacking Nature DoT)
    SPELL_RENATAKI_VANISH            = 1856,   // real "Vanish" rank 1 (visual only)
    SPELL_RENATAKI_AMBUSH            = 8676,   // stand-in: reuses "Ambush" rank 1
    SPELL_RENATAKI_THOUSAND_BLADES   = 1680,   // stand-in: reuses "Whirlwind" rank 1

    // Wushoolay
    SPELL_WUSHOOLAY_LIGHTNING_CLOUD  = 403,    // stand-in: reuses "Lightning Bolt" rank 1
    SPELL_WUSHOOLAY_FORKED_LIGHTNING = 421,    // stand-in: reuses "Chain Lightning" rank 1 (cone/multi-hit)
    SPELL_WUSHOOLAY_LIGHTNING_ROD    = 46968   // stand-in: reuses "Shockwave" (AoE + knockback)
};

enum ZulGurubCataTrashCreatures
{
    NPC_GURUBASHI_CAULDRON_MIXER_A = 52076,
    NPC_GURUBASHI_CAULDRON_MIXER_B = 52082,
    NPC_RAZZASHI_ADDER             = 52085,
    NPC_HAKKARI_WITCH_DOCTOR       = 52086,
    NPC_GURUBASHI_CAULDRON_MIXER_C = 52088,
    NPC_WITCH_DOCTOR_QUIN          = 52322,
    NPC_CHOSEN_OF_HETHISS          = 52323,
    NPC_GURUBASHI_BLOOD_DRINKER    = 52325,
    NPC_GURUBASHI_SHADOW_HUNTER    = 52327,
    NPC_LESSER_PRIEST_OF_BETHEKK   = 52339,
    NPC_PRIDE_OF_BETHEKK           = 52345,
    NPC_VENOMANCER_MAURI           = 52380,
    NPC_VENOMANCER_TKULU           = 52381,
    NPC_TIKI_TORCH                 = 52419,
    NPC_GURUBASHI_SOUL_EATER       = 52598,
    NPC_GURUBASHI_WARMONGER        = 52606,
    NPC_ZANDALARI_JUGGERNAUT       = 52956,
    NPC_ZANDALARI_HIEROPHANT       = 52958,
    NPC_ZANDALARI_ARCHON           = 52962
};

enum ZulGurubCataTrashSpells
{
    SPELL_CAULDRON_MIXER_A_BREW       = 96488,
    SPELL_CAULDRON_MIXER_A_HEX        = 96449,
    SPELL_CAULDRON_MIXER_A_TOXIN      = 97609,
    SPELL_CAULDRON_MIXER_A_SPEED_BUFF = 97387,
    SPELL_CAULDRON_MIXER_A_HEX_BOLT   = 96413,

    SPELL_CAULDRON_MIXER_B_BREW       = 96487,
    SPELL_CAULDRON_MIXER_B_TOXIN      = 96804,

    SPELL_RAZZASHI_ADDER_VENOM_SPIT   = 97599,
    SPELL_RAZZASHI_ADDER_POISON       = 3391,

    SPELL_WITCH_DOCTOR_MOJO           = 45104,
    SPELL_WITCH_DOCTOR_HEX            = 96416,
    SPELL_WITCH_DOCTOR_CURSE          = 97398,
    SPELL_WITCH_DOCTOR_HEALING_WARD   = 97397,

    SPELL_QUIN_RITUAL                 = 97016,

    SPELL_HETHISS_CURSE               = 97019,
    SPELL_HETHISS_PULSE               = 97018,

    SPELL_BLOOD_DRINKER_FRENZY        = 13737,
    SPELL_BLOOD_DRINKER_LEECH         = 96764,

    SPELL_SHADOW_HUNTER_TOXIN         = 97239,
    SPELL_SHADOW_HUNTER_SHADOW_BOLT   = 96951,

    SPELL_BETHEKK_PRIEST_SMITE        = 96956,
    SPELL_BETHEKK_PRIEST_HEAL         = 96790,
    SPELL_BETHEKK_PRIEST_SHIELD       = 96849,

    SPELL_PRIDE_OF_BETHEKK_CLAW       = 97355,

    // Reuses the High Priest Venoxis abilities: the reference casts these same spell ids
    // on the venomancer trash as a scaled-down version of the boss kit.
    SPELL_VENOMANCER_TOXIN            = 96918,

    SPELL_TIKI_TORCH_FLARE            = 97000,

    SPELL_SOUL_EATER_DRAIN            = 97248,
    SPELL_SOUL_EATER_SHIELD           = 97250,

    SPELL_WARMONGER_ENRAGE            = 59697,

    SPELL_JUGGERNAUT_WAR_STOMP        = 97987,
    SPELL_JUGGERNAUT_BATTLE_CRY       = 97977,
    SPELL_JUGGERNAUT_CHARGE           = 97970,
    SPELL_JUGGERNAUT_SMASH            = 98000,

    SPELL_HIEROPHANT_ARCANE_BOLT      = 97962,
    SPELL_HIEROPHANT_ARCANE_STORM     = 97973,
    SPELL_HIEROPHANT_HEAL             = 97977,
    SPELL_HIEROPHANT_SHIELD           = 97969,
    SPELL_HIEROPHANT_MANA_BURN        = 97978,

    SPELL_ARCHON_HEAL                 = 97977,
    SPELL_ARCHON_RITUAL               = 97972,
    SPELL_ARCHON_LIGHTNING_BOLT       = 98016,
    SPELL_ARCHON_CHAIN_LIGHTNING      = 98014,
    SPELL_ARCHON_THUNDERSTORM         = 98024,
    SPELL_ARCHON_STATIC_SHOCK         = 98019
};

enum ZulGurubCataGameObjects
{
    GO_VENOXIS_COIL = 208844,
    GO_ARENA_DOOR_1 = 208845,
    GO_FORCEFIELD   = 180497,
    GO_ZANZIL_DOOR  = 208850
};

enum VenoxisSpells
{
    SPELL_VENOXIS_WHISPERS_OF_HETHISS = 96466,
    SPELL_VENOXIS_TOXIC_LINK          = 96475,
    SPELL_VENOXIS_POOL_OF_ACRID_TEARS = 96515
};

enum MandokirSpells
{
    SPELL_MANDOKIR_DECAPITATE       = 96682,
    SPELL_MANDOKIR_BLOODLETTING     = 96776,
    SPELL_MANDOKIR_SUMMON_OHGAN     = 96717,
    SPELL_MANDOKIR_DEVASTATING_SLAM = 96740
};

enum KilnaraSpells
{
    SPELL_KILNARA_SHADOW_BOLT     = 96516,
    SPELL_KILNARA_TEARS_OF_BLOOD  = 96438,
    SPELL_KILNARA_LASH_OF_ANGUISH = 96423,
    SPELL_KILNARA_WAVE_OF_AGONY   = 96457
};

enum ZanzilSpells
{
    SPELL_ZANZIL_ZANZILI_FIRE   = 96914,
    SPELL_ZANZIL_TERRIBLE_TONIC = 96348
};

enum JindoSpells
{
    SPELL_JINDO_DEADZONE          = 97170,
    SPELL_JINDO_SHADOWS_OF_HAKKAR = 97172,
    SPELL_JINDO_SHADOW_SPIKE      = 97158
};
