/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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

#ifndef _PLAYER_DEFINES_H
#define _PLAYER_DEFINES_H

#include "CommonTypes.hpp"

#include <ctime>

enum PlayerTeam : int
{
    TEAM_ALLIANCE = 0,
    TEAM_HORDE    = 1,
    MAX_PLAYER_TEAMS
};


const float playerNormalRunSpeed = 7.0f;
const float playerNormalSwimSpeed = 4.72222f;
const float playerNormalFlightSpeed = 7.0f;

#define PLAYER_HONORLESS_TARGET_SPELL 2479

#define DBC_PLAYER_LEVEL_CAP 80
#define PLAYER_ARENA_MIN_LEVEL 70

#define PLAYER_EXPLORED_ZONES_LENGTH 128
#ifdef ENABLE_ACHIEVEMENTS
#define ACHIEVEMENT_SEND_DELAY 1000 /// we have this delay of sending auras to other players so client will have time to create object first
#endif
#define LOGIN_CIENT_SEND_DELAY 1000 /// we have this delay of sending auras to other players so client will have time to create object first

enum Gender
{
    GENDER_MALE     = 0,
    GENDER_FEMALE   = 1,
    GENDER_NONE     = 2
};

enum Classes
{
    WARRIOR         = 1,
    PALADIN         = 2,
    HUNTER          = 3,
    ROGUE           = 4,
    PRIEST          = 5,
    DEATHKNIGHT     = 6,
    SHAMAN          = 7,
    MAGE            = 8,
    WARLOCK         = 9,
    DRUID           = 11,
    MAX_PLAYER_CLASSES
};

enum Races
{
    RACE_HUMAN      = 1,
    RACE_ORC        = 2,
    RACE_DWARF      = 3,
    RACE_NIGHTELF   = 4,
    RACE_UNDEAD     = 5,
    RACE_TAUREN     = 6,
    RACE_GNOME      = 7,
    RACE_TROLL      = 8,
    RACE_BLOODELF   = 10,
    RACE_DRAENEI    = 11
};

enum PlayerStatus
{
    NONE             = 0,
    TRANSFER_PENDING = 1
};

enum RankTitles
{
    PVPTITLE_NONE                           = 0,
    PVPTITLE_PRIVATE                        = 1,
    PVPTITLE_CORPORAL                       = 2,
    PVPTITLE_SERGEANT                       = 3,
    PVPTITLE_MASTER_SERGEANT                = 4,
    PVPTITLE_SERGEANT_MAJOR                 = 5,
    PVPTITLE_KNIGHT                         = 6,
    PVPTITLE_KNIGHT_LIEUTENANT              = 7,
    PVPTITLE_KNIGHT_CAPTAIN                 = 8,
    PVPTITLE_KNIGHT_CHAMPION                = 9,
    PVPTITLE_LIEUTENANT_COMMANDER           = 10,
    PVPTITLE_COMMANDER                      = 11,
    PVPTITLE_MARSHAL                        = 12,
    PVPTITLE_FIELD_MARSHAL                  = 13,
    PVPTITLE_GRAND_MARSHAL                  = 14,
    PVPTITLE_SCOUT                          = 15,
    PVPTITLE_GRUNT                          = 16,
    PVPTITLE_HSERGEANT                      = 17,
    PVPTITLE_SENIOR_SERGEANT                = 18,
    PVPTITLE_FIRST_SERGEANT                 = 19,
    PVPTITLE_STONE_GUARD                    = 20,
    PVPTITLE_BLOOD_GUARD                    = 21,
    PVPTITLE_LEGIONNAIRE                    = 22,
    PVPTITLE_CENTURION                      = 23,
    PVPTITLE_CHAMPION                       = 24,
    PVPTITLE_LIEUTENANT_GENERAL             = 25,
    PVPTITLE_GENERAL                        = 26,
    PVPTITLE_WARLORD                        = 27,
    PVPTITLE_HIGH_WARLORD                   = 28,
    PVPTITLE_GLADIATOR                      = 29,
    PVPTITLE_DUELIST                        = 30,
    PVPTITLE_RIVAL                          = 31,
    PVPTITLE_CHALLENGER                     = 32,
    PVPTITLE_SCARAB_LORD                    = 33,
    PVPTITLE_CONQUEROR                      = 34,
    PVPTITLE_JUSTICAR                       = 35,
    PVPTITLE_CHAMPION_OF_THE_NAARU          = 36,
    PVPTITLE_MERCILESS_GLADIATOR            = 37,
    PVPTITLE_OF_THE_SHATTERED_SUN           = 38,
    PVPTITLE_HAND_OF_ADAL                   = 39,
    PVPTITLE_VENGEFUL_GLADIATOR             = 40,
    PVPTITLE_BATTLEMASTER                   = 41,
    PVPTITLE_THE_SEEKER                     = 42,
    PVPTITLE_ELDER                          = 43,
    PVPTITLE_FLAME_WARDEN                   = 44,
    PVPTITLE_FLAME_KEEPER                   = 45,
    PVPTITLE_THE_EXALTED                    = 46,
    PVPTITLE_THE_EXPLORER                   = 47,
    PVPTITLE_THE_DIPLOMAT                   = 48,
    PVPTITLE_BRUTAL_GLADIATOR               = 49,
    PVPTITLE_ARENA_MASTER                   = 50,
    PVPTITLE_SALTY                          = 51,
    PVPTITLE_CHEF                           = 52,
    PVPTITLE_THE_SUPREME                    = 53,
    PVPTITLE_OF_THE_TEN_STORMS              = 54,
    PVPTITLE_OF_THE_EMERALD_DREAM           = 55,
    PVPTITLE_DEADLY_GLADIATOR               = 56,
    PVPTITLE_PROPHET                        = 57,
    PVPTITLE_THE_MALEFIC                    = 58,
    PVPTITLE_STALKER                        = 59,
    PVPTITLE_OF_THE_EBON_BLADE              = 60,
    PVPTITLE_ARCHMAGE                       = 61,
    PVPTITLE_WARBRINGER                     = 62,
    PVPTITLE_ASSASSIN                       = 63,
    PVPTITLE_GRAND_MASTER_ALCHEMIST         = 64,
    PVPTITLE_GRAND_MASTER_BLACKSMITH        = 65,
    PVPTITLE_IRON_CHEF                      = 66,
    PVPTITLE_GRAND_MASTER_ENCHANTER         = 67,
    PVPTITLE_GRAND_MASTER_ENGINEER          = 68,
    PVPTITLE_DOCTOR                         = 69,
    PVPTITLE_GRAND_MASTER_ANGLER            = 70,
    PVPTITLE_GRAND_MASTER_HERBALIST         = 71,
    PVPTITLE_GRAND_MASTER_SCRIBE            = 72,
    PVPTITLE_GRAND_MASTER_JEWLCRAFTER       = 73,
    PVPTITLE_GRAND_MASTER_LETHERWORKER      = 74,
    PVPTITLE_GRAND_MASTER_MINER             = 75,
    PVPTITLE_GRAND_MASTER_SKINNER           = 76,
    PVPTITLE_GRAND_MASTER_TAILOR            = 77,
    PVPTITLE_OF_QUEL_THALAS                 = 78,
    PVPTITLE_OF_ARGUS                       = 79,
    PVPTITLE_OF_KHAZ_MODAN                  = 80,
    PVPTITLE_OF_GNOMEREGAN                  = 81,
    PVPTITLE_THE_LION_HEARTHED              = 82,
    PVPTITLE_CHAMPION_OF_ELUNE              = 83,
    PVPTITLE_HERO_OF_ORGIMMAR               = 84,
    PVPTITLE_PLAINSRUNNER                   = 85,
    PVPTITLE_OF_THE_DARKSPEARTRIPE          = 86,
    PVPTITLE_THE_FORSAKEN                   = 87,
    PVPTITLE_THE_MAGIC_SEEKER               = 88,
    PVPTITLE_TWILIGHT_VANQUISHER            = 89,
    PVPTITLE_CONQUEROR_OF_NAXXRAMAS         = 90,
    PVPTITLE_HERO_OF_NORTHREND              = 91,
    PVPTITLE_THE_HALLOWED                   = 92,
    PVPTITLE_LOREMASTER                     = 93,
    PVPTITLE_OF_THE_ALLIANCE                = 94,
    PVPTITLE_OF_THE_HORDE                   = 95,
    PVPTITLE_THE_FLAWLESS_VICTOR            = 96,
    PVPTITLE_CHAMPION_OF_THE_FROZEN_WASTES  = 97,
    PVPTITLE_AMBASSADOR                     = 98,
    PVPTITLE_THE_ARGENT_CHAMPION            = 99,
    PVPTITLE_GUARDIAN_OF_CENARIUS           = 100,
    PVPTITLE_BREWMASTER                     = 101,
    PVPTITLE_MERRYMAKER                     = 102,
    PVPTITLE_THE_LOVE_FOOL                  = 103,
    PVPTITLE_MATRON                         = 104,
    PVPTITLE_PATRON                         = 105,
    PVPTITLE_OBSIDIAN_SLAYER                = 106,
    PVPTITLE_OF_THE_NIGHTFALL               = 107,
    PVPTITLE_THE_IMMORTAL                   = 108,
    PVPTITLE_THE_UNDYING                    = 109,
    PVPTITLE_JENKINS                        = 110,
    PVPTITLE_BLOODSAIL_ADMIRAL              = 111,
    PVPTITLE_THE_INSANE                     = 112,
    PVPTITLE_OF_THE_EXODAR                  = 113,
    PVPTITLE_OF_DARNASSUS                   = 114,
    PVPTITLE_OF_IRONFORGE                   = 115,
    PVPTITLE_OF_STORMWIND                   = 116,
    PVPTITLE_OF_ORGRIMMAR                   = 117,
    PVPTITLE_OF_SENJIN                      = 118,
    PVPTITLE_OF_SILVERMOON                  = 119,
    PVPTITLE_OF_TUNDERBLUFF                 = 120,
    PVPTITLE_OF_THE_UNDERCITY               = 121,
    PVPTITLE_THE_NOBLE                      = 122,
    PVPTITLE_CRUSADER                       = 123,
    PVPTITLE_DEATHS_DEMISE                  = 124,
    PVPTITLE_CELESTIAL_DEFENDER             = 125,
    PVPTITLE_CONQUEROR_OF_ULDUAR            = 126,
    PVPTITLE_CHAMPION_OF_ULDUAR             = 127,
    PVPTITLE_VANQUISHER                     = 128,
    PVPTITLE_STARCALLER                     = 129,
    PVPTITLE_THE_ASTRAL_WALKER              = 130,
    PVPTITLE_HERALD_OF_THE_TITANS           = 131,
    PVPTITLE_FURIOUS_GLADIATOR              = 132,
    PVPTITLE_THE_PILGRIM                    = 133,
    PVPTITLE_RELENTLESS_GLADIATOR           = 134,
    PVPTITLE_GRAND_CRUSADER                 = 135,
    PVPTITLE_THE_ARGENT_DEFENDER            = 136,
    PVPTITLE_THE_PATIENT                    = 137,
    PVPTITLE_THE_LIGHT_OF_THE_DAWN          = 138,
    PVPTITLE_BANE_OF_THE_FALLEN_KING        = 139,
    PVPTITLE_THE_KINGSLAYER                 = 140,
    PVPTITLE_OF_THE_ASHEN_VERDICT           = 141,
    PVPTITLE_WRATHFUL_GLADIATOR             = 142,
    PVPTITLE_END                            = 143
};

enum PvPAreaStatus
{
    AREA_ALLIANCE       = 1,
    AREA_HORDE          = 2,
    AREA_CONTESTED      = 3,
    AREA_PVPARENA       = 4
};

enum PlayerMovementType
{
    MOVE_ROOT           = 1,
    MOVE_UNROOT         = 2,
    MOVE_WATER_WALK     = 3,
    MOVE_LAND_WALK      = 4,
};

enum PlayerSpeedType
{
    RUN                 = 1,
    RUNBACK             = 2,
    SWIM                = 3,
    SWIMBACK            = 4,
    WALK                = 5,
    FLY                 = 6,
};

/*
Exalted             1,000     Access to racial mounts. Capped at 999.7
Revered             21,000     Heroic mode keys for Outland dungeons
Honored             12,000     10% discount from faction vendors
Friendly            6,000
Neutral             3,000
Unfriendly          3,000     Cannot buy, sell or interact.
Hostile             3,000     You will always be attacked on sight
Hated               36,000
*/
enum Standing
{
    STANDING_HATED,
    STANDING_HOSTILE,
    STANDING_UNFRIENDLY,
    STANDING_NEUTRAL,
    STANDING_FRIENDLY,
    STANDING_HONORED,
    STANDING_REVERED,
    STANDING_EXALTED
};

enum PlayerFlags
{
    PLAYER_FLAG_NONE                = 0x00000000,
    PLAYER_FLAG_PARTY_LEADER        = 0x00000001,
    PLAYER_FLAG_AFK                 = 0x00000002,
    PLAYER_FLAG_DND                 = 0x00000004,
    PLAYER_FLAG_GM                  = 0x00000008,
    PLAYER_FLAG_DEATH_WORLD_ENABLE  = 0x00000010,
    PLAYER_FLAG_RESTING             = 0x00000020,
    PLAYER_FLAG_ADMIN               = 0x00000040,
    PLAYER_FLAG_FREE_FOR_ALL_PVP    = 0x00000080,
    PLAYER_FLAG_CONT_PVP            = 0x00000100,
    PLAYER_FLAG_PVP_TOGGLE          = 0x00000200,
    PLAYER_FLAG_NOHELM              = 0x00000400,
    PLAYER_FLAG_NOCLOAK             = 0x00000800,
    PLAYER_FLAG_NEED_REST_3_HOURS   = 0x00001000,
    PLAYER_FLAG_IS_DEAD             = 0x00002000,
    PLAYER_FLAGS_RENAME_FIRST       = 0x00004000,
    PLAYER_FLAG_DEVELOPER           = 0x00008000,
    PLAYER_FLAG_UNK1                = 0x00010000,
    PLAYER_FLAG_UNK2                = 0x00020000,
    PLAYER_FLAG_PVP                 = 0x00040000,
    PLAYER_FLAG_UNK3                = 0x00080000,
    PLAYER_FLAG_UNK4                = 0x00100000,
    PLAYER_FLAG_UNK5                = 0x00200000,
    PLAYER_FLAG_UNK6                = 0x00400000,
    PLAYER_FLAG_UNK7                = 0x00800000,
    PLAYER_FLAG_IS_BANNED           = 0x01000000,
    PLAYER_FLAG_UNK8                = 0x02000000,
    PLAYER_FLAG_UNK9                = 0x04000000,
    PLAYER_FLAG_UNK10               = 0x08000000,
    PLAYER_FLAG_UNK11               = 0x10000000,
    PLAYER_FLAG_UNK12               = 0x20000000,
    PLAYER_FLAG_UNK13               = 0x40000000,
    PLAYER_FLAG_UNK14               = 0x80000000
};

enum CustomizeFlags
{
    CHAR_CUSTOMIZE_FLAG_NONE        = 0x00000000,   // Implemented          * Allows normal login no customization needed
    CHAR_CUSTOMIZE_FLAG_CUSTOMIZE   = 0x00000001,   // Implemented          * Allows name, gender, and looks to be customized
    CHAR_CUSTOMIZE_FLAG_FACTION     = 0x00010000,   ///\todo Implement      * Allows name, gender, race, faction, and looks to be customized
    CHAR_CUSTOMIZE_FLAG_RACE        = 0x00100000    ///\todo Implement      * Allows name, gender, race, and looks to be customized
};

enum LoginFlags
{
    LOGIN_NO_FLAG               = 0,
    LOGIN_FORCED_RENAME         = 1,
    LOGIN_CUSTOMIZE_FACTION     = 2,
    LOGIN_CUSTOMIZE_RACE        = 4,
    LOGIN_CUSTOMIZE_LOOKS       = 8
};

enum FriendsResult
{
    FRIEND_DB_ERROR             = 0x00,
    FRIEND_LIST_FULL            = 0x01,
    FRIEND_ONLINE               = 0x02,
    FRIEND_OFFLINE              = 0x03,
    FRIEND_NOT_FOUND            = 0x04,
    FRIEND_REMOVED              = 0x05,
    FRIEND_ADDED_ONLINE         = 0x06,
    FRIEND_ADDED_OFFLINE        = 0x07,
    FRIEND_ALREADY              = 0x08,
    FRIEND_SELF                 = 0x09,
    FRIEND_ENEMY                = 0x0A,
    FRIEND_IGNORE_FULL          = 0x0B,
    FRIEND_IGNORE_SELF          = 0x0C,
    FRIEND_IGNORE_NOT_FOUND     = 0x0D,
    FRIEND_IGNORE_ALREADY       = 0x0E,
    FRIEND_IGNORE_ADDED         = 0x0F,
    FRIEND_IGNORE_REMOVED       = 0x10
};

enum CharterTypes
{
    CHARTER_TYPE_GUILD            = 0,
    CHARTER_TYPE_ARENA_2V2        = 1,
    CHARTER_TYPE_ARENA_3V3        = 2,
    CHARTER_TYPE_ARENA_5V5        = 3,
    NUM_CHARTER_TYPES             = 4
};

enum ArenaTeamTypes
{
    ARENA_TEAM_TYPE_2V2            = 0,
    ARENA_TEAM_TYPE_3V3            = 1,
    ARENA_TEAM_TYPE_5V5            = 2,
    NUM_ARENA_TEAM_TYPES           = 3
};

enum CooldownTypes
{
    COOLDOWN_TYPE_SPELL            = 0,
    COOLDOWN_TYPE_CATEGORY         = 1,
    NUM_COOLDOWN_TYPES
};

///\todo are the values really ignored by client?
enum LootType
{
    LOOT_CORPSE                 = 1,
    LOOT_SKINNING               = 2,        // 6
    LOOT_FISHING                = 3,
    LOOT_PICKPOCKETING          = 2,        // 2
    LOOT_DISENCHANTING          = 2,        // 4    // ignored
    LOOT_PROSPECTING            = 2,        // 7
    LOOT_MILLING                = 2,        // 8    
    LOOT_INSIGNIA               = 2         // 21 unsupported by client, sending LOOT_SKINNING instead
};

enum ModType
{
    MOD_MELEE     = 0,
    MOD_RANGED    = 1,
    MOD_SPELL     = 2
};

struct spells
{
    uint16 spellId;
    uint16 slotId;
};

enum DrunkenState
{
    DRUNKEN_SOBER    = 0,
    DRUNKEN_TIPSY    = 1,
    DRUNKEN_DRUNK    = 2,
    DRUNKEN_SMASHED  = 3
};

/**
    TalentTree table

    mage - arcane - 81
    mage - fire - 41
    mage - frost - 61

    rogue - assassination - 182
    rogue - combat - 181
    rogue - subelty - 183

    warlock - affliction - 302
    warlock - demonology - 303
    warlock - destruction - 301

    warrior - arms - 161
    warrior - fury - 163
    warrior - protection - 164

    shaman - elemental - 261
    shaman - enchantment - 263
    shaman - restoration - 262

    paladin - holy - 382
    paladin - protection - 383
    paladin - retribution - 381

    death knight - blood - 398
    death knight - frost - 399
    death knight - unholy - 400

    priest - discipline - 201
    priest - holy - 202
    priest - shadow - 203

    hunter - beast - 361
    hunter - marksmanship - 363
    hunter - survival - 362

    druid - balance - 283
    druid - feral combat - 281
    druid - restoration - 282
*/

static const uint32 TalentTreesPerClass[DRUID + 1][3] =
{
        { 0, 0, 0 },        // NONE
        { 161, 163, 164 },  // WARRIOR
        { 382, 383, 381 },  // PALADIN
        { 361, 363, 362 },  // HUNTER
        { 182, 181, 183 },  // ROGUE
        { 201, 202, 203 },  // PRIEST
        { 398, 399, 400 },  // DEATH KNIGHT
        { 261, 263, 262 },  // SHAMAN
        { 81, 41, 61 },     // MAGE
        { 302, 303, 301 },  // WARLOCK
        { 0, 0, 0 },        // NONE
        { 283, 281, 282 },  // DRUID
};


enum RestState
{
    RESTSTATE_RESTED        = 1,
    RESTSTATE_NORMAL        = 2,
    RESTSTATE_TIRED100      = 3,
    RESTSTATE_TIRED50       = 4,
    RESTSTATE_EXHAUSTED     = 5
};

enum UnderwaterState
{
    UNDERWATERSTATE_NONE            = 0,
    UNDERWATERSTATE_SWIMMING        = 1,
    UNDERWATERSTATE_UNDERWATER      = 2,
    UNDERWATERSTATE_RECOVERING      = 4,
    UNDERWATERSTATE_TAKINGDAMAGE    = 8,
    UNDERWATERSTATE_FATIGUE         = 16,
    UNDERWATERSTATE_LAVA            = 32,
    UNDERWATERSTATE_SLIME           = 64
};

enum TradeStatus
{
    TRADE_STATUS_PLAYER_BUSY        = 0x00,
    TRADE_STATUS_PROPOSED           = 0x01,
    TRADE_STATUS_INITIATED          = 0x02,
    TRADE_STATUS_CANCELLED          = 0x03,
    TRADE_STATUS_ACCEPTED           = 0x04,
    TRADE_STATUS_ALREADY_TRADING    = 0x05,
    TRADE_STATUS_PLAYER_NOT_FOUND   = 0x06,
    TRADE_STATUS_STATE_CHANGED      = 0x07,
    TRADE_STATUS_COMPLETE           = 0x08,
    TRADE_STATUS_UNACCEPTED         = 0x09,
    TRADE_STATUS_TOO_FAR_AWAY       = 0x0A,
    TRADE_STATUS_WRONG_FACTION      = 0x0B,
    TRADE_STATUS_FAILED             = 0x0C,
    TRADE_STATUS_DEAD               = 0x0D,
    TRADE_STATUS_PETITION           = 0x0E,
    TRADE_STATUS_PLAYER_IGNORED     = 0x0F
};

enum TradeData
{
    TRADE_GIVE        = 0x00,
    TRADE_RECEIVE     = 0x01
};

enum DuelStatus
{
    DUEL_STATUS_OUTOFBOUNDS,
    DUEL_STATUS_INBOUNDS
};

enum DuelState
{
    DUEL_STATE_REQUESTED,
    DUEL_STATE_STARTED,
    DUEL_STATE_FINISHED
};

enum DuelWinner
{
    DUEL_WINNER_KNOCKOUT,
    DUEL_WINNER_RETREAT
};

const time_t attackTimeoutInterval = 5000;
const time_t forcedResurrectInterval = 360000;  // 1000*60*6= 6 minutes

enum PlayerCombatRating
{
    PCR_RANGED_SKILL                = 0,
    PCR_DEFENCE                     = 1,
    PCR_DODGE                       = 2,
    PCR_PARRY                       = 3,
    PCR_BLOCK                       = 4,
    PCR_MELEE_HIT                   = 5,
    PCR_RANGED_HIT                  = 6,
    PCR_SPELL_HIT                   = 7,
    PCR_MELEE_CRIT                  = 8,
    PCR_RANGED_CRIT                 = 9,
    PCR_SPELL_CRIT                  = 10,
    PCR_MELEE_HIT_AVOIDANCE         = 11,   // Not 100% sure but the numbers line up
    PCR_RANGED_HIT_AVOIDANCE        = 12,   // GUESSED
    PCR_SPELL_HIT_AVOIDANCE         = 13,   // GUESSED
    PCR_MELEE_CRIT_RESILIENCE       = 14,
    PCR_RANGED_CRIT_RESILIENCE      = 15,
    PCR_SPELL_CRIT_RESILIENCE       = 16,
    PCR_MELEE_HASTE                 = 17,
    PCR_RANGED_HASTE                = 18,
    PCR_SPELL_HASTE                 = 19,
    PCR_MELEE_MAIN_HAND_SKILL       = 20,
    PCR_MELEE_OFF_HAND_SKILL        = 21,
    PCR_MELEE_RANGED_SKILL          = 22,   // Not used
    PCR_EXPERTISE                   = 23,
    PCR_ARMOR_PENETRATION_RATING    = 24,
    MAX_PCR                         = 25
};

enum MirrorTimerTypes
{
    MIRROR_TYPE_FATIGUE     = 0,
    MIRROR_TYPE_BREATH      = 1,
    MIRROR_TYPE_FIRE        = 2
};

enum PlayerCheats
{
    PLAYER_CHEAT_NONE           = 0x000,
    PLAYER_CHEAT_COOLDOWN       = 0x001,
    PLAYER_CHEAT_CAST_TIME      = 0x002,
    PLAYER_CHEAT_GOD_MODE       = 0x004,
    PLAYER_CHEAT_POWER          = 0x008,
    PLAYER_CHEAT_FLY            = 0x010,
    PLAYER_CHEAT_AURA_STACK     = 0x020,
    PLAYER_CHEAT_ITEM_STACK     = 0x040,
    PLAYER_CHEAT_TRIGGERPASS    = 0x080,
    PLAYER_CHEAT_TAXI           = 0x100,
};

// action button defines
#define PLAYER_ACTION_BUTTON_COUNT 144
#define PLAYER_ACTION_BUTTON_SIZE PLAYER_ACTION_BUTTON_COUNT * sizeof(ActionButton)

#define MAX_SPEC_COUNT 2
#define GLYPHS_COUNT 6

#endif // _PLAYER_DEFINES_H
