/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

uint32_t const HellfireCitadelEncounterCount = 13;

// slot order follows the dungeon encounter ids, the order instance_encounters rows load in
enum HellfireCitadelData
{
    DATA_FEL_LORD_ZAKUUN        = 0,    // 1777
    DATA_HELLFIRE_ASSAULT       = 1,    // 1778
    DATA_GOREFIEND              = 2,    // 1783
    DATA_TYRANT_VELHARI         = 3,    // 1784
    DATA_IRON_REAVER            = 4,    // 1785
    DATA_KILROGG_DEADEYE        = 5,    // 1786
    DATA_KORMROK                = 6,    // 1787
    DATA_SHADOW_LORD_ISKAR      = 7,    // 1788
    DATA_SOCRETHAR              = 8,    // 1794
    DATA_MANNOROTH              = 9,    // 1795
    DATA_HELLFIRE_HIGH_COUNCIL  = 10,   // 1798
    DATA_ARCHIMONDE             = 11,   // 1799
    DATA_XHULHORAC              = 12    // 1800
};

enum HellfireCitadelCreatures
{
    BOSS_SIEGEMASTER_MARTAK         = 93023,
    BOSS_IRON_REAVER                = 90284,
    BOSS_KORMROK                    = 90435,
    BOSS_DIA_DARKWHISPER            = 92144,
    BOSS_GURTOGG_BLOODBOIL          = 92146,
    BOSS_BLADEMASTER_JUBEITHOS      = 92142,
    BOSS_KILROGG_DEADEYE            = 90378,
    BOSS_GOREFIEND                  = 90199,
    BOSS_SHADOW_LORD_ISKAR          = 90316,
    BOSS_SOULBOUND_CONSTRUCT        = 90296,
    BOSS_SOCRETHAR                  = 92330,
    BOSS_FEL_LORD_ZAKUUN            = 89890,
    BOSS_XHULHORAC                  = 93068,
    BOSS_TYRANT_VELHARI             = 90269,
    BOSS_MANNOROTH                  = 91349,
    BOSS_ARCHIMONDE                 = 91331,

    // Hellfire Assault
    NPC_GOREBOUND_FELCASTER         = 93931,
    NPC_HULKING_BERSERKER           = 92911,
    NPC_IRON_DRAGOON                = 90114,

    // Hellfire High Council
    NPC_JUBEITHOS_MIRROR_IMAGE      = 92993,

    // Kilrogg Deadeye
    NPC_SALIVATING_BLOODTHIRSTER    = 90521,
    NPC_HULKING_TERROR              = 90523,

    // Gorefiend
    NPC_CORRUPTED_SOUL              = 93288,

    // Shadow-Lord Iskar
    NPC_CORRUPTED_TALONPRIEST       = 91543,
    NPC_SHADOWFEL_WARDEN            = 91541,
    NPC_ISKAR_FEL_RAVEN             = 91539,

    // Socrethar the Eternal
    NPC_SARGEREI_DOMINATOR          = 92767,
    NPC_SARGEREI_SHADOWCALLER       = 91941,
    NPC_HAUNTING_SOUL               = 91938,

    // Xhul'horac
    NPC_VANGUARD_AKKELION           = 94185,
    NPC_OMNUS                       = 94239,
    NPC_WILD_PYROMANIAC             = 94231,
    NPC_UNSTABLE_VOIDFIEND          = 94397,

    // Tyrant Velhari
    NPC_ANCIENT_ENFORCER            = 90270,
    NPC_ANCIENT_HARBINGER           = 90271,
    NPC_ANCIENT_SOVEREIGN           = 90272,

    // Mannoroth
    NPC_FEL_IRON_SUMMONER           = 91305,
    NPC_MANNOROTH_DOOM_LORD         = 91241,
    NPC_MANNOROTH_FEL_IMP           = 91259,
    NPC_MANNOROTH_DREAD_INFERNAL    = 91270,

    // Archimonde
    NPC_HELLFIRE_DEATHCALLER        = 92740,
    NPC_FELBORNE_OVERFIEND          = 93615,
    NPC_INFERNAL_DOOMBRINGER        = 94412
};

enum HellfireCitadelGameObjects
{
    GO_HFC_HELLFIRE_GATE            = 242454,
    GO_HFC_REINFORCED_DOOR          = 242464,
    GO_HFC_ZAKUUN_ENTRANCE          = 242510,
    GO_HFC_ZAKUUN_EXIT              = 240687,
    GO_HFC_SOCRETHAR_DOOR_1         = 242362,
    GO_HFC_SOCRETHAR_DOOR_2         = 242363,
    GO_HFC_SOCRETHAR_DOOR_3         = 242360,
    GO_HFC_SOCRETHAR_DOOR_4         = 242361,
    GO_HFC_ISKAR_DOOR_1             = 243183,
    GO_HFC_ISKAR_DOOR_2             = 242419,
    GO_HFC_ISKAR_DOOR_3             = 242280,
    GO_HFC_ISKAR_DOOR_4             = 243482,
    GO_HFC_VELHARI_EXIT_1           = 241649,
    GO_HFC_VELHARI_EXIT_2           = 241650,
    GO_HFC_VELHARI_ENTRANCE_1       = 242359,
    GO_HFC_VELHARI_ENTRANCE_2       = 242358,
    GO_HFC_DARK_PORTAL_FIRE_WALL    = 241485
};

enum HellfireCitadelGenericSpells
{
    SPELL_HFC_BERSERK                       = 26662
};

enum HellfireAssaultSpells
{
    SPELL_MARTAK_HOWLING_AXE                = 184369,
    SPELL_MARTAK_SHOCKWAVE                  = 184394,
    SPELL_MARTAK_INSPIRING_PRESENCE         = 185090,

    SPELL_FELCASTER_FELFIRE_VOLLEY          = 183452,
    SPELL_BERSERKER_SLAM                    = 184243,
    SPELL_DRAGOON_CONDUCTED_SHOCK_PULSE     = 185806
};

enum IronReaverSpells
{
    SPELL_REAVER_BARRAGE                    = 185282,
    SPELL_REAVER_UNSTABLE_ORB               = 182001,
    SPELL_REAVER_POUNDING                   = 182020,
    SPELL_REAVER_ARTILLERY                  = 182280,
    SPELL_REAVER_BLITZ                      = 179889,
    SPELL_REAVER_FIREBOMB                   = 181999,
    SPELL_REAVER_FALLING_SLAM               = 182066
};

enum KormrokSpells
{
    SPELL_KORMROK_SWAT                      = 181305,
    SPELL_KORMROK_POUND                     = 180244,
    SPELL_KORMROK_EXPLOSIVE_BURST           = 181306,
    SPELL_KORMROK_FOUL_CRUSH                = 181307,
    SPELL_KORMROK_EXPLOSIVE_RUNES           = 181296,
    SPELL_KORMROK_EMPOWERED_EXPLOSIVE_RUNES = 181297,
    SPELL_KORMROK_GRASPING_HANDS            = 181299,
    SPELL_KORMROK_DRAGGING_HANDS            = 181300,
    SPELL_KORMROK_FEL_OUTPOURING            = 181292,
    SPELL_KORMROK_EMPOWERED_FEL_OUTPOURING  = 181293,
    SPELL_KORMROK_SHADOW_ENERGY             = 180115,
    SPELL_KORMROK_EXPLOSIVE_ENERGY          = 180116,
    SPELL_KORMROK_FOUL_ENERGY               = 180117
};

enum HellfireHighCouncilSpells
{
    SPELL_DIA_NIGHTMARE_VISAGE              = 184657,
    SPELL_DIA_WAILING_HORROR                = 184681,
    SPELL_DIA_REAP                          = 184476,
    SPELL_DIA_MARK_OF_THE_NECROMANCER       = 184449,

    SPELL_GURTOGG_ACIDIC_WOUND              = 184847,
    SPELL_GURTOGG_FEL_RAGE                  = 184360,
    SPELL_GURTOGG_BLOODBOIL                 = 184355,
    SPELL_GURTOGG_DEMOLISHING_LEAP          = 184366,

    SPELL_JUBEITHOS_FELSTORM                = 183701,
    SPELL_JUBEITHOS_FEL_BLADE               = 183226,
    SPELL_JUBEITHOS_MIRROR_IMAGES           = 183885
};

enum KilroggDeadeyeSpells
{
    SPELL_KILROGG_SHRED_ARMOR               = 180199,
    SPELL_KILROGG_HEART_SEEKER              = 180372,
    SPELL_KILROGG_DEATH_THROES              = 180224,
    SPELL_KILROGG_VISION_OF_DEATH           = 182428,

    SPELL_HULKING_TERROR_RENDING_HOWL       = 183917
};

enum GorefiendSpells
{
    SPELL_GOREFIEND_SHADOW_OF_DEATH         = 179864,
    SPELL_GOREFIEND_TOUCH_OF_DOOM           = 179977,
    SPELL_GOREFIEND_SHARED_FATE             = 179909,
    SPELL_GOREFIEND_CRUSHING_DARKNESS       = 180017,
    SPELL_GOREFIEND_FEAST_OF_SOULS          = 181973
};

enum ShadowLordIskarSpells
{
    SPELL_ISKAR_EYE_OF_ANZU                 = 179202,
    SPELL_ISKAR_PHANTASMAL_WINDS            = 181957,
    SPELL_ISKAR_PHANTASMAL_WOUNDS           = 182325,
    SPELL_ISKAR_FEL_CHAKRAM                 = 182178,
    SPELL_ISKAR_FEL_BOMB                    = 181753,
    SPELL_ISKAR_FOCUSED_BLAST               = 181912,

    SPELL_TALONPRIEST_PHANTASMAL_CORRUPTION = 181824,
    SPELL_TALONPRIEST_SHADOW_RIPOSTE        = 185345,
    SPELL_WARDEN_FEL_CONDUIT                = 181827,
    SPELL_WARDEN_PHANTASMAL_FEL_BOMB        = 179219
};

enum SocretharSpells
{
    SPELL_CONSTRUCT_REVERBERATING_BLOW      = 180008,
    SPELL_CONSTRUCT_FELBLAZE_CHARGE         = 182051,
    SPELL_CONSTRUCT_VOLATILE_FEL_ORB        = 180221,

    SPELL_SOCRETHAR_SHADOW_WORD_AGONY       = 184239,
    SPELL_SOCRETHAR_APOCALYPSE              = 183329,

    SPELL_DOMINATOR_GIFT_OF_THE_MANARI      = 184124,
    SPELL_HAUNTING_SOUL_GHASTLY_FIXATION    = 182769
};

enum FelLordZakuunSpells
{
    SPELL_ZAKUUN_HEAVILY_ARMED              = 179671,
    SPELL_ZAKUUN_DISARMED                   = 179667,
    SPELL_ZAKUUN_THROW_AXE                  = 179668,
    SPELL_ZAKUUN_SOUL_CLEAVE                = 179406,
    SPELL_ZAKUUN_RUMBLING_FISSURES          = 179583,
    SPELL_ZAKUUN_BEFOULED                   = 179711,
    SPELL_ZAKUUN_CAVITATION                 = 181461,
    SPELL_ZAKUUN_SEED_OF_DESTRUCTION        = 181508,
    SPELL_ZAKUUN_WAKE_OF_DESTRUCTION        = 181499,
    SPELL_ZAKUUN_ENRAGE                     = 179681
};

enum XhulhoracSpells
{
    SPELL_XHUL_FEL_STRIKE                   = 186271,
    SPELL_XHUL_VOID_STRIKE                  = 186292,
    SPELL_XHUL_FEL_SURGE                    = 186407,
    SPELL_XHUL_VOID_SURGE                   = 186333,
    SPELL_XHUL_OVERWHELMING_CHAOS           = 187209,

    SPELL_AKKELION_FELBLAZE_FLURRY          = 186453,
    SPELL_OMNUS_WITHERING_GAZE              = 186783,
    SPELL_OMNUS_BLACK_HOLE                  = 186546
};

enum TyrantVelhariSpells
{
    SPELL_VELHARI_AURA_OF_OPPRESSION        = 181718,
    SPELL_VELHARI_AURA_OF_CONTEMPT          = 179986,
    SPELL_VELHARI_AURA_OF_MALICE            = 179991,
    SPELL_VELHARI_EDICT_OF_CONDEMNATION     = 182459,
    SPELL_VELHARI_INFERNAL_TEMPEST          = 180300,
    SPELL_VELHARI_ANNIHILATING_STRIKE       = 180260,
    SPELL_VELHARI_TAINTED_SHADOWS           = 180533,
    SPELL_VELHARI_TOUCH_OF_HARM             = 180166,
    SPELL_VELHARI_SEAL_OF_DECAY             = 180000,
    SPELL_VELHARI_FONT_OF_CORRUPTION        = 180526,
    SPELL_VELHARI_GAVEL_OF_THE_TYRANT       = 180608,

    SPELL_ENFORCER_ONSLAUGHT                = 180004,
    SPELL_HARBINGER_MENDING                 = 180025,
    SPELL_SOVEREIGN_WARD                    = 180040
};

enum MannorothSpells
{
    SPELL_MANNOROTH_GLAIVE_COMBO            = 181354,
    SPELL_MANNOROTH_MASSIVE_BLAST           = 181359,
    SPELL_MANNOROTH_FEL_HELLSTORM           = 181557,
    SPELL_MANNOROTH_EMPOWERED_FEL_HELLSTORM = 181948,
    SPELL_MANNOROTH_SHADOWFORCE             = 181799,
    SPELL_MANNOROTH_EMPOWERED_SHADOWFORCE   = 182084,
    SPELL_MANNOROTH_GAZE                    = 181597,
    SPELL_MANNOROTH_EMPOWERED_GAZE          = 182006,
    SPELL_MANNOROTH_CURSE_OF_THE_LEGION     = 181275,
    SPELL_MANNOROTH_FELSEEKER               = 181735,

    SPELL_DOOM_LORD_MARK_OF_DOOM            = 181099,
    SPELL_DOOM_LORD_SHADOW_BOLT_VOLLEY      = 181126,
    SPELL_FEL_IMP_FEL_BLAST                 = 181134,
    SPELL_DREAD_INFERNAL_FEL_HELLFIRE       = 181191
};

enum ArchimondeSpells
{
    SPELL_ARCHIMONDE_DOOMFIRE               = 182826,
    SPELL_ARCHIMONDE_ALLURE_OF_FLAMES       = 183254,
    SPELL_ARCHIMONDE_DEATH_BRAND            = 183828,
    SPELL_ARCHIMONDE_DESECRATE              = 185590,
    SPELL_ARCHIMONDE_SHADOWFEL_BURST        = 183817,
    SPELL_ARCHIMONDE_SHACKLED_TORMENT       = 184931,
    SPELL_ARCHIMONDE_WROUGHT_CHAOS          = 186123,
    SPELL_ARCHIMONDE_RAIN_OF_CHAOS          = 182225,

    SPELL_DEATHCALLER_SHADOW_BLAST          = 183864,
    SPELL_DEATHCALLER_DEMONIC_HAVOC         = 183865
};
