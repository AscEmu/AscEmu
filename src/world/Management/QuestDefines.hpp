/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _QUEST_DEFINES_HPP
#define _QUEST_DEFINES_HPP

#define MAX_QUEST_LOG_SIZE 25

#define QUEST_REPEATABLE 1
#define QUEST_REPEATABLE_DAILY 2

#define QUEST_MAX_REWARD_ITEM 4
#define QUEST_MAX_OBJECTIVES 4
#define MAX_REQUIRED_QUEST_ITEM 6
#define QUEST_REWARD_CURRENCY_COUNT 4
#define QUEST_REQUIRED_CURRENCY_COUNT 4

enum QUEST_STATUS
{
    QMGR_QUEST_NOT_AVAILABLE                = 0x00,     // There aren't any quests available.              | "No Mark"
    QMGR_QUEST_AVAILABLELOW_LEVEL           = 0x01,     // Quest available, and your level isn't enough.   | "Gray Quotation Mark !"
    QMGR_QUEST_CHAT                         = 0x02,     // Quest available it shows a talk balloon.        | "No Mark"
    QMGR_QUEST_REPEATABLE_FINISHED_LOWLEVEL = 0x03,
    QMGR_QUEST_REPEATABLE_LOWLEVEL          = 0x04,
    QMGR_QUEST_NOT_FINISHED                 = 0x05,     // Quest isn't finished yet.                       | "Gray Question ? Mark"
    QMGR_QUEST_REPEATABLE_FINISHED          = 0x06,
    QMGR_QUEST_REPEATABLE                   = 0x07,     // Quest repeatable                                | "Blue Question ? Mark"
    QMGR_QUEST_AVAILABLE                    = 0x08,     // Quest available, and your level is enough       | "Yellow Quotation ! Mark"
    QMGR_QUEST_FINISHED                     = 0x0A,     // Quest has been finished.                        | "Yellow Question  ? Mark" (7 has no minimap icon)
    //QUEST_ITEM_UPDATE                     = 0x06      // Yellow Question "?" Mark. //Unknown
};

enum QUESTGIVER_QUEST_TYPE
{
    QUESTGIVER_QUEST_START  = 0x01,
    QUESTGIVER_QUEST_END    = 0x02
};

enum QUEST_TYPE
{
    QUEST_GATHER    = 0x01,
    QUEST_SLAY      = 0x02
};

enum QuestFlag
{
    QUEST_FLAG_NONE               = 0x00000000,
    QUEST_FLAG_DELIVER            = 0x00000001,
    QUEST_FLAG_KILL               = 0x00000002,
    QUEST_FLAG_SPEAKTO            = 0x00000004,
    QUEST_FLAG_REPEATABLE         = 0x00000008,
    QUEST_FLAG_EXPLORATION        = 0x00000010,
    QUEST_FLAG_TIMED              = 0x00000020,
    QUEST_FLAG_UNK1               = 0x00000040,
    QUEST_FLAG_REPUTATION         = 0x00000080,
    QUEST_FLAGS_UNK2              = 0x00000100,     // Not used currently: _DELIVER_MORE Quest needs more than normal _q-item_ drops from mobs
    QUEST_FLAGS_HIDDEN_REWARDS    = 0x00000200,     // Items and money rewarded only sent in SMSG_QUESTGIVER_OFFER_REWARD (not in SMSG_QUESTGIVER_QUEST_DETAILS or in client quest log(SMSG_QUEST_QUERY_RESPONSE))
    QUEST_FLAGS_AUTO_REWARDED     = 0x00000400,     // These quests are automatically rewarded on quest complete and they will never appear in quest log client side.
    QUEST_FLAGS_TBC_RACES         = 0x00000800,     // Not used currently: Blood elf/Draenei starting zone quests
    QUEST_FLAGS_DAILY             = 0x00001000,     // Daily quest. Can be done once a day. Quests reset at regular intervals for all players.
    QUEST_FLAGS_FLAGS_PVP         = 0x00002000,     // activates PvP on accept
    QUEST_FLAGS_UNK4              = 0x00004000,     // ? Membership Card Renewal
    QUEST_FLAGS_WEEKLY            = 0x00008000,     // Weekly quest. Can be done once a week. Quests reset at regular intervals for all players.
    QUEST_FLAGS_AUTOCOMPLETE      = 0x00010000,     // auto complete
    QUEST_FLAGS_UNK5              = 0x00020000,     // has something to do with ReqItemId and SrcItemId
    QUEST_FLAGS_UNK6              = 0x00040000,     // use Objective text as Complete text
    QUEST_FLAGS_AUTO_ACCEPT       = 0x00080000      // quests in starting areas
};

enum FAILED_REASON
{
    FAILED_REASON_FAILED            = 0,
    FAILED_REASON_INV_FULL          = 4,
    FAILED_REASON_DUPE_ITEM_FOUND   = 17
};

enum INVALID_REASON
{
    INVALID_REASON_DONT_HAVE_REQ            = 0,
    INVALID_REASON_DONT_HAVE_LEVEL          = 1,
    INVALID_REASON_DONT_HAVE_RACE           = 6,
    INVALID_REASON_COMPLETED_QUEST          = 7,
    INVALID_REASON_HAVE_TIMED_QUEST         = 12,
    INVALID_REASON_HAVE_QUEST               = 13,
//	INVALID_REASON_DONT_HAVE_REQ_ITEMS      = 0x13,
//	INVALID_REASON_DONT_HAVE_REQ_MONEY      = 0x15,
    INVALID_REASON_DONT_HAVE_EXP_ACCOUNT    = 16,
    INVALID_REASON_DONT_HAVE_REQ_ITEMS      = 21,       //changed for 2.1.3
    INVALID_REASON_DONT_HAVE_REQ_MONEY      = 23,
    INVALID_REASON_REACHED_DAILY_LIMIT      = 26,       // "you have completed xx daily quests today" confirmed :)
    INVALID_REASON_UNKNOW27                 = 27,       // "You cannot completed quests once you have reached tired time"
};

enum QUEST_SHARE
{
    QUEST_SHARE_MSG_SHARING_QUEST           = 0,
    QUEST_SHARE_MSG_CANT_TAKE_QUEST         = 1,
    QUEST_SHARE_MSG_ACCEPT_QUEST            = 2,
    QUEST_SHARE_MSG_REFUSE_QUEST            = 3,
    QUEST_SHARE_MSG_BUSY                    = 4,
    QUEST_SHARE_MSG_LOG_FULL                = 5,
    QUEST_SHARE_MSG_HAVE_QUEST              = 6,
    QUEST_SHARE_MSG_FINISH_QUEST			= 7,
    QUEST_SHARE_MSG_CANT_BE_SHARED_TODAY    = 8,        // The following 4 messages (from 8 to 11) are unused on ArcEmu, but for completeness I have included them here, maybe we'll need them later...
    QUEST_SHARE_MSG_SHARING_TIMER_EXPIRED   = 9,
    QUEST_SHARE_MSG_NOT_IN_PARTY            = 10,
    QUEST_SHARE_MSG_DIFFERENT_SERVER_DAILY  = 11,
};

enum QUEST_MOB_TYPES
{
    QUEST_MOB_TYPE_CREATURE         = 1,
    QUEST_MOB_TYPE_GAMEOBJECT       = 2
};

enum QuestCompletionStatus
{
	QUEST_INCOMPLETE = 0,
	QUEST_COMPLETE   = 1,
	QUEST_FAILED     = 2
};

#endif // _QUEST_DEFINES_HPP
