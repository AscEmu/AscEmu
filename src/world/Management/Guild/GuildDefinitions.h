/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldConf.h"

#include <string>
#include <cstdint>

namespace CharterEntry
{
    enum
    {
        Guild = 5863,
        TwoOnTwo = 23560,
        ThreeOnThree = 23561,
        FiveOnFive = 23562
    };
}

namespace CharterRequiredSigns
{
    enum
    {
        Guild = 9,
        TwoOnTwo = 1,
        ThreeOnThree = 2,
        FiveOnFive = 4
    };
}

#define CHARTER_DISPLAY_ID 16161

namespace CharterType
{
    enum
    {
        Guild = 0,
        Arena = 1
    };
}

#define MAX_GUILD_BANK_SLOTS        98

#if VERSION_STRING >= Cata
#define MAX_GUILD_BANK_TABS         8
#else
#define MAX_GUILD_BANK_TABS         6
#endif

#define GUILD_BANK_MONEY_TAB        100

#define MIN_GUILD_RANKS             2
#define MAX_GUILD_RANKS             10

#define UNCAPPED_GUILD_LEVEL        20

#define GUILD_RANK_NONE             0xFF
#define UNDEFINED_GUILD_TAB         0xFF
#define UNDEFINED_TAB_SLOT          0xFF
#define UNLIMITED_WITHDRAW_MONEY    0xFFFFFFFF
#define UNLIMITED_WITHDRAW_SLOTS    0xFFFFFFFF
#define UNK_EVENT_LOG_GUID          0xFFFFFFFF

#define MAX_GUILD_BANK_TAB_TEXT_LEN 500
#define GOLD                        10000
#define EMBLEM_PRICE                10 * GOLD


enum GuildMemberData
{
    GM_DATA_ZONEID = 0,
    GM_DATA_ACHIEVEMENT_POINTS = 1,
    GM_DATA_LEVEL = 2
};

enum GuildDefaultRanks
{
    GR_GUILDMASTER = 0,
    GR_OFFICER = 1,
    GR_VETERAN = 2,
    GR_MEMBER = 3,
    GR_INITIATE = 4
};

enum GuildRankRights
{
    GR_RIGHT_EMPTY = 0x00000040,
    GR_RIGHT_GCHATLISTEN = GR_RIGHT_EMPTY | 0x00000001,
    GR_RIGHT_GCHATSPEAK = GR_RIGHT_EMPTY | 0x00000002,
    GR_RIGHT_OFFCHATLISTEN = GR_RIGHT_EMPTY | 0x00000004,
    GR_RIGHT_OFFCHATSPEAK = GR_RIGHT_EMPTY | 0x00000008,
    GR_RIGHT_INVITE = GR_RIGHT_EMPTY | 0x00000010,
    GR_RIGHT_REMOVE = GR_RIGHT_EMPTY | 0x00000020,
    GR_RIGHT_PROMOTE = GR_RIGHT_EMPTY | 0x00000080,
    GR_RIGHT_DEMOTE = GR_RIGHT_EMPTY | 0x00000100,
    GR_RIGHT_SETMOTD = GR_RIGHT_EMPTY | 0x00001000,
    GR_RIGHT_EPNOTE = GR_RIGHT_EMPTY | 0x00002000,
    GR_RIGHT_VIEWOFFNOTE = GR_RIGHT_EMPTY | 0x00004000,
    GR_RIGHT_EOFFNOTE = GR_RIGHT_EMPTY | 0x00008000,
    GR_RIGHT_MODIFY_GUILD_INFO = GR_RIGHT_EMPTY | 0x00010000,
    GR_RIGHT_WITHDRAW_GOLD_LOCK = 0x00020000,
    GR_RIGHT_WITHDRAW_REPAIR = 0x00040000,
    GR_RIGHT_WITHDRAW_GOLD = 0x00080000,
    GR_RIGHT_CREATE_GUILD_EVENT = 0x00100000,
    GR_RIGHT_ALL = 0x00DDFFBF
};

enum GuildCommandType
{
    GC_TYPE_CREATE = 0,
    GC_TYPE_INVITE = 1,
    GC_TYPE_QUIT = 3,
    GC_TYPE_ROSTER = 5,
    GC_TYPE_PROMOTE = 6,
    GC_TYPE_DEMOTE = 7,
    GC_TYPE_REMOVE = 8,
    GC_TYPE_CHANGE_LEADER = 10,
    GC_TYPE_EDIT_MOTD = 11,
    GC_TYPE_GUILD_CHAT = 13,
    GC_TYPE_FOUNDER = 14,
    GC_TYPE_CHANGE_RANK = 16,
    GC_TYPE_PUBLIC_NOTE = 19,
    GC_TYPE_VIEW_TAB = 21,
    GC_TYPE_MOVE_ITEM = 22,
    GC_TYPE_REPAIR = 25
};

enum GuildCommandError
{
    GC_ERROR_SUCCESS = 0,
    GC_ERROR_INTERNAL = 1,
    GC_ERROR_ALREADY_IN_GUILD = 2,
    GC_ERROR_ALREADY_IN_GUILD_S = 3,
    GC_ERROR_INVITED_TO_GUILD = 4,
    GC_ERROR_ALREADY_INVITED_TO_GUILD = 5,
    GC_ERROR_NAME_INVALID = 6,
    GC_ERROR_NAME_EXISTS_S = 7,
    GC_ERROR_LEADER_LEAVE = 8,
    GC_ERROR_PERMISSIONS = 8,
    GC_ERROR_PLAYER_NOT_IN_GUILD = 9,
    GC_ERROR_PLAYER_NOT_IN_GUILD_S = 10,
    GC_ERROR_PLAYER_NOT_FOUND_S = 11,
    GC_ERROR_NOT_ALLIED = 12,
    GC_ERROR_RANK_TOO_HIGH_S = 13,
    GC_ERROR_RANK_TOO_LOW_S = 14,
    GC_ERROR_RANKS_LOCKED = 17,
    GC_ERROR_RANK_IN_USE = 18,
    GC_ERROR_IGNORING_YOU_S = 19,
    GC_ERROR_UNK1 = 20,
    GC_ERROR_WITHDRAW_LIMIT = 25,
    GC_ERROR_NOT_ENOUGH_MONEY = 26,
    GC_ERROR_BANK_FULL = 28,
    GC_ERROR_ITEM_NOT_FOUND = 29,
    GC_ERROR_TOO_MUCH_MONEY = 31,
    GC_ERROR_BANK_WRONG_TAB = 32,
    GC_ERROR_REQUIRES_AUTHENTICATOR = 34,
    GC_ERROR_BANK_VOUCHER_FAILED = 35,
    GC_ERROR_TRIAL_ACCOUNT = 36,
    GC_ERROR_UNDELETABLE_DUE_TO_LEVEL = 37,
    GC_ERROR_MOVE_STARTING = 38,
    GC_ERROR_REP_TOO_LOW = 39
};

enum GuildEvents : uint8_t
{
#if VERSION_STRING >= Cata
    GE_PROMOTION = 1,
    GE_DEMOTION = 2,
    GE_MOTD = 3,
    GE_JOINED = 4,
    GE_LEFT = 5,
    GE_REMOVED = 6,
    GE_LEADER_IS = 7,
    GE_LEADER_CHANGED = 8,
    GE_DISBANDED = 9,
    GE_TABARDCHANGE = 10,
    GE_RANK_UPDATED = 11,
    GE_RANK_CREATED = 12,
    GE_RANK_DELETED = 13,
    GE_RANK_ORDER_CHANGED = 14,
    GE_FOUNDER = 15,
    GE_SIGNED_ON = 16,
    GE_SIGNED_OFF = 17,
    GE_GUILDBANKBAGSLOTS_CHANGED = 18,
    GE_BANK_TAB_PURCHASED = 19,
    GE_BANK_TAB_UPDATED = 20,
    GE_BANK_MONEY_SET = 21,
    GE_BANK_TAB_AND_MONEY_UPDATED = 22,
    GE_BANK_TEXT_CHANGED = 23,
    GE_SIGNED_ON_MOBILE = 25,
    GE_SIGNED_Off_MOBILE = 26
#else
    GE_PROMOTION = 0,
    GE_DEMOTION = 1,
    GE_MOTD = 2,
    GE_JOINED = 3,
    GE_LEFT = 4,
    GE_REMOVED = 5,
    GE_LEADER_IS = 6,
    GE_LEADER_CHANGED = 7,
    GE_DISBANDED = 8,
    GE_TABARDCHANGE = 9,
    GE_RANK_UPDATED = 10,
    GE_RANK_DELETED = 11,
    GE_SIGNED_ON = 12,
    GE_SIGNED_OFF = 13,
    GE_GUILDBANKBAGSLOTS_CHANGED = 14,
    GE_BANK_TAB_PURCHASED = 15,
    GE_BANK_TAB_UPDATED = 16,
    GE_BANK_MONEY_SET = 17,
    GE_BANK_TAB_AND_MONEY_UPDATED = 18,
    GE_BANK_TEXT_CHANGED = 19
#endif
};

enum PetitionError
{
    PETITION_ERROR_OK = 0,
    PETITION_ERROR_ALREADY_SIGNED = 1,
    PETITION_ERROR_ALREADY_IN_GUILD = 2,
    PETITION_ERROR_CREATOR = 3,
    PETITION_ERROR_NEED_MORE_SIGNATURES = 4,
    PETITION_ERROR_GUILD_PERMISSIONS = 11,
    PETITION_ERROR_GUILD_NAME_INVALID = 12
};

enum PetitionSigns
{
    PETITION_SIGN_OK = 0,
    PETITION_SIGN_ALREADY_SIGNED = 1,
    PETITION_SIGN_ALREADY_IN_GUILD = 2,
    PETITION_SIGN_CANT_SIGN_OWN = 3,
    PETITION_SIGN_NOT_SERVER = 4,
    PETITION_SIGN_FULL = 5,
    PETITION_SIGN_ALREADY_SIGNED_OTHER = 6,
    PETITION_SIGN_RESTRICTED_ACCOUNT = 7
};

enum GuildBankRights
{
    GB_RIGHT_VIEW_TAB = 0x01,
    GB_RIGHT_PUT_ITEM = 0x02,
    GB_RIGHT_UPDATE_TEXT = 0x04,

    GB_RIGHT_DEPOSIT_ITEM = GB_RIGHT_VIEW_TAB | GB_RIGHT_PUT_ITEM,
    GB_RIGHT_FULL = 0xFF
};

enum GuildBankEventLogTypes
{
    GB_LOG_DEPOSIT_ITEM = 1,
    GB_LOG_WITHDRAW_ITEM = 2,
    GB_LOG_MOVE_ITEM = 3,
    GB_LOG_DEPOSIT_MONEY = 4,
    GB_LOG_WITHDRAW_MONEY = 5,
    GB_LOG_REPAIR_MONEY = 6,
    GB_LOG_MOVE_ITEM2 = 7,
    GB_LOG_UNK1 = 8,
    GB_LOG_BUY_SLOT = 9,
    GB_LOG_CASH_FLOW_DEPOSIT = 10
};

enum GuildEventLogTypes
{
    GE_LOG_INVITE_PLAYER = 1,
    GE_LOG_JOIN_GUILD = 2,
    GE_LOG_PROMOTE_PLAYER = 3,
    GE_LOG_DEMOTE_PLAYER = 4,
    GE_LOG_UNINVITE_PLAYER = 5,
    GE_LOG_LEAVE_GUILD = 6
};

enum GuildEmblemError
{
    GEM_ERROR_SUCCESS = 0,
    GEM_ERROR_INVALID_TABARD_COLORS = 1,
    GEM_ERROR_NOGUILD = 2,
    GEM_ERROR_NOTGUILDMASTER = 3,
    GEM_ERROR_NOTENOUGHMONEY = 4,
    GEM_ERROR_INVALIDVENDOR = 5
};

enum GuildMemberFlags
{
    GEM_STATUS_NONE = 0x0,
    GEM_STATUS_ONLINE = 0x1,
    GEM_STATUS_AFK = 0x2,
    GEM_STATUS_DND = 0x4,
    GEM_STATUS_MOBILE = 0x8
};

enum GuildNews
{
    GN_GUILD_ACHIEVEMENT = 0,
    GN_PLAYER_ACHIEVEMENT = 1,
    GN_DUNGEON_ENCOUNTER = 2,
    GN_ITEM_LOOTED = 3,
    GN_ITEM_CRAFTED = 4,
    GN_ITEM_PURCHASED = 5,
    GN_LEVEL_UP = 6
};

struct GuildReward
{
    uint32_t entry;
    int32_t racemask;
    uint64_t price;
    uint32_t achievementId;
    uint8_t standing;
};

uint32_t const minNewItemLevel[4] = { 61, 90, 200, 353 };
uint32_t const guildChallengeGoldReward[4] = { 0, 250, 1000, 500 };
uint32_t const guildChallengeMaxLevelGoldReward[4] = { 0, 125, 500, 250 };
uint32_t const guildChallengeXPReward[4] = { 0, 300000, 3000000, 1500000 };
uint32_t const guildChallengesPerWeek[4] = { 0, 7, 1, 3 };

inline std::string _GetGuildEventString(GuildEvents event)
{
    switch (event)
    {
        case GE_PROMOTION:                  { return "Member promotion"; }
        case GE_DEMOTION:                   { return "Member demotion"; }
        case GE_MOTD:                       { return "Guild MOTD"; }
        case GE_JOINED:                     { return "Member joined"; }
        case GE_LEFT:                       { return "Member left"; }
        case GE_REMOVED:                    { return "Member removed"; }
        case GE_LEADER_IS:                  { return "Leader is"; }
        case GE_LEADER_CHANGED:             { return "Leader changed"; }
        case GE_DISBANDED:                  { return "Guild disbanded"; }
        case GE_TABARDCHANGE:               { return "Tabard change"; }
        case GE_RANK_UPDATED:               { return "Rank updated"; }
        case GE_RANK_DELETED:               { return "Rank deleted"; }
        case GE_SIGNED_ON:                  { return "Member signed on"; }
        case GE_SIGNED_OFF:                 { return "Member signed off"; }
        case GE_GUILDBANKBAGSLOTS_CHANGED:  { return "Bank bag slots changed"; }
        case GE_BANK_TAB_PURCHASED:         { return "Bank tab purchased"; }
        case GE_BANK_TAB_UPDATED:           { return "Bank tab updated"; }
        case GE_BANK_MONEY_SET:             { return "Bank money set"; }
        case GE_BANK_TAB_AND_MONEY_UPDATED: { return "Bank and money updated"; }
        case GE_BANK_TEXT_CHANGED:          { return "Bank tab text changed"; }
        default:
            break;
    }
    return "None";
}

inline uint32_t _GetGuildBankTabPrice(uint8_t tabId)
{
    switch (tabId)
    {
        case 0: { return 100; }
        case 1: { return 250; }
        case 2: { return 500; }
        case 3: { return 1000; }
        case 4: { return 2500; }
        case 5: { return 5000; }
        default: { return 0; }
    }
}
