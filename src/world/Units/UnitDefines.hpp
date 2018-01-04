/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "CommonTypes.hpp"
#include "LocationVector.h"

enum DeathState
{
    ALIVE = 0,  // Unit is alive and well
    JUST_DIED,  // Unit has JUST died
    CORPSE,     // Unit has died but remains in the world as a corpse
    DEAD        // Unit is dead and his corpse is gone from the world
};

#define HIGHEST_FACTION = 46
enum Factions
{
    FACTION_BLOODSAIL_BUCCANEERS,
    FACTION_BOOTY_BAY,
    FACTION_GELKIS_CLAN_CENTAUR,
    FACTION_MAGRAM_CLAN_CENTAUR,
    FACTION_THORIUM_BROTHERHOOD,
    FACTION_RAVENHOLDT,
    FACTION_SYNDICATE,
    FACTION_GADGETZAN,
    FACTION_WILDHAMMER_CLAN,
    FACTION_RATCHET,
    FACTION_STEAMWHEEDLE_CARTEL,
    FACTION_ALLIANCE,
    FACTION_HORDE,
    FACTION_ARGENT_DAWN,
    FACTION_ORGRIMMAR,
    FACTION_DARKSPEAR_TROLLS,
    FACTION_THUNDER_BLUFF,
    FACTION_UNDERCITY,
    FACTION_GNOMEREGAN_EXILES,
    FACTION_STORMWIND,
    FACTION_IRONFORGE,
    FACTION_DARNASSUS,
    FACTION_LEATHERWORKING_DRAGON,
    FACTION_LEATHERWORKING_ELEMENTAL,
    FACTION_LEATHERWORKING_TRIBAL,
    FACTION_ENGINEERING_GNOME,
    FACTION_ENGINEERING_GOBLIN,
    FACTION_WINTERSABER_TRAINERS,
    FACTION_EVERLOOK,
    FACTION_BLACKSMITHING_ARMOR,
    FACTION_BLACKSMITHING_WEAPON,
    FACTION_BLACKSMITHING_AXE,
    FACTION_BLACKSMITHING_SWORD,
    FACTION_BLACKSMITHING_HAMMER,
    FACTION_CAER_DARROW,
    FACTION_TIMBERMAW_HOLD,
    FACTION_CENARION_CIRCLE,
    FACTION_THRALLMAR,
    FACTION_HONOR_HOLD,
    FACTION_THE_SHA_TAR,
    FACTION_STORMPIKE_GUARDS,
    FACTION_FROSTWOLF_CLAN,
    FACTION_HYDRAXIAN_WATERLORDS,
    FACTION_OUTLAND,
    FACTION_SHEN_DRALAR,
    FACTION_SILVERWING_SENTINELS,
    FACTION_WARSONG_OUTRIDERS,
    FACTION_ALLIANCE_FORCES,
    FACTION_HORDE_FORCES,
    FACTION_EXODAR,
    FACTION_DARKMOON_FAIRE,
    FACTION_ZANDALAR_TRIBE,
    FACTION_THE_DEFILERS,
    FACTION_THE_LEAGUE_OF_ARATHOR,
    FACTION_BROOD_OF_NOZDORMU,
    FACTION_SILVERMOON_CITY,
    FACTION_TRANQUILLIEN,
    FACTION_THE_SCALE_OF_THE_SANDS,
    FACTION_THE_ALDOR,
    FACTION_SHATTRATH_CITY,
    FACTION_THE_CONSORTIUM,
    FACTION_THE_MAG_HAR,
    FACTION_THE_SCRYERS,
    FACTION_THE_VIOLET_EYE,
    FACTION_CENARION_EXPEDITION,
    FACTION_SPOREGGAR,
    FACTION_KURENAI,
    FACTION_KEEPERS_OF_TIME,
    FACTION_FRIENDLY_HIDDEN,
    FACTION_LOWER_CITY,
    FACTION_ASHTONGUE_DEATHSWORN,
    FACTION_NETHERWING,
    FACTION_SHATARI_SKYGUARD,
    FACTION_OGRI_LA,
    FACTION_SHATTERED_SUN_OFFENSIVE = 80
};

enum TextEmoteType
{
    TEXTEMOTE_AGREE             = 1,
    TEXTEMOTE_AMAZE             = 2,
    TEXTEMOTE_ANGRY             = 3,
    TEXTEMOTE_APOLOGIZE         = 4,
    TEXTEMOTE_APPLAUD           = 5,
    TEXTEMOTE_BASHFUL           = 6,
    TEXTEMOTE_BECKON            = 7,
    TEXTEMOTE_BEG               = 8,
    TEXTEMOTE_BITE              = 9,
    TEXTEMOTE_BLEED             = 10,
    TEXTEMOTE_BLINK             = 11,
    TEXTEMOTE_BLUSH             = 12,
    TEXTEMOTE_BONK              = 13,
    TEXTEMOTE_BORED             = 14,
    TEXTEMOTE_BOUNCE            = 15,
    TEXTEMOTE_BRB               = 16,
    TEXTEMOTE_BOW               = 17,
    TEXTEMOTE_BURP              = 18,
    TEXTEMOTE_BYE               = 19,
    TEXTEMOTE_CACKLE            = 20,
    TEXTEMOTE_CHEER             = 21,
    TEXTEMOTE_CHICKEN           = 22,
    TEXTEMOTE_CHUCKLE           = 23,
    TEXTEMOTE_CLAP              = 24,
    TEXTEMOTE_CONFUSED          = 25,
    TEXTEMOTE_CONGRATULATE      = 26,
    TEXTEMOTE_COUGH             = 27,
    TEXTEMOTE_COWER             = 28,
    TEXTEMOTE_CRACK             = 29,
    TEXTEMOTE_CRINGE            = 30,
    TEXTEMOTE_CRY               = 31,
    TEXTEMOTE_CURIOUS           = 32,
    TEXTEMOTE_CURTSEY           = 33,
    TEXTEMOTE_DANCE             = 34,
    TEXTEMOTE_DRINK             = 35,
    TEXTEMOTE_DROOL             = 36,
    TEXTEMOTE_EAT               = 37,
    TEXTEMOTE_EYE               = 38,
    TEXTEMOTE_FART              = 39,
    TEXTEMOTE_FIDGET            = 40,
    TEXTEMOTE_FLEX              = 41,
    TEXTEMOTE_FROWN             = 42,
    TEXTEMOTE_GASP              = 43,
    TEXTEMOTE_GAZE              = 44,
    TEXTEMOTE_GIGGLE            = 45,
    TEXTEMOTE_GLARE             = 46,
    TEXTEMOTE_GLOAT             = 47,
    TEXTEMOTE_GREET             = 48,
    TEXTEMOTE_GRIN              = 49,
    TEXTEMOTE_GROAN             = 50,
    TEXTEMOTE_GROVEL            = 51,
    TEXTEMOTE_GUFFAW            = 52,
    TEXTEMOTE_HAIL              = 53,
    TEXTEMOTE_HAPPY             = 54,
    TEXTEMOTE_HELLO             = 55,
    TEXTEMOTE_HUG               = 56,
    TEXTEMOTE_HUNGRY            = 57,
    TEXTEMOTE_KISS              = 58,
    TEXTEMOTE_KNEEL             = 59,
    TEXTEMOTE_LAUGH             = 60,
    TEXTEMOTE_LAYDOWN           = 61,
    TEXTEMOTE_MASSAGE           = 62,
    TEXTEMOTE_MOAN              = 63,
    TEXTEMOTE_MOON              = 64,
    TEXTEMOTE_MOURN             = 65,
    TEXTEMOTE_NO                = 66,
    TEXTEMOTE_NOD               = 67,
    TEXTEMOTE_NOSEPICK          = 68,
    TEXTEMOTE_PANIC             = 69,
    TEXTEMOTE_PEER              = 70,
    TEXTEMOTE_PLEAD             = 71,
    TEXTEMOTE_POINT             = 72,
    TEXTEMOTE_POKE              = 73,
    TEXTEMOTE_PRAY              = 74,
    TEXTEMOTE_ROAR              = 75,
    TEXTEMOTE_ROFL              = 76,
    TEXTEMOTE_RUDE              = 77,
    TEXTEMOTE_SALUTE            = 78,
    TEXTEMOTE_SCRATCH           = 79,
    TEXTEMOTE_SEXY              = 80,
    TEXTEMOTE_SHAKE             = 81,
    TEXTEMOTE_SHOUT             = 82,
    TEXTEMOTE_SHRUG             = 83,
    TEXTEMOTE_SHY               = 84,
    TEXTEMOTE_SIGH              = 85,
    TEXTEMOTE_SIT               = 86,
    TEXTEMOTE_SLEEP             = 87,
    TEXTEMOTE_SNARL             = 88,
    TEXTEMOTE_SPIT              = 89,
    TEXTEMOTE_STARE             = 90,
    TEXTEMOTE_SURPRISED         = 91,
    TEXTEMOTE_SURRENDER         = 92,
    TEXTEMOTE_TALK              = 93,
    TEXTEMOTE_TALKEX            = 94,
    TEXTEMOTE_TALKQ             = 95,
    TEXTEMOTE_TAP               = 96,
    TEXTEMOTE_THANK             = 97,
    TEXTEMOTE_THREATEN          = 98,
    TEXTEMOTE_TIRED             = 99,
    TEXTEMOTE_VICTORY           = 100,
    TEXTEMOTE_WAVE              = 101,
    TEXTEMOTE_WELCOME           = 102,
    TEXTEMOTE_WHINE             = 103,
    TEXTEMOTE_WHISTLE           = 104,
    TEXTEMOTE_WORK              = 105,
    TEXTEMOTE_YAWN              = 106,
    TEXTEMOTE_BOGGLE            = 107,
    TEXTEMOTE_CALM              = 108,
    TEXTEMOTE_COLD              = 109,
    TEXTEMOTE_COMFORT           = 110,
    TEXTEMOTE_CUDDLE            = 111,
    TEXTEMOTE_DUCK              = 112,
    TEXTEMOTE_INSULT            = 113,
    TEXTEMOTE_INTRODUCE         = 114,
    TEXTEMOTE_JK                = 115,
    TEXTEMOTE_LICK              = 116,
    TEXTEMOTE_LISTEN            = 117,
    TEXTEMOTE_LOST              = 118,
    TEXTEMOTE_MOCK              = 119,
    TEXTEMOTE_PONDER            = 120,
    TEXTEMOTE_POUNCE            = 121,
    TEXTEMOTE_PRAISE            = 122,
    TEXTEMOTE_PURR              = 123,
    TEXTEMOTE_PUZZLE            = 124,
    TEXTEMOTE_RAISE             = 125,
    TEXTEMOTE_READY             = 126,
    TEXTEMOTE_SHIMMY            = 127,
    TEXTEMOTE_SHIVER            = 128,
    TEXTEMOTE_SHOO              = 129,
    TEXTEMOTE_SLAP              = 130,
    TEXTEMOTE_SMIRK             = 131,
    TEXTEMOTE_SNIFF             = 132,
    TEXTEMOTE_SNUB              = 133,
    TEXTEMOTE_SOOTHE            = 134,
    TEXTEMOTE_STINK             = 135,
    TEXTEMOTE_TAUNT             = 136,
    TEXTEMOTE_TEASE             = 137,
    TEXTEMOTE_THIRSTY           = 138,
    TEXTEMOTE_VETO              = 139,
    TEXTEMOTE_SNICKER           = 140,
    TEXTEMOTE_STAND             = 141,
    TEXTEMOTE_TICKLE            = 142,
    TEXTEMOTE_VIOLIN            = 143,
    TEXTEMOTE_SMILE             = 163,
    TEXTEMOTE_RASP              = 183,
    TEXTEMOTE_PITY              = 203,
    TEXTEMOTE_GROWL             = 204,
    TEXTEMOTE_BARK              = 205,
    TEXTEMOTE_SCARED            = 223,
    TEXTEMOTE_FLOP              = 224,
    TEXTEMOTE_LOVE              = 225,
    TEXTEMOTE_MOO               = 226,
    TEXTEMOTE_COMMEND           = 243,
    TEXTEMOTE_TRAIN             = 264,
    TEXTEMOTE_HELPME            = 303,
    TEXTEMOTE_INCOMING          = 304,
    TEXTEMOTE_CHARGE            = 305,
    TEXTEMOTE_FLEE              = 306,
    TEXTEMOTE_ATTACKMYTARGET    = 307,
    TEXTEMOTE_OOM               = 323,
    TEXTEMOTE_FOLLOW            = 324,
    TEXTEMOTE_WAIT              = 325,
    TEXTEMOTE_HEALME            = 326,
    TEXTEMOTE_OPENFIRE          = 327,
    TEXTEMOTE_FLIRT             = 328,
    TEXTEMOTE_JOKE              = 329,
    TEXTEMOTE_GOLFCLAP          = 343,
    TEXTEMOTE_WINK              = 363,
    TEXTEMOTE_PAT               = 364,
    TEXTEMOTE_SERIOUS           = 365,
    TEXTEMOTE_MOUNTSPECIAL      = 366,
    TEXTEMOTE_GOODLUCK          = 367,
    TEXTEMOTE_BLAME             = 368,
    TEXTEMOTE_BLANK             = 369,
    TEXTEMOTE_BRANDISH          = 370,
    TEXTEMOTE_BREATH            = 371,
    TEXTEMOTE_DISAGREE          = 372,
    TEXTEMOTE_DOUBT             = 373,
    TEXTEMOTE_EMBARRASS         = 374,
    TEXTEMOTE_ENCOURAGE         = 375,
    TEXTEMOTE_ENEMY             = 376,
    TEXTEMOTE_EYEBROW           = 377,
    TEXTEMOTE_TOAST             = 378,
    TEXTEMOTE_FAIL              = 379,
    TEXTEMOTE_HIGHFIVE          = 380,
    TEXTEMOTE_ABSENT            = 381,
    TEXTEMOTE_ARM               = 382,
    TEXTEMOTE_AWE               = 383,
    TEXTEMOTE_BACKPACK          = 384,
    TEXTEMOTE_BADFEELING        = 385,
    TEXTEMOTE_CHALLENGE         = 386,
    TEXTEMOTE_CHUG              = 387,
    TEXTEMOTE_DING              = 389,
    TEXTEMOTE_FACEPALM          = 390,
    TEXTEMOTE_FAINT             = 391,
    TEXTEMOTE_GO                = 392,
    TEXTEMOTE_GOING             = 393,
    TEXTEMOTE_GLOWER            = 394,
    TEXTEMOTE_HEADACHE          = 395,
    TEXTEMOTE_HICCUP            = 396,
    TEXTEMOTE_HISS              = 398,
    TEXTEMOTE_HOLDHAND          = 399,
    TEXTEMOTE_HURRY             = 401,
    TEXTEMOTE_IDEA              = 402,
    TEXTEMOTE_JEALOUS           = 403,
    TEXTEMOTE_LUCK              = 404,
    TEXTEMOTE_MAP               = 405,
    TEXTEMOTE_MERCY             = 406,
    TEXTEMOTE_MUTTER            = 407,
    TEXTEMOTE_NERVOUS           = 408,
    TEXTEMOTE_OFFER             = 409,
    TEXTEMOTE_PET               = 410,
    TEXTEMOTE_PINCH             = 411,
    TEXTEMOTE_PROUD             = 413,
    TEXTEMOTE_PROMISE           = 414,
    TEXTEMOTE_PULSE             = 415,
    TEXTEMOTE_PUNCH             = 416,
    TEXTEMOTE_POUT              = 417,
    TEXTEMOTE_REGRET            = 418,
    TEXTEMOTE_REVENGE           = 420,
    TEXTEMOTE_ROLLEYES          = 421,
    TEXTEMOTE_RUFFLE            = 422,
    TEXTEMOTE_SAD               = 423,
    TEXTEMOTE_SCOFF             = 424,
    TEXTEMOTE_SCOLD             = 425,
    TEXTEMOTE_SCOWL             = 426,
    TEXTEMOTE_SEARCH            = 427,
    TEXTEMOTE_SHAKEFIST         = 428,
    TEXTEMOTE_SHIFTY            = 429,
    TEXTEMOTE_SHUDDER           = 430,
    TEXTEMOTE_SIGNAL            = 431,
    TEXTEMOTE_SILENCE           = 432,
    TEXTEMOTE_SING              = 433,
    TEXTEMOTE_SMACK             = 434,
    TEXTEMOTE_SNEAK             = 435,
    TEXTEMOTE_SNEEZE            = 436,
    TEXTEMOTE_SNORT             = 437,
    TEXTEMOTE_SQUEAL            = 438,
    TEXTEMOTE_STOPATTACK        = 439,
    TEXTEMOTE_SUSPICIOUS        = 440,
    TEXTEMOTE_THINK             = 441,
    TEXTEMOTE_TRUCE             = 442,
    TEXTEMOTE_TWIDDLE           = 443,
    TEXTEMOTE_WARN              = 444,
    TEXTEMOTE_SNAP              = 445,
    TEXTEMOTE_CHARM             = 446,
    TEXTEMOTE_COVEREARS         = 447,
    TEXTEMOTE_CROSSARMS         = 448,
    TEXTEMOTE_LOOK              = 449,
    TEXTEMOTE_OBJECT            = 450,
    TEXTEMOTE_SWEAT             = 451,
};

enum EmoteType
{
    EMOTE_ONESHOT_NONE                  = 0,
    EMOTE_ONESHOT_TALK                  = 1, // DNR
    EMOTE_ONESHOT_BOW                   = 2,
    EMOTE_ONESHOT_WAVE                  = 3, // DNR
    EMOTE_ONESHOT_CHEER                 = 4, // DNR
    EMOTE_ONESHOT_EXCLAMATION           = 5, // DNR
    EMOTE_ONESHOT_QUESTION              = 6,
    EMOTE_ONESHOT_EAT                   = 7,
    EMOTE_STATE_DANCE                   = 10,
    EMOTE_ONESHOT_LAUGH                 = 11,
    EMOTE_STATE_SLEEP                   = 12,
    EMOTE_STATE_SIT                     = 13,
    EMOTE_ONESHOT_RUDE                  = 14, // DNR
    EMOTE_ONESHOT_ROAR                  = 15, // DNR
    EMOTE_ONESHOT_KNEEL                 = 16,
    EMOTE_ONESHOT_KISS                  = 17,
    EMOTE_ONESHOT_CRY                   = 18,
    EMOTE_ONESHOT_CHICKEN               = 19,
    EMOTE_ONESHOT_BEG                   = 20,
    EMOTE_ONESHOT_APPLAUD               = 21,
    EMOTE_ONESHOT_SHOUT                 = 22, // DNR
    EMOTE_ONESHOT_FLEX                  = 23,
    EMOTE_ONESHOT_SHY                   = 24, // DNR
    EMOTE_ONESHOT_POINT                 = 25, // DNR
    EMOTE_STATE_STAND                   = 26,
    EMOTE_STATE_READYUNARMED            = 27,
    EMOTE_STATE_WORK_SHEATHED           = 28,
    EMOTE_STATE_POINT                   = 29, // DNR
    EMOTE_STATE_NONE                    = 30,
    EMOTE_ONESHOT_WOUND                 = 33,
    EMOTE_ONESHOT_WOUNDCRITICAL         = 34,
    EMOTE_ONESHOT_ATTACKUNARMED         = 35,
    EMOTE_ONESHOT_ATTACK1H              = 36,
    EMOTE_ONESHOT_ATTACK2HTIGHT         = 37,
    EMOTE_ONESHOT_ATTACK2HLOOSE         = 38,
    EMOTE_ONESHOT_PARRYUNARMED          = 39,
    EMOTE_ONESHOT_PARRYSHIELD           = 43,
    EMOTE_ONESHOT_READYUNARMED          = 44,
    EMOTE_ONESHOT_READY1H               = 45,
    EMOTE_ONESHOT_READYBOW              = 48,
    EMOTE_ONESHOT_SPELLPRECAST          = 50,
    EMOTE_ONESHOT_SPELLCAST             = 51,
    EMOTE_ONESHOT_BATTLEROAR            = 53,
    EMOTE_ONESHOT_SPECIALATTACK1H       = 54,
    EMOTE_ONESHOT_KICK                  = 60,
    EMOTE_ONESHOT_ATTACKTHROWN          = 61,
    EMOTE_STATE_STUN                    = 64,
    EMOTE_STATE_DEAD                    = 65,
    EMOTE_ONESHOT_SALUTE                = 66,
    EMOTE_STATE_KNEEL                   = 68,
    EMOTE_STATE_USESTANDING             = 69,
    EMOTE_ONESHOT_WAVE_NOSHEATHE        = 70,
    EMOTE_ONESHOT_CHEER_NOSHEATHE       = 71,
    EMOTE_ONESHOT_EAT_NOSHEATHE         = 92,
    EMOTE_STATE_STUN_NOSHEATHE          = 93,
    EMOTE_ONESHOT_DANCE                 = 94,
    EMOTE_ONESHOT_SALUTE_NOSHEATH       = 113,
    EMOTE_STATE_USESTANDING_NOSHEATHE   = 133,
    EMOTE_ONESHOT_LAUGH_NOSHEATHE       = 153,
    EMOTE_STATE_WORK                    = 173,
    EMOTE_STATE_SPELLPRECAST            = 193,
    EMOTE_ONESHOT_READYRIFLE            = 213,
    EMOTE_STATE_READYRIFLE              = 214,
    EMOTE_STATE_WORK_MINING             = 233,
    EMOTE_STATE_WORK_CHOPWOOD           = 234,
    EMOTE_STATE_APPLAUD                 = 253,
    EMOTE_ONESHOT_LIFTOFF               = 254,
    EMOTE_ONESHOT_YES                   = 273, // DNR
    EMOTE_ONESHOT_NO                    = 274, // DNR
    EMOTE_ONESHOT_TRAIN                 = 275, // DNR
    EMOTE_ONESHOT_LAND                  = 293,
    EMOTE_STATE_AT_EASE                 = 313,
    EMOTE_STATE_READY1H                 = 333,
    EMOTE_STATE_SPELLKNEELSTART         = 353,
    EMOTE_STATE_SUBMERGED               = 373,
    EMOTE_ONESHOT_SUBMERGE              = 374,
    EMOTE_STATE_READY2H                 = 375,
    EMOTE_STATE_READYBOW                = 376,
    EMOTE_ONESHOT_MOUNTSPECIAL          = 377,
    EMOTE_STATE_TALK                    = 378,
    EMOTE_STATE_FISHING                 = 379,
    EMOTE_ONESHOT_FISHING               = 380,
    EMOTE_ONESHOT_LOOT                  = 381,
    EMOTE_STATE_WHIRLWIND               = 382,
    EMOTE_STATE_DROWNED                 = 383,
    EMOTE_STATE_HOLD_BOW                = 384,
    EMOTE_STATE_HOLD_RIFLE              = 385,
    EMOTE_STATE_HOLD_THROWN             = 386,
    EMOTE_ONESHOT_DROWN                 = 387,
    EMOTE_ONESHOT_STOMP                 = 388,
    EMOTE_ONESHOT_ATTACKOFF             = 389,
    EMOTE_ONESHOT_ATTACKOFFPIERCE       = 390,
    EMOTE_STATE_ROAR                    = 391,
    EMOTE_STATE_LAUGH                   = 392,
    EMOTE_ONESHOT_CREATURE_SPECIAL      = 393,
    EMOTE_ONESHOT_JUMPLANDRUN           = 394,
    EMOTE_ONESHOT_JUMPEND               = 395,
    EMOTE_ONESHOT_TALK_NOSHEATHE        = 396,
    EMOTE_ONESHOT_POINT_NOSHEATHE       = 397,
    EMOTE_STATE_CANNIBALIZE             = 398,
    EMOTE_ONESHOT_JUMPSTART             = 399,
    EMOTE_STATE_DANCESPECIAL            = 400,
    EMOTE_ONESHOT_DANCESPECIAL          = 401,
    EMOTE_ONESHOT_CUSTOMSPELL01         = 402,
    EMOTE_ONESHOT_CUSTOMSPELL02         = 403,
    EMOTE_ONESHOT_CUSTOMSPELL03         = 404,
    EMOTE_ONESHOT_CUSTOMSPELL04         = 405,
    EMOTE_ONESHOT_CUSTOMSPELL05         = 406,
    EMOTE_ONESHOT_CUSTOMSPELL06         = 407,
    EMOTE_ONESHOT_CUSTOMSPELL07         = 408,
    EMOTE_ONESHOT_CUSTOMSPELL08         = 409,
    EMOTE_ONESHOT_CUSTOMSPELL09         = 410,
    EMOTE_ONESHOT_CUSTOMSPELL10         = 411,
    EMOTE_STATE_EXCLAIM                 = 412,
    EMOTE_STATE_DANCE_CUSTOM            = 413,
    EMOTE_STATE_SIT_CHAIR_MED           = 415,
    EMOTE_STATE_CUSTOM_SPELL_01         = 416,
    EMOTE_STATE_CUSTOM_SPELL_02         = 417,
    EMOTE_STATE_EAT                     = 418,
    EMOTE_STATE_CUSTOM_SPELL_04         = 419,
    EMOTE_STATE_CUSTOM_SPELL_03         = 420,
    EMOTE_STATE_CUSTOM_SPELL_05         = 421,
    EMOTE_STATE_SPELLEFFECT_HOLD        = 422,
    EMOTE_STATE_EAT_NO_SHEATHE          = 423,
    EMOTE_STATE_MOUNT                   = 424,
    EMOTE_STATE_READY2HL                = 425,
    EMOTE_STATE_SIT_CHAIR_HIGH          = 426,
    EMOTE_STATE_FALL                    = 427,
    EMOTE_STATE_LOOT                    = 428,
    EMOTE_STATE_SUBMERGED_NEW           = 429, // NEW
    EMOTE_ONESHOT_COWER                 = 430, // DNR
    EMOTE_STATE_COWER                   = 431,
    EMOTE_ONESHOT_USESTANDING           = 432,
    EMOTE_STATE_STEALTH_STAND           = 433,
    EMOTE_ONESHOT_OMNICAST_GHOUL        = 434, // W/SOUND
    EMOTE_ONESHOT_ATTACKBOW             = 435,
    EMOTE_ONESHOT_ATTACKRIFLE           = 436,
    EMOTE_STATE_SWIM_IDLE               = 437,
    EMOTE_STATE_ATTACK_UNARMED          = 438,
    EMOTE_ONESHOT_SPELLCAST_W_SOUND     = 439,
    EMOTE_ONESHOT_DODGE                 = 440,
    EMOTE_ONESHOT_PARRY1H               = 441,
    EMOTE_ONESHOT_PARRY2H               = 442,
    EMOTE_ONESHOT_PARRY2HL              = 443,
    EMOTE_STATE_FLYFALL                 = 444,
    EMOTE_ONESHOT_FLYDEATH              = 445,
    EMOTE_STATE_FLY_FALL                = 446,
    EMOTE_ONESHOT_FLY_SIT_GROUND_DOWN   = 447,
    EMOTE_ONESHOT_FLY_SIT_GROUND_UP     = 448,
    EMOTE_ONESHOT_EMERGE                = 449,
    EMOTE_ONESHOT_DRAGONSPIT            = 450,
    EMOTE_STATE_SPECIALUNARMED          = 451,
    EMOTE_ONESHOT_FLYGRAB               = 452,
    EMOTE_STATE_FLYGRABCLOSED           = 453,
    EMOTE_ONESHOT_FLYGRABTHROWN         = 454,
    EMOTE_STATE_FLY_SIT_GROUND          = 455,
    EMOTE_STATE_WALKBACKWARDS           = 456,
    EMOTE_ONESHOT_FLYTALK               = 457,
    EMOTE_ONESHOT_FLYATTACK1H           = 458,
    EMOTE_STATE_CUSTOMSPELL08           = 459,
    EMOTE_ONESHOT_FLY_DRAGONSPIT        = 460,
    EMOTE_STATE_SIT_CHAIR_LOW           = 461,
    EMOTE_ONE_SHOT_STUN                 = 462,
    EMOTE_ONESHOT_SPELLCAST_OMNI        = 463,
    EMOTE_STATE_READYTHROWN             = 465,
    EMOTE_ONESHOT_WORK_CHOPWOOD         = 466,
    EMOTE_ONESHOT_WORK_MINING           = 467,
    EMOTE_STATE_SPELL_CHANNEL_OMNI      = 468,
    EMOTE_STATE_SPELL_CHANNEL_DIRECTED  = 469,
    EMOTE_STAND_STATE_NONE              = 470,
    EMOTE_STATE_READYJOUST              = 471,
    EMOTE_STATE_STRANGULATE             = 473,
    EMOTE_STATE_READY_SPELL_OMNI        = 474,
    EMOTE_STATE_HOLD_JOUST              = 475,
    EMOTE_ONESHOT_CRY_JAINA             = 476,
    EMOTE_ONESHOT_SPECIAL_UNARMED       = 477,
    EMOTE_STATE_DANCE_NOSHEATHE         = 478,
    EMOTE_ONESHOT_SNIFF                 = 479,
    EMOTE_ONESHOT_DRAGONSTOMP           = 480,
    EMOTE_ONESHOT_KNOCKDOWN             = 482,
    EMOTE_STATE_READ                    = 483,
    EMOTE_ONESHOT_FLYEMOTETALK          = 485,
    EMOTE_STATE_READ_ALLOWMOVEMENT      = 492,
    EMOTE_STATE_READY1H_ALLOW_MOVEMENT  = 505,
    EMOTE_STATE_READY2H_ALLOW_MOVEMENT  = 506,
    EMOTE_ONESHOT_OPEN                  = 517,
    EMOTE_STATE_READ_CHRISTMAS          = 518
};


enum UnitStates
{
    UNIT_STATE_NONE      = 0x0000,
    UNIT_STATE_DIED      = 0x0001,
    UNIT_STATE_ATTACKING = 0x0002,
    UNIT_STATE_DISARMED  = 0X0004,
    UNIT_STATE_CHARM     = 0x0008,
    UNIT_STATE_FEAR      = 0x0010,
    UNIT_STATE_STUN      = 0x0020,
    UNIT_STATE_CONFUSE   = 0x0040,
    UNIT_STATE_PACIFY    = 0x0080,
    UNIT_STATE_SILENCE   = 0x0100
};


// byte flags value (UNIT_FIELD_BYTES_1,0)
enum StandState : uint16_t
{
    STANDSTATE_STAND             = 0,
    STANDSTATE_SIT               = 1,
    STANDSTATE_SIT_CHAIR         = 2,
    STANDSTATE_SLEEP             = 3,
    STANDSTATE_SIT_LOW_CHAIR     = 4,
    STANDSTATE_SIT_MEDIUM_CHAIR  = 5,
    STANDSTATE_SIT_HIGH_CHAIR    = 6,
    STANDSTATE_DEAD              = 7,
    STANDSTATE_KNEEL             = 8,
    STANDSTATE_SUBMERGED         = 9
    //STANDSTATE_FORM_ALL          = 0x00FF0000,
    //STANDSTATE_FLAG_CREEP        = 0x02000000,
    //STANDSTATE_FLAG_UNTRACKABLE  = 0x04000000,
    //STANDSTATE_FLAG_ALL          = 0xFF000000
};

// byte flags value (UNIT_FIELD_BYTES_1,2)
enum UnitStandFlags
{
    UNIT_STAND_FLAGS_UNK1         = 0x01,
    UNIT_STAND_FLAGS_CREEP        = 0x02,
    UNIT_STAND_FLAGS_UNTRACKABLE  = 0x04,
    UNIT_STAND_FLAGS_UNK4         = 0x08,
    UNIT_STAND_FLAGS_UNK5         = 0x10,
    UNIT_STAND_FLAGS_ALL          = 0xFF
};

// byte flags value (UNIT_FIELD_BYTES_1,3)
enum UnitBytes1_Flags
{
    UNIT_BYTE1_FLAG_ALWAYS_STAND = 0x01,
    UNIT_BYTE1_FLAG_HOVER        = 0x02,    // It is called hover
    UNIT_BYTE1_FLAG_UNTRACKABLE  = 0x04,
    UNIT_BYTE1_FLAG_ALL          = 0xFF
};

// byte value (UNIT_FIELD_BYTES_2,0)
enum UnitBytes2_SheathState
{
    SHEATH_STATE_UNARMED  = 0,                              // non prepared weapon
    SHEATH_STATE_MELEE    = 1,                              // prepared melee weapon
    SHEATH_STATE_RANGED   = 2                               // prepared ranged weapon
};

// UNIT_FIELD_BYTES_2, 1
enum UnitBytes2_PvPFlags
{
    U_FIELD_BYTES_FLAG_PVP          = 0x01,
    U_FIELD_BYTES_FLAG_UNK1         = 0x04,
    U_FIELD_BYTES_FLAG_FFA_PVP      = 0x04,
    U_FIELD_BYTES_FLAG_SANCTUARY    = 0x08,
    U_FIELD_BYTES_FLAG_AURAS        = 0x10,
    U_FIELD_BYTES_FLAG_UNK2         = 0x20,
    U_FIELD_BYTES_FLAG_UNK3         = 0x40,
    U_FIELD_BYTES_FLAG_UNK4         = 0x80
};

// byte flags value (UNIT_FIELD_BYTES_2,2)
enum UnitBytes2_PetFlags
{
    UNIT_CAN_BE_RENAMED     = 0x01,
    UNIT_CAN_BE_ABANDONED   = 0x02,
};

// byte value (UNIT_FIELD_BYTES_2,3)
enum ShapeshiftForm
{
    FORM_NORMAL             = 0x00,
    FORM_CAT                = 0x01,
    FORM_TREE               = 0x02,
    FORM_TRAVEL             = 0x03,
    FORM_AQUA               = 0x04,
    FORM_BEAR               = 0x05,
    FORM_AMBIENT            = 0x06,
    FORM_GHOUL              = 0x07,
    FORM_DIREBEAR           = 0x08,
    FORM_SKELETON           = 0x0A,
    FORM_TEST_OF_STRENGTH   = 0x0B,
    FORM_BLB_PLAYER         = 0x0C,
    FORM_SHADOWDANCE        = 0x0D,
    FORM_CREATUREBEAR       = 0x0E,
    FORM_CREATURECAT        = 0x0F,
    FORM_GHOSTWOLF          = 0x10,
    FORM_BATTLESTANCE       = 0x11,
    FORM_DEFENSIVESTANCE    = 0x12,
    FORM_BERSERKERSTANCE    = 0x13,
    FORM_TEST               = 0x14,
    FORM_ZOMBIE             = 0x15,
    FORM_METAMORPHOSIS      = 0x16,
    FORM_DEMON              = 0x17,
    FORM_UNK2               = 0x18,
    FORM_UNDEAD             = 0x19,
    FORM_MASTER_ANGLER      = 0x1A,
    FORM_SWIFT              = 0x1B,
    FORM_SHADOW             = 0x1C,
    FORM_FLIGHT             = 0x1D,
    FORM_STEALTH            = 0x1E,
    FORM_MOONKIN            = 0x1F,
    FORM_SPIRITOFREDEMPTION = 0x20
};


enum UnitFieldFlags : uint32_t // UNIT_FIELD_FLAGS #46 - these are client flags
{
    //                                            Hex    Bit     Decimal  Comments
    UNIT_FLAG_SERVER_CONTROLLED          = 0x00000001, // 1            1
    UNIT_FLAG_NOT_ATTACKABLE_2           = 0x00000002, // 2            2  client won't let you attack them
    UNIT_FLAG_LOCK_PLAYER                = 0x00000004, // 3            4  ? does nothing to client (probably wrong) - only taxi code checks this
    UNIT_FLAG_PVP_ATTACKABLE             = 0x00000008, // 4            8  makes players and NPCs attackable / not attackable
    UNIT_FLAG_UNKNOWN_5                  = 0x00000010, // 5           16  ? some NPCs have this
    UNIT_FLAG_NO_REAGANT_COST            = 0x00000020, // 6           32  no reagant cost
    UNIT_FLAG_PLUS_MOB                   = 0x00000040, // 7           64  ? some NPCs have this (Rare/Elite/Boss?)
    UNIT_FLAG_IGNORE_CREATURE_COMBAT     = 0x00000080, // 8          128  unit will not enter combat with creatures
    UNIT_FLAG_IGNORE_PLAYER_COMBAT       = 0x00000100, // 9          256  unit will not enter combat with players
    UNIT_FLAG_UNKNOWN_10                 = 0x00000200, // 10         512  ? some NPCs have this
    UNIT_FLAG_LOOTING                    = 0x00000400, // 11        1024
    UNIT_FLAG_SELF_RES                   = 0x00000800, // 12        2048  ? some NPCs have this
    UNIT_FLAG_PVP                        = 0x00001000, // 13        4096  sets PvP flag
    UNIT_FLAG_SILENCED                   = 0x00002000, // 14        8192
    UNIT_FLAG_DEAD                       = 0x00004000, // 15       16384  used for special "dead" NPCs like Withered Corpses
    UNIT_FLAG_UNKNOWN_16                 = 0x00008000, // 16       32768  ? some NPCs have this
    UNIT_FLAG_ALIVE                      = 0x00010000, // 17       65536  ?
    UNIT_FLAG_PACIFIED                   = 0x00020000, // 18      131072
    UNIT_FLAG_STUNNED                    = 0x00040000, // 19      262144
    UNIT_FLAG_COMBAT                     = 0x00080000, // 20      524288  sets combat flag
    UNIT_FLAG_MOUNTED_TAXI               = 0x00100000, // 21     1048576  mounted on a taxi
    UNIT_FLAG_DISARMED                   = 0x00200000, // 22     2097152
    UNIT_FLAG_CONFUSED                   = 0x00400000, // 23     4194304
    UNIT_FLAG_FLEEING                    = 0x00800000, // 24     8388608  fear
    UNIT_FLAG_PLAYER_CONTROLLED_CREATURE = 0x01000000, // 25    16777216
    UNIT_FLAG_NOT_SELECTABLE             = 0x02000000, // 26    33554432  cannot select the unit
    UNIT_FLAG_SKINNABLE                  = 0x04000000, // 27    67108864
    UNIT_FLAG_MOUNT                      = 0x08000000, // 28   134217728  ? was MAKE_CHAR_UNTOUCHABLE (probably wrong), nothing ever set it
    UNIT_FLAG_UNKNOWN_29                 = 0x10000000, // 29   268435456
    UNIT_FLAG_FEIGN_DEATH                = 0x20000000, // 30   536870912
    UNIT_FLAG_UNKNOWN_31                 = 0x40000000, // 31  1073741824  ? was WEAPON_OFF and being used for disarm
    UNIT_FLAG_UNKNOWN_32                 = 0x80000000  // 32  2147483648
};

enum UnitFieldFlags2
{
    UNIT_FLAG2_FEIGN_DEATH                  = 0x0000001,
    UNIT_FLAG2_UNK1                         = 0x0000002,    // Hides body and body armor. Weapons, shoulder and head armors still visible
    UNIT_FLAG2_UNK2                         = 0x0000004,
    UNIT_FLAG2_COMPREHEND_LANG              = 0x0000008,    // Allows target to understand itself while talking in different language
    UNIT_FLAG2_MIRROR_IMAGE                 = 0x0000010,
    UNIT_FLAG2_UNK5                         = 0x0000020,
    UNIT_FLAG2_FORCE_MOVE                   = 0x0000040,    // Makes target to run forward
    UNIT_FLAG2_DISARM_OFFHAND               = 0x0000080,
    UNIT_FLAG2_UNK8                         = 0x0000100,
    UNIT_FLAG2_UNK9                         = 0x0000200,
    UNIT_FLAG2_DISARM_RANGED                = 0x0000400,
    UNIT_FLAG2_ENABLE_POWER_REGEN           = 0x0000800,
    UNIT_FLAG2_RESTRICT_PARTY_INTERACTION   = 0x0001000,   // Restrict interaction to party or raid
    UNIT_FLAG2_PREVENT_SPELL_CLICK          = 0x0002000,   // Prevent spellclick
    UNIT_FLAG2_ALLOW_ENEMY_INTERACT         = 0x0004000,
    UNIT_FLAG2_DISABLE_TURN                 = 0x0008000,
    UNIT_FLAG2_UNK10                        = 0x0010000,
    UNIT_FLAG2_PLAY_DEATH_ANIM              = 0x0020000,   // Plays special death animation upon death
    UNIT_FLAG2_ALLOW_CHEAT_SPELLS           = 0x0040000    // Allows casting spells with AttributesExG & SP_ATTR_EX_G_IS_CHEAT_SPELL
};

enum UnitDynamicFlags
{
    U_DYN_FLAG_LOOTABLE             = 0x01,
    U_DYN_FLAG_UNIT_TRACKABLE       = 0x02,
    U_DYN_FLAG_TAGGED_BY_OTHER      = 0x04,
    U_DYN_FLAG_TAPPED_BY_PLAYER     = 0x08,
    U_DYN_FLAG_PLAYER_INFO          = 0x10,
    U_DYN_FLAG_DEAD                 = 0x20,
};

enum DamageFlags
{
    DAMAGE_FLAG_MELEE   = 1,
    DAMAGE_FLAG_HOLY    = 2,
    DAMAGE_FLAG_FIRE    = 4,
    DAMAGE_FLAG_NATURE  = 8,
    DAMAGE_FLAG_FROST   = 16,
    DAMAGE_FLAG_SHADOW  = 32,
    DAMAGE_FLAG_ARCANE  = 64
};

enum WeaponDamageType // this is NOT the same as SPELL_ENTRY_Spell_Dmg_Type, or Spell::GetType(), or SPELL_ENTRY_School !!
{
    MELEE   = 0,
    OFFHAND = 1,
    RANGED  = 2,
};

enum VisualState
{
    ATTACK = 1,
    DODGE,
    PARRY,
    INTERRUPT,
    BLOCK,
    EVADE,
    IMMUNE,
    DEFLECT
};

enum HitStatus
{
    HITSTATUS_NORMALSWING       = 0x00000000,
    HITSTATUS_UNK_00            = 0x00000001,
    HITSTATUS_HITANIMATION      = 0x00000002,
    HITSTATUS_DUALWIELD         = 0x00000004,
    HITSTATUS_UNK_01            = 0x00000008,
    HITSTATUS_MISS              = 0x00000010,
    HITSTATUS_ABSORB_FULL       = 0x00000020,
    HITSTATUS_ABSORB_PARTIAL    = 0x00000040,
    HITSTATUS_RESIST_FULL       = 0x00000080,
    HITSTATUS_RESIST_PARTIAL    = 0x00000100,
    HITSTATUS_CRICTICAL         = 0x00000200,
    HITSTATUS_UNK_02            = 0x00000400,
    HITSTATUS_UNK_03            = 0x00000800,
    HITSTATUS_UNK_04            = 0x00001000,
    HITSTATUS_BLOCK             = 0x00002000,
    HITSTATUS_UNK_05            = 0x00004000,
    HITSTATUS_CRUSHINGBLOW      = 0x00008000,
    HITSTATUS_GLANCING          = 0x00010000,
    HITSTATUS_Crushing          = 0x00020000,
    HITSTATUS_NOACTION          = 0x00040000,
    HITSTATUS_UNK_06            = 0x00080000,
    HITSTATUS_UNK_07            = 0x00100000,
    HITSTATUS_SWINGNOHITSOUND   = 0x00200000,
    HITSTATUS_UNK_08            = 0x00400000,
    HITSTATUS_RAGE_GAIN         = 0x00800000,
    HITSTATUS_UNK_09            = 0x01000000,

    HITSTATUS_ABSORBED          = HITSTATUS_ABSORB_FULL | HITSTATUS_ABSORB_PARTIAL,
    HITSTATUS_RESIST            = HITSTATUS_RESIST_FULL | HITSTATUS_RESIST_PARTIAL
};

enum INVIS_FLAG
{
    INVIS_FLAG_NORMAL, // players and units with no special invisibility flags
    INVIS_FLAG_SPIRIT1,
    INVIS_FLAG_SPIRIT2,
    INVIS_FLAG_TRAP,
    INVIS_FLAG_QUEST,
    INVIS_FLAG_GHOST,
    INVIS_FLAG_UNKNOWN6,
    INVIS_FLAG_UNKNOWN7,
    INVIS_FLAG_SHADOWMOON,
    INVIS_FLAG_NETHERSTORM,
    INVIS_FLAG_BASHIR,
    INVIS_FLAG_UNKNOWN8,
    INVIS_FLAG_TOTAL
};

enum FIELD_PADDING//Since this field isn't used you can expand it for you needs
{
    PADDING_NONE
};

enum AURA_CHECK_RESULT
{
    AURA_CHECK_RESULT_NONE                  = 1,
    AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT   = 2,
    AURA_CHECK_RESULT_LOWER_BUFF_PRESENT    = 3
};

struct TransportData
{
    uint64 transportGuid;
    LocationVector relativePosition;
};

//////////////////////////////////////////////////////////////////////////////////////////
// values used in .npc info command
struct UnitFlagNames
{
    uint32 Flag;
    const char* Name;
};

struct UnitDynFlagNames
{
    uint32 Flag;
    const char* Name;
};

static const char* POWERTYPE[] =
{
    "Mana",
    "Rage",
    "Focus",
    "Energy",
    "Happiness",
    "Runes",
    "Runic Power"
};

static const UnitFlagNames UnitFlagToName[] =
{
    { UNIT_FLAG_SERVER_CONTROLLED, "UNIT_FLAG_SERVER_CONTROLLED" },
    { UNIT_FLAG_NOT_ATTACKABLE_2, "UNIT_FLAG_NOT_ATTACKABLE_2" },
    { UNIT_FLAG_LOCK_PLAYER, "UNIT_FLAG_LOCK_PLAYER" },
    { UNIT_FLAG_PVP_ATTACKABLE, "UNIT_FLAG_PVP_ATTACKABLE" },
    { UNIT_FLAG_UNKNOWN_5, "UNIT_FLAG_UNKNOWN_5" },
    { UNIT_FLAG_NO_REAGANT_COST, "UNIT_FLAG_NO_REAGANT_COST" },
    { UNIT_FLAG_PLUS_MOB, "UNIT_FLAG_PLUS_MOB" },
    { UNIT_FLAG_IGNORE_CREATURE_COMBAT, "UNIT_FLAG_IGNORE_CREATURE_COMBAT" },
    { UNIT_FLAG_IGNORE_PLAYER_COMBAT, "UNIT_FLAG_IGNORE_PLAYER_COMBAT" },
    { UNIT_FLAG_UNKNOWN_10, "UNIT_FLAG_UNKNOWN_10" },
    { UNIT_FLAG_LOOTING, "UNIT_FLAG_LOOTING" },
    { UNIT_FLAG_SELF_RES, "UNIT_FLAG_SELF_RES" },
    { UNIT_FLAG_PVP, "UNIT_FLAG_PVP" },
    { UNIT_FLAG_SILENCED, "UNIT_FLAG_SILENCED" },
    { UNIT_FLAG_DEAD, "UNIT_FLAG_DEAD" },
    { UNIT_FLAG_UNKNOWN_16, "UNIT_FLAG_UNKNOWN_16" },
    { UNIT_FLAG_ALIVE, "UNIT_FLAG_ALIVE" },
    { UNIT_FLAG_PACIFIED, "UNIT_FLAG_PACIFIED" },
    { UNIT_FLAG_STUNNED, "UNIT_FLAG_STUNNED" },
    { UNIT_FLAG_COMBAT, "UNIT_FLAG_COMBAT" },
    { UNIT_FLAG_MOUNTED_TAXI, "UNIT_FLAG_MOUNTED_TAXI" },
    { UNIT_FLAG_DISARMED, "UNIT_FLAG_DISARMED" },
    { UNIT_FLAG_CONFUSED, "UNIT_FLAG_CONFUSED" },
    { UNIT_FLAG_FLEEING, "UNIT_FLAG_FLEEING" },
    { UNIT_FLAG_PLAYER_CONTROLLED_CREATURE, "UNIT_FLAG_PLAYER_CONTROLLED_CREATURE" },
    { UNIT_FLAG_NOT_SELECTABLE, "UNIT_FLAG_NOT_SELECTABLE" },
    { UNIT_FLAG_SKINNABLE, "UNIT_FLAG_SKINNABLE" },
    { UNIT_FLAG_MOUNT, "UNIT_FLAG_MOUNT" },
    { UNIT_FLAG_UNKNOWN_29, "UNIT_FLAG_UNKNOWN_29" },
    { UNIT_FLAG_FEIGN_DEATH, "UNIT_FLAG_FEIGN_DEATH" },
    { UNIT_FLAG_UNKNOWN_31, "UNIT_FLAG_UNKNOWN_31" },
    { UNIT_FLAG_UNKNOWN_32, "UNIT_FLAG_UNKNOWN_32" }
};

static uint32 numflags = sizeof(UnitFlagToName) / sizeof(UnitFlagNames);

static const UnitDynFlagNames UnitDynFlagToName[] =
{
    { U_DYN_FLAG_LOOTABLE, "U_DYN_FLAG_LOOTABLE" },
    { U_DYN_FLAG_UNIT_TRACKABLE, "U_DYN_FLAG_UNIT_TRACKABLE" },
    { U_DYN_FLAG_TAGGED_BY_OTHER, "U_DYN_FLAG_TAGGED_BY_OTHER" },
    { U_DYN_FLAG_TAPPED_BY_PLAYER, "U_DYN_FLAG_TAPPED_BY_PLAYER" },
    { U_DYN_FLAG_PLAYER_INFO, "U_DYN_FLAG_PLAYER_INFO" },
    { U_DYN_FLAG_DEAD, "U_DYN_FLAG_DEAD" }
};

static uint32 numdynflags = sizeof(UnitDynFlagToName) / sizeof(UnitDynFlagNames);

static const char* GENDER[] =
{
    "male",
    "female",
    "neutral"
};

static const char* CLASS[] =
{
    "invalid 0",
    "warrior",
    "paladin",
    "hunter",
    "rogue",
    "priest",
    "deathknight",
    "shaman",
    "mage",
    "warlock",
    "invalid 10",
    "druid"
};

static const char* SHEATSTATE[] =
{
    "none",
    "melee",
    "ranged"
};

struct UnitPvPFlagNames
{
    uint32 Flag;
    const char* Name;
};

static const UnitPvPFlagNames UnitPvPFlagToName[] =
{
    { U_FIELD_BYTES_FLAG_PVP, "U_FIELD_BYTES_FLAG_PVP" },
    { U_FIELD_BYTES_FLAG_FFA_PVP, "U_FIELD_BYTES_FLAG_FFA_PVP" },
    { U_FIELD_BYTES_FLAG_SANCTUARY, "U_FIELD_BYTES_FLAG_SANCTUARY" }
};

static const uint32 numpvpflags = sizeof(UnitPvPFlagToName) / sizeof(UnitPvPFlagNames);

struct PetFlagNames
{
    uint32 Flag;
    const char* Name;
};

static const PetFlagNames PetFlagToName[] =
{
    { UNIT_CAN_BE_RENAMED, "UNIT_CAN_BE_RENAMED" },
    { UNIT_CAN_BE_ABANDONED, "UNIT_CAN_BE_ABANDONED" }
};

static const uint32 numpetflags = sizeof(PetFlagToName) / sizeof(PetFlagNames);
