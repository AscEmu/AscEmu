/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#include <ctime>
#include <string>

enum PlayerTeam : uint8_t
{
    TEAM_ALLIANCE = 0,
    TEAM_HORDE    = 1,
    MAX_PLAYER_TEAMS
};

#define PLAYER_HONORLESS_TARGET_SPELL 2479

//////////////////////////////////////////////////////////////////////////////////////////
// DBC_PLAYER_LEVEL_CAP
//
// \param level cap
//
// Vanilla = 60
// The Burning Crusade = 70
// Wrath of the Lich King = 80
// Cataclysm = 85
// Mists of Pandaria = 90
// Warlords of Draenor = 100
// Legion = 110
//
//////////////////////////////////////////////////////////////////////////////////////////

#if VERSION_STRING == Classic
    #define DBC_PLAYER_LEVEL_CAP 60
#elif VERSION_STRING == TBC
    #define DBC_PLAYER_LEVEL_CAP 70
#elif VERSION_STRING == WotLK
    #define DBC_PLAYER_LEVEL_CAP 80
#elif VERSION_STRING == Cata
    #define DBC_PLAYER_LEVEL_CAP 85
#elif VERSION_STRING == Mop
    #define DBC_PLAYER_LEVEL_CAP 85
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// DBC_PLAYER_SKILL_MAX
//
// \param skill max
//
// Vanilla = 300
// The Burning Crusade = 375
// Wrath of the Lich King = 450
// Cataclysm = 525
// Mists of Pandaria = 600
// Warlords of Draenor = 700
// Legion = 800
//
//////////////////////////////////////////////////////////////////////////////////////////

#if VERSION_STRING == Classic
    #define DBC_PLAYER_SKILL_MAX 300
#elif VERSION_STRING == TBC
    #define DBC_PLAYER_SKILL_MAX 375
#elif VERSION_STRING == WotLK
    #define DBC_PLAYER_SKILL_MAX 450
#elif VERSION_STRING == Cata
    #define DBC_PLAYER_SKILL_MAX 525
#elif VERSION_STRING == Mop
    #define DBC_PLAYER_SKILL_MAX 525
#endif


//////////////////////////////////////////////////////////////////////////////////////////
// Minimum level required arena
#define PLAYER_ARENA_MIN_LEVEL 70

#if VERSION_STRING > Classic
#define PLAYER_EXPLORED_ZONES_LENGTH 128
#else
#define PLAYER_EXPLORED_ZONES_LENGTH 64
#endif

#define ACHIEVEMENT_SEND_DELAY 1000 // we have this delay of sending auras to other players so client will have time to create object first
#define LOGIN_CIENT_SEND_DELAY 1000 // we have this delay of sending auras to other players so client will have time to create object first

enum Gender
{
    GENDER_MALE     = 0,
    GENDER_FEMALE   = 1,
    GENDER_NONE     = 2
};

//\note defined for all versions!
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
#if VERSION_STRING >= Cata
    RACE_GOBLIN     = 9,
#endif
#if VERSION_STRING > Classic
    RACE_BLOODELF   = 10,
    RACE_DRAENEI    = 11,
#endif
#if VERSION_STRING < Cata
    NUM_RACES
#else
    RACE_WORGEN     = 22,
    NUM_RACES
#endif
};

enum PlayerStatus
{
    NONE             = 0,
    TRANSFER_PENDING = 1
};

enum RankTitles : uint16_t
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
    PLAYER_FLAG_NONE                    = 0x00000000,
    PLAYER_FLAG_PARTY_LEADER            = 0x00000001, // (TODO: implement for all versions) Informs players outside of your group who is your group leader
    PLAYER_FLAG_AFK                     = 0x00000002, // <AFK> or <Away> tag ingame
    PLAYER_FLAG_DND                     = 0x00000004, // <DND> or <Busy> tag ingame
    PLAYER_FLAG_GM                      = 0x00000008, // <GM> tag ingame
    PLAYER_FLAG_DEATH_WORLD_ENABLE      = 0x00000010, // Adds death glow to the world
    PLAYER_FLAG_RESTING                 = 0x00000020, // Applies rested state on your character portrait
    PLAYER_FLAG_ADMIN                   = 0x00000040, // Unknown effect in 3.3.5a
    PLAYER_FLAG_FREE_FOR_ALL_PVP        = 0x00000080, // Unknown in 3.3.5a, pre-wotlk FFA-pvp tag
    PLAYER_FLAG_PVP_GUARD_ATTACKABLE    = 0x00000100, // Player will be attacked by neutral guards
    PLAYER_FLAG_PVP_TOGGLE              = 0x00000200, // Toggles PvP combat on/off
    PLAYER_FLAG_NOHELM                  = 0x00000400, // Hides helm
    PLAYER_FLAG_NOCLOAK                 = 0x00000800, // Hides cloak
    PLAYER_FLAG_PLAYED_3_HOURS          = 0x00001000, // Obsolete: "You have more than 3 hours of online time. You will receive 1/2 money and XP during this period."
    PLAYER_FLAG_PLAYED_5_HOURS          = 0x00002000, // Obsolete: "You have more than 5 hours of online time. You will not be able to gain loot, XP, or complete quests."
    PLAYER_FLAG_UNK1                    = 0x00004000,
    // TBC flags begin (needs verification)
    PLAYER_FLAG_DEVELOPER               = 0x00008000, // <Dev> tag ingame
    PLAYER_FLAG_SANCTUARY               = 0x00010000, // (TODO: not implemented?) Makes player unattackable, added in sanctuary areas
    PLAYER_FLAG_UNK2                    = 0x00020000, // Toggles 'Taxi Time Test' and FPS counter, unused
    // WoTLK flags begin
    PLAYER_FLAG_PVP_TIMER               = 0x00040000, // PvP timer after toggling manually PvP combat state off
    PLAYER_FLAG_UNK3                    = 0x00080000,
    PLAYER_FLAG_UNK4                    = 0x00100000,
    PLAYER_FLAG_UNK5                    = 0x00200000,
    PLAYER_FLAG_UNK6                    = 0x00400000,
    PLAYER_FLAG_PREVENT_SPELL_CAST      = 0x00800000, // Prevents spell casting but excludes auto attack, used by Bladestorm for example
    PLAYER_FLAG_PREVENT_MELEE_SPELLS    = 0x01000000, // Prevents melee spell casting and includes auto attack, unused?
    PLAYER_FLAG_NO_XP                   = 0x02000000, // (TODO: implement this and remove variable from player class) Disables XP gain and hides XP bar
    // Cataclysm flags begin (needs verification)
    PLAYER_FLAG_UNK7                    = 0x04000000,
    PLAYER_FLAGS_AUTO_DECLINE_GUILD     = 0x08000000,
    PLAYER_FLAGS_GUILD_LVL_ENABLED      = 0x10000000,
    PLAYER_FLAG_UNK8                    = 0x20000000,
    PLAYER_FLAG_UNK9                    = 0x40000000,
    PLAYER_FLAG_UNK10                   = 0x80000000
};

enum CustomizeFlags
{
    CHAR_CUSTOMIZE_FLAG_NONE        = 0x00000000,   // Implemented          * Allows normal login no customization needed
    CHAR_CUSTOMIZE_FLAG_CUSTOMIZE   = 0x00000001,   // Implemented          * Allows name, gender, and looks to be customized
    CHAR_CUSTOMIZE_FLAG_FACTION     = 0x00010000,   //\todo Implement      * Allows name, gender, race, faction, and looks to be customized
    CHAR_CUSTOMIZE_FLAG_RACE        = 0x00100000    //\todo Implement      * Allows name, gender, race, and looks to be customized
};

enum LoginFlags
{
    LOGIN_NO_FLAG               = 0,
    LOGIN_FORCED_RENAME         = 1,
    LOGIN_CUSTOMIZE_FACTION     = 2,
    LOGIN_CUSTOMIZE_RACE        = 4,
    LOGIN_CUSTOMIZE_LOOKS       = 8
};

enum CharacterScreenFlags
{
    CHARACTER_SCREEN_FLAG_NONE              = 0x00000000,
    CHARACTER_SCREEN_FLAG_LOCKED_UPDATE     = 0x00000004, // Prevents login: "You cannot log in until the character update process you recently initiated is complete."
    CHARACTER_SCREEN_FLAG_HIDE_HELM         = 0x00000400, // Hides character's helm
    CHARACTER_SCREEN_FLAG_HIDE_CLOAK        = 0x00000800, // Hides character's cloak
    CHARACTER_SCREEN_FLAG_DEAD              = 0x00002000, // Shows character is dead
    CHARACTER_SCREEN_FLAG_FORCED_RENAME     = 0x00004000, // Prevents login: "Your name has been flagged for rename, please enter a new name:"
    CHARACTER_SCREEN_FLAG_BANNED            = 0x01000000  // Prevents login: "Character locked. Contact Billing for more information."
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

static const uint32 TalentTreesPerClass[MAX_PLAYER_CLASSES][3] =
{
#if VERSION_STRING < Cata
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
#else
    { 0, 0, 0 },        // NONE
    { 746, 815, 845 },  // WARRIOR      - arms - fury - protection -
    { 831, 839, 855 },  // PALADIN      - holy - protection - retribution -
    { 811, 807, 809 },  // HUNTER       - beast - marksmanship - survival -
    { 182, 181, 183 },  // ROGUE        - assassination - combat - subelty -
    { 760, 813, 795 },  // PRIEST       - discipline - holy - shadow -
    { 398, 399, 400 },  // DEATH KNIGHT - blood - frost - unholy -
    { 261, 263, 262 },  // SHAMAN       - elemental - enchantment - restoration -
    { 799, 851, 823 },  // MAGE         - arcane - fire - frost -
    { 871, 867, 865 },  // WARLOCK      - affliction - demonology - destruction -
    { 0, 0, 0 },        // NONE
    { 752, 750, 748 },  // DRUID        - balance - feral/combat - restoration -
#endif
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

#if VERSION_STRING >= Cata
enum TradeStatus
{
    TRADE_STATUS_OPEN_WINDOW            = 0,
    TRADE_STATUS_INITIATED              = 1,
    TRADE_STATUS_NOT_ON_TAPLIST         = 2,
    TRADE_STATUS_YOU_LOGOUT             = 3,
    TRADE_STATUS_IGNORE_YOU             = 4,
    TRADE_STATUS_TARGET_DEAD            = 5,
    TRADE_STATUS_TRADE_ACCEPT           = 6,
    TRADE_STATUS_TARGET_LOGOUT          = 7,
    TRADE_STATUS_UNK8                   = 8,
    TRADE_STATUS_TRADE_COMPLETE         = 9,
    TRADE_STATUS_UNK10                  = 10,
    TRADE_STATUS_UNK11                  = 11,
    TRADE_STATUS_BEGIN_TRADE            = 12,
    TRADE_STATUS_YOU_DEAD               = 13,
    TRADE_STATUS_UNK14                  = 14,
    TRADE_STATUS_UNK15                  = 15,
    TRADE_STATUS_TARGET_TO_FAR          = 16,
    TRADE_STATUS_NO_TARGET              = 17,
    TRADE_STATUS_UNK18                  = 18,
    TRADE_STATUS_CURRENCY_NOT_TRADEABLE = 19,
    TRADE_STATUS_WRONG_FACTION          = 20,
    TRADE_STATUS_BUSY                   = 21,
    TRADE_STATUS_UNK22                  = 22,
    TRADE_STATUS_TRADE_CANCELED         = 23,
    TRADE_STATUS_CURRENCY               = 24,
    TRADE_STATUS_BACK_TO_TRADE          = 25,
    TRADE_STATUS_ONLY_CONJURED          = 26,
    TRADE_STATUS_YOU_STUNNED            = 27,
    TRADE_STATUS_UNK28                  = 28,
    TRADE_STATUS_TARGET_STUNNED         = 29,
    TRADE_STATUS_UNK30                  = 30,
    TRADE_STATUS_CLOSE_WINDOW           = 31
};

enum TradeSlots
{
    TRADE_SLOT_COUNT            = 7,
    TRADE_SLOT_TRADED_COUNT     = 6,
    TRADE_SLOT_NONTRADED        = 6
};
#else
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
#endif

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

enum PlayerCombatRating : uint16_t
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

//char enum
struct PlayerItem
{
    uint32_t displayId;
    uint8_t inventoryType;
    uint32_t enchantmentId;
};

struct CharEnum_Pet
{
    uint32_t display_id;
    uint32_t level;
    uint32_t family;
};

struct CharEnumData
{
    uint64_t guid;
    uint8_t level;
    uint8_t race;
    uint8_t Class;
    uint8_t gender;
    uint32_t bytes;
    uint32_t bytes2;
    std::string name;
    float x;
    float y;
    float z;
    uint32_t mapId;
    uint32_t zoneId;
    uint32_t banned;

    uint32_t deathState;
    uint32_t loginFlags;
    uint32_t flags;
    uint32_t guildId;

    uint32_t char_flags;
    uint32_t customization_flag;

    CharEnum_Pet pet_data;

    //\todo verfify this!
#if VERSION_STRING <= TBC
    PlayerItem player_items[20];
#else
    PlayerItem player_items[23];
#endif
};


// table taken from https://wow.gamepedia.com/Class
static const uint32_t ClassRaceCombinations[91][3] =
{
    //WARRIOR
    {1, 1, 4044},
    {1, 2, 4044},
    {1, 3, 4044},
    {1, 4, 4044},
    {1, 5, 4044},
    {1, 6, 4044},
    {1, 7, 4044},
    {1, 8, 4044},
    {1, 9, 13164},
    {1, 10, 13164},
    {1, 11, 6080},
    {1, 22, 13164},
    //PALADIN
    {2, 1, 4044},
    {2, 3, 4044},
    {2, 6, 13164},
    {2, 10, 6080},
    {2, 11, 6080},
    //HUNTER
    {3, 1, 13164},
    {3, 2, 4044},
    {3, 3, 4044},
    {3, 4, 4044},
    {3, 5, 13164},
    {3, 6, 4044},
    {3, 8, 4044},
    {3, 9, 13164},
    {3, 10, 6080},
    {3, 11, 6080},
    {3, 22, 13164},
    //ROGUE
    {4, 1, 4044},
    {4, 2, 4044},
    {4, 3, 4044},
    {4, 4, 4044},
    {4, 5, 4044},
    {4, 7, 4044},
    {4, 8, 4044},
    {4, 9, 13164},
    {4, 10, 6080},
    {4, 22, 13164},
    //PRIEST
    {5, 1, 4044},
    {5, 3, 4044},
    {5, 4, 4044},
    {5, 5, 4044},
    {5, 6, 13164},
    {5, 7, 13164},
    {5, 8, 4044},
    {5, 9, 13164},
    {5, 10, 6080},
    {5, 11, 6080},
    {5, 22, 13164},
    //DEATHKNIGHT
    {6, 1, 9056},
    {6, 2, 9056},
    {6, 3, 9056},
    {6, 4, 9056},
    {6, 5, 9056},
    {6, 6, 9056},
    {6, 7, 9056},
    {6, 8, 9056},
    {6, 9, 13164},
    {6, 10, 9056},
    {6, 11, 9056},
    {6, 22, 13164},
    //SHAMAN
    {7, 2, 4044},
    {7, 3, 13164},
    {7, 6, 4044},
    {7, 8, 4044},
    {7, 9, 13164},
    {7, 11, 6080},
    //MAGE
    {8, 1, 4044},
    {8, 2, 13164},
    {8, 3, 13164},
    {8, 4, 13164},
    {8, 5, 4044},
    {8, 7, 4044},
    {8, 8, 4044},
    {8, 9, 13164},
    {8, 10, 6080},
    {8, 11, 6080},
    {8, 22, 13164},
    //WARLOCK
    {9, 1, 4044},
    {9, 2, 4044},
    {9, 3, 13164},
    {9, 5, 4044},
    {9, 7, 4044},
    {9, 8, 13164},
    {9, 9, 13164},
    {9, 10, 6080},
    {9, 22, 13164},
    //DRUID
    {11, 4, 4044},
    {11, 6, 4044},
    {11, 8, 13164},
    {11, 22, 13164},
};

inline uint32_t getAEVersion()
{
#if VERSION_STRING == Classic
    return 5875;
#elif VERSION_STRING == TBC
    return 8606;
#elif VERSION_STRING == WotLK
    return 12340;
#else
    return 15595;
#endif
}

inline bool isClassRaceCombinationPossible(uint8_t _class, uint8_t _race)
{
    for (uint8_t i = 0; i < 91; ++i)
    {
        if (ClassRaceCombinations[i][0] == _class && ClassRaceCombinations[i][1] == _race)
        {
            if (ClassRaceCombinations[i][2] < getAEVersion())
                return true;
        }
    }

    return false;
}

// table from http://www.wowwiki.com/Mana_regeneration
static float BaseManaRegenByAEVersion[165][3] =
{
    //\ brief wrong for classic. Mana regen was calculated based on spirit etc.
    //       redo this for classic someday.

    // Build: 8089
    { 1, 0.034965f, 5875 },
    { 2, 0.034191f, 5875 },
    { 3, 0.033465f, 5875 },
    { 4, 0.032526f, 5875 },
    { 5, 0.031661f, 5875 },
    { 6, 0.031076f, 5875 },
    { 7, 0.030523f, 5875 },
    { 8, 0.029994f, 5875 },
    { 9, 0.029307f, 5875 },
    { 10, 0.028661f, 5875 },
    { 11, 0.027584f, 5875 },
    { 12, 0.026215f, 5875 },
    { 13, 0.025381f, 5875 },
    { 14, 0.024300f, 5875 },
    { 15, 0.023345f, 5875 },
    { 16, 0.022748f, 5875 },
    { 17, 0.021958f, 5875 },
    { 18, 0.021386f, 5875 },
    { 19, 0.020790f, 5875 },
    { 20, 0.020121f, 5875 },
    { 21, 0.019733f, 5875 },
    { 22, 0.019155f, 5875 },
    { 23, 0.018819f, 5875 },
    { 24, 0.018316f, 5875 },
    { 25, 0.017936f, 5875 },
    { 26, 0.017576f, 5875 },
    { 27, 0.017201f, 5875 },
    { 28, 0.016919f, 5875 },
    { 29, 0.016581f, 5875 },
    { 30, 0.016233f, 5875 },
    { 31, 0.015994f, 5875 },
    { 32, 0.015707f, 5875 },
    { 33, 0.015464f, 5875 },
    { 34, 0.015204f, 5875 },
    { 35, 0.014956f, 5875 },
    { 36, 0.014744f, 5875 },
    { 37, 0.014495f, 5875 },
    { 38, 0.014302f, 5875 },
    { 39, 0.014094f, 5875 },
    { 40, 0.013895f, 5875 },
    { 41, 0.013724f, 5875 },
    { 42, 0.013522f, 5875 },
    { 43, 0.013363f, 5875 },
    { 44, 0.013175f, 5875 },
    { 45, 0.012996f, 5875 },
    { 46, 0.012853f, 5875 },
    { 47, 0.012687f, 5875 },
    { 48, 0.012539f, 5875 },
    { 49, 0.012384f, 5875 },
    { 50, 0.012233f, 5875 },
    { 51, 0.012113f, 5875 },
    { 52, 0.011973f, 5875 },
    { 53, 0.011859f, 5875 },
    { 54, 0.011714f, 5875 },
    { 55, 0.011575f, 5875 },
    { 56, 0.011473f, 5875 },
    { 57, 0.011342f, 5875 },
    { 58, 0.011245f, 5875 },
    { 59, 0.011110f, 5875 },
    { 60, 0.010999f, 5875 },

    // Build: 8089
    { 61, 0.010700f, 8478 },
    { 62, 0.010522f, 8478 },
    { 63, 0.010290f, 8478 },
    { 64, 0.010119f, 8478 },
    { 65, 0.009968f, 8478 },
    { 66, 0.009808f, 8478 },
    { 67, 0.009651f, 8478 },
    { 68, 0.009553f, 8478 },
    { 69, 0.009445f, 8478 },
    { 70, 0.009327f, 8478 },

    //Build: 9056
    { 71, 0.008859f, 12340 },
    { 72, 0.008415f, 12340 },
    { 73, 0.007993f, 12340 },
    { 74, 0.007592f, 12340 },
    { 75, 0.007211f, 12340 },
    { 76, 0.006849f, 12340 },
    { 77, 0.006506f, 12340 },
    { 78, 0.006179f, 12340 },
    { 79, 0.005869f, 12340 },
    { 80, 0.005575f, 12340 },

    // Build: 13164
    { 1, 0.020979f, 15595 },
    { 2, 0.020515f, 15595 },
    { 3, 0.020079f, 15595 },
    { 4, 0.019516f, 15595 },
    { 5, 0.018997f, 15595 },
    { 6, 0.018646f, 15595 },
    { 7, 0.018314f, 15595 },
    { 8, 0.017997f, 15595 },
    { 9, 0.017584f, 15595 },
    { 10, 0.017197f, 15595 },
    { 11, 0.016551f, 15595 },
    { 12, 0.015729f, 15595 },
    { 13, 0.015229f, 15595 },
    { 14, 0.014580f, 15595 },
    { 15, 0.014008f, 15595 },
    { 16, 0.013650f, 15595 },
    { 17, 0.013175f, 15595 },
    { 18, 0.012832f, 15595 },
    { 19, 0.012475f, 15595 },
    { 20, 0.012073f, 15595 },
    { 21, 0.011840f, 15595 },
    { 22, 0.011494f, 15595 },
    { 23, 0.011292f, 15595 },
    { 24, 0.010990f, 15595 },
    { 25, 0.010761f, 15595 },
    { 26, 0.010546f, 15595 },
    { 27, 0.010321f, 15595 },
    { 28, 0.010151f, 15595 },
    { 29, 0.009949f, 15595 },
    { 30, 0.009740f, 15595 },
    { 31, 0.009597f, 15595 },
    { 32, 0.009425f, 15595 },
    { 33, 0.009278f, 15595 },
    { 34, 0.009123f, 15595 },
    { 35, 0.008974f, 15595 },
    { 36, 0.008847f, 15595 },
    { 37, 0.008698f, 15595 },
    { 38, 0.008581f, 15595 },
    { 39, 0.008457f, 15595 },
    { 40, 0.008338f, 15595 },
    { 41, 0.008235f, 15595 },
    { 42, 0.008113f, 15595 },
    { 43, 0.008018f, 15595 },
    { 44, 0.007906f, 15595 },
    { 45, 0.007798f, 15595 },
    { 46, 0.007713f, 15595 },
    { 47, 0.007612f, 15595 },
    { 48, 0.007524f, 15595 },
    { 49, 0.007430f, 15595 },
    { 50, 0.007340f, 15595 },
    { 51, 0.007268f, 15595 },
    { 52, 0.007184f, 15595 },
    { 53, 0.007116f, 15595 },
    { 54, 0.007029f, 15595 },
    { 55, 0.006945f, 15595 },
    { 56, 0.006884f, 15595 },
    { 57, 0.006805f, 15595 },
    { 58, 0.006747f, 15595 },
    { 59, 0.006667f, 15595 },
    { 60, 0.006600f, 15595 },
    { 61, 0.006421f, 15595 },
    { 62, 0.006314f, 15595 },
    { 63, 0.006175f, 15595 },
    { 64, 0.006072f, 15595 },
    { 65, 0.005981f, 15595 },
    { 66, 0.005885f, 15595 },
    { 67, 0.005791f, 15595 },
    { 68, 0.005732f, 15595 },
    { 69, 0.005668f, 15595 },
    { 70, 0.005596f, 15595 },
    { 71, 0.005316f, 15595 },
    { 72, 0.005049f, 15595 },
    { 73, 0.004796f, 15595 },
    { 74, 0.004555f, 15595 },
    { 75, 0.004327f, 15595 },
    { 76, 0.004110f, 15595 },
    { 77, 0.003903f, 15595 },
    { 78, 0.003708f, 15595 },
    { 79, 0.003522f, 15595 },
    { 80, 0.003345f, 15595 },
    { 81, 0.003345f, 15595 },
    { 82, 0.003345f, 15595 },
    { 83, 0.003345f, 15595 },
    { 84, 0.003345f, 15595 },
    { 85, 0.003345f, 15595 },
};

inline float getBaseManaRegen(uint32_t level)
{
    // classic   = 60
    // tbc      += 10
    // wotlk    += 10
    // cata     += 85

    for (uint8_t i = 0; i < 165; ++i)
    {
        if (BaseManaRegenByAEVersion[i][0] == level)
        {
#if VERSION_STRING <= WotLK
            if (BaseManaRegenByAEVersion[i][2] <= getAEVersion())
                return BaseManaRegenByAEVersion[i][1];
#else
            if (BaseManaRegenByAEVersion[i][2] == getAEVersion())
                return BaseManaRegenByAEVersion[i][1];
#endif
        }
    }

    return 0.f;
}


static uint8_t getSideByRace(uint8_t race)
{
    switch (race)
    {
        case RACE_HUMAN:
        case RACE_DWARF:
        case RACE_NIGHTELF:
        case RACE_GNOME:
#if VERSION_STRING > Classic
        case RACE_DRAENEI:
#endif
#if VERSION_STRING > WotLK
        case RACE_WORGEN:
#endif
            return TEAM_ALLIANCE;
        default:
            return TEAM_HORDE;
    }
}

// action button defines
#if VERSION_STRING != TBC
    #define PLAYER_ACTION_BUTTON_COUNT 144
#else
    #define PLAYER_ACTION_BUTTON_COUNT 132
#endif

#if VERSION_STRING < Cata
    #define PLAYER_ACTION_BUTTON_SIZE PLAYER_ACTION_BUTTON_COUNT * sizeof(ActionButton)
#else
    #define PLAYER_ACTION_BUTTON_SIZE PLAYER_ACTION_BUTTON_COUNT * sizeof(uint32)
#endif

#ifdef FT_DUAL_SPEC
#define MAX_SPEC_COUNT 2
#else
#define MAX_SPEC_COUNT 1
#endif

#if VERSION_STRING >= Cata
#define GLYPHS_COUNT 9
#elif VERSION_STRING == WotLK
#define GLYPHS_COUNT 6
#endif


#if VERSION_STRING == Classic
#define MAX_QUEST_SLOT 20
#else
#define MAX_QUEST_SLOT 25
#endif
