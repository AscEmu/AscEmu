/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Throne of Thunder - eleven bosses: Jin'rokh the Breaker, Horridon, Council of Elders,
// Tortos, Megaera, Ji-Kun, Durumu the Forgotten, Primordius, Dark Animus, Iron Qon, the Twin
// Consorts, Lei Shen. Ra-den, the hidden bonus final boss unlocked only by a server-wide
// realm-first achievement, is not ported - same precedent as Cata's Sinestra in Bastion of
// Twilight. No prior scripting or dialogue data exists for
// this raid - all kits below are built from wowhead's MoP Classic data instead. The Council
// of Elders (four independent casters in the real fight: Kazra'jin, Sul the Sandcrawler,
// Frost King Malakk, High Priestess Mar'li) is simplified to a single representative boss
// running one ability from each of the four themes, since only Kazra'jin is registered here.
// Megaera and Lei Shen's kits are sourced from their wowhead strategy-guide pages (their
// individual NPC pages carry no ability data). Jin'rokh's entry (69465) was independently
// confirmed on wowhead against a same-named but unrelated Classic Stranglethorn Vale mob
// (14902, level ~30) that turned up in an earlier reference-database pass for this raid -
// 69465 is the real Throne of Thunder boss.

uint32_t const ThroneOfThunderEncounterCount = 12;

enum ThroneOfThunderData
{
    DATA_JINROKH           = 0,
    DATA_HORRIDON          = 1,
    DATA_COUNCIL_OF_ELDERS = 2,
    DATA_TORTOS            = 3,
    DATA_MEGAERA           = 4,
    DATA_JIKUN             = 5,
    DATA_DURUMU            = 6,
    DATA_PRIMORDIUS        = 7,
    DATA_DARK_ANIMUS       = 8,
    DATA_IRON_QON          = 9,
    DATA_TWIN_CONSORTS     = 10,
    DATA_LEI_SHEN          = 11
};

enum ThroneOfThunderCreatures
{
    BOSS_JINROKH              = 69465,
    BOSS_HORRIDON             = 68476,
    BOSS_KAZRAJIN             = 69134,
    BOSS_TORTOS               = 67977,
    BOSS_MEGAERA              = 68065,
    BOSS_JIKUN                = 69712,
    BOSS_DURUMU               = 68036,
    BOSS_PRIMORDIUS           = 69017,
    BOSS_DARK_ANIMUS          = 69427,
    BOSS_IRON_QON             = 68078,
    BOSS_LULIN                = 68905,
    BOSS_LEI_SHEN             = 68397,

    // Trash - this raid has no placed spawn data in either reference database, but these two
    // real, named creature templates sit in a dense "Zandalari" cluster directly adjacent to
    // the Council of Elders' own confirmed entries (69131-69134) and match this raid's
    // Zandalari-troll-army theme closely enough to be a reasonably confident (if not
    // spawn-confirmed) match.
    NPC_ZANDALARI_BEASTCALLER = 69065,
    NPC_ZANDALARI_SKYSCREAMER = 69156
};

enum ZandalariBeastcallerSpells
{
    SPELL_BEASTCALLER_CLEAVE = 845,    // stand-in: reuses "Cleave" rank 1
    SPELL_BEASTCALLER_ENRAGE = 8599    // stand-in: reuses the generic "Enrage" self-buff
};

enum ZandalariSkyscreamerSpells
{
    SPELL_SKYSCREAMER_MULTI_SHOT = 2643,  // stand-in: reuses the real "Multi-Shot" spell
    SPELL_SKYSCREAMER_SCREECH    = 5782   // stand-in: reuses generic "Fear"
};

enum JinrokhSpells
{
    SPELL_JINROKH_THUNDERING_THROW = 137167,
    SPELL_JINROKH_STATIC_BURST     = 137162,
    SPELL_JINROKH_LIGHTNING_STORM  = 137261,
    SPELL_JINROKH_LIGHTNING_STRIKE = 137647
};

enum HorridonSpells
{
    SPELL_HORRIDON_TRIPLE_PUNCTURE = 136767,
    SPELL_HORRIDON_DOUBLE_SWIPE    = 136740,
    SPELL_HORRIDON_DIRE_CALL       = 137458,
    SPELL_HORRIDON_DIRE_FIXATION   = 140946
};

// Simplified to Kazra'jin's own kit plus one representative ability from each of the other
// three council members (frost/sand/loa) - see header comment.
enum CouncilOfEldersSpells
{
    SPELL_COUNCIL_DARK_POWER       = 136507,
    SPELL_COUNCIL_FRIGID_ASSAULT   = 136904,
    SPELL_COUNCIL_SAND_BOLT        = 136189,
    SPELL_COUNCIL_WRATH_OF_THE_LOA = 137344
};

enum TortosSpells
{
    SPELL_TORTOS_SPINNING_SHELL       = 133974,
    SPELL_TORTOS_KICK_SHELL           = 134030,
    SPELL_TORTOS_FURIOUS_STONE_BREATH = 133939,
    SPELL_TORTOS_ROCKFALL             = 134539
};

// Sourced from Megaera's wowhead strategy-guide page (three-headed hydra fight: fire/frost/
// venom heads).
enum MegaeraSpells
{
    SPELL_MEGAERA_CINDERS        = 139822,
    SPELL_MEGAERA_TORRENT_OF_ICE = 139866,
    SPELL_MEGAERA_ACID_RAIN      = 134378,
    SPELL_MEGAERA_HYDRA_FRENZY   = 139942
};

enum JikunSpells
{
    SPELL_JIKUN_TALON_STRIKE = 139100,
    SPELL_JIKUN_SCREECH      = 140640,
    SPELL_JIKUN_TALON_RAKE   = 134366,
    SPELL_JIKUN_QUILLS       = 134380
};

enum DurumuSpells
{
    SPELL_DURUMU_DISINTEGRATION_BEAM = 133776,
    SPELL_DURUMU_GAZE                = 134029,
    SPELL_DURUMU_BRIGHT_LIGHT        = 133738,
    SPELL_DURUMU_LIFE_DRAIN          = 133795
};

enum PrimordiusSpells
{
    SPELL_PRIMORDIUS_PRIMORDIAL_STRIKE = 136037,
    SPELL_PRIMORDIUS_MALFORMED_BLOOD   = 136050,
    SPELL_PRIMORDIUS_MUTAGENIC_POOL    = 136049,
    SPELL_PRIMORDIUS_BLACK_BLOOD       = 137000
};

enum DarkAnimusSpells
{
    SPELL_ANIMUS_EXPLOSIVE_SLAM      = 138569,
    SPELL_ANIMUS_SIPHON_ANIMA        = 138644,
    SPELL_ANIMUS_TOUCH_OF_THE_ANIMUS = 138659,
    SPELL_ANIMUS_ANIMA_RING          = 136954
};

enum IronQonSpells
{
    SPELL_QON_THROW_SPEAR      = 115062,
    SPELL_QON_IMPALE           = 134691,
    SPELL_QON_UNLEASHED_FLAME  = 134628,
    SPELL_QON_ARCING_LIGHTNING = 136193,
    SPELL_QON_FROST_SPIKE      = 139180
};

enum TwinConsortsSpells
{
    SPELL_CONSORTS_COSMIC_BARRAGE  = 136752,
    SPELL_CONSORTS_NUCLEAR_INFERNO = 137491,
    SPELL_CONSORTS_ICE_COMET       = 137722,
    SPELL_CONSORTS_MOON_LOTUS      = 136690
};

// Sourced from Lei Shen's wowhead strategy-guide page.
enum LeiShenSpells
{
    SPELL_LEISHEN_DECAPITATE       = 134916,
    SPELL_LEISHEN_THUNDERSTRUCK    = 135095,
    SPELL_LEISHEN_CRASHING_THUNDER = 135150,
    SPELL_LEISHEN_LIGHTNING_WHIP   = 136850
};
