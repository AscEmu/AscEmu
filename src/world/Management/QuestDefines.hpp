/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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

#ifndef _QUEST_DEFINES_HPP
#define _QUEST_DEFINES_HPP

#define MAX_QUEST_LOG_SIZE 25

#define QUEST_OBJECTIVES_COUNT 4
#define QUEST_ITEM_OBJECTIVES_COUNT 6
#define QUEST_SOURCE_ITEM_IDS_COUNT 4
#define QUEST_REWARD_CHOICES_COUNT 6
#define QUEST_REWARDS_COUNT 4
#define QUEST_DEPLINK_COUNT 10
#define QUEST_REPUTATIONS_COUNT 5
#define QUEST_EMOTE_COUNT 4
#define QUEST_CURRENCY_COUNT 4
#define QUEST_REQUIRED_AREA_TRIGGERS 3

enum QuestStatus
{
    QMGR_QUEST_NOT_AVAILABLE        = 0x0001,   // There aren't quests avaiable.                | "No Mark"
    QMGR_QUEST_AVAILABLELOW_LEVEL   = 0x0002,   // Quest avaiable, and your level is enough.    | "Gray Quotation Mark !"
    QMGR_QUEST_REPEATABLE_LOWLEVEL  = 0x0004,
    QMGR_QUEST_NOT_FINISHED         = 0x0010,   // Quest isnt finished yet.                     | "Gray Question ? Mark"
    QMGR_QUEST_REPEATABLE_FINISHED  = 0x0040,   // Quest is finishable                          | "Blue Question ? Mark"
    QMGR_QUEST_REPEATABLE           = 0x0080,   // Quest repeatable                             | "Blue Question ? Mark" 
    QMGR_QUEST_AVAILABLE            = 0x0100,   // Quest avaiable, and your level is enough     | "Yellow Quotation ! Mark" 
    QMGR_QUEST_FINISHED_2           = 0x0200,   // Quest has been finished                      | "No icon on the minimap"
    QMGR_QUEST_FINISHED             = 0x0400,   // Quest has been finished                      | "Yellow Question  ? Mark" 
    QMGR_QUEST_CHAT                 = 0x0800,   // Quest avaiable it shows a talk baloon.       | "No Mark"
    //QUEST_ITEM_UPDATE             = 0x06      // Yellow Question "?" Mark. //Unknown
};

enum QuestFailedReasons
{
    INVALIDREASON_DONT_HAVE_REQ                     = 0,
    INVALIDREASON_QUEST_FAILED_LOW_LEVEL            = 1,        // You are not high enough level for that quest.
    INVALIDREASON_QUEST_FAILED_WRONG_RACE           = 6,        // That quest is not available to your race.
    INVALIDREASON_QUEST_ALREADY_DONE                = 7,        // You have completed that quest.
    INVALIDREASON_QUEST_ONLY_ONE_TIMED              = 12,       // You can only be on one timed quest at a time.
    INVALIDREASON_QUEST_ALREADY_ON                  = 13,       // You are already on that quest.
    INVALIDREASON_QUEST_FAILED_EXPANSION            = 16,       // This quest requires an expansion enabled account.
    INVALIDREASON_QUEST_ALREADY_ON2                 = 18,       // You are already on that quest.
    INVALIDREASON_QUEST_FAILED_MISSING_ITEMS        = 21,       // You don't have the required items with you. Check storage.
    INVALIDREASON_QUEST_FAILED_NOT_ENOUGH_MONEY     = 23,       // You don't have enough money for that quest.
    INVALIDREASON_DAILY_QUESTS_REMAINING            = 26,       // You have already completed 25 daily quests today.
    INVALIDREASON_QUEST_FAILED_CAIS                 = 27,       // You cannot complete quests once you have reached tired time.
    INVALIDREASON_DAILY_QUEST_COMPLETED_TODAY       = 29        // You have completed that daily quest today.
};

enum FAILED_REASON
{
    FAILED_REASON_FAILED            = 0,
    FAILED_REASON_INV_FULL          = 4,
    FAILED_REASON_DUPE_ITEM_FOUND   = 17,
};

enum QuestShareMessages
{
    QUEST_PARTY_MSG_SHARING_QUEST           = 0,
    QUEST_PARTY_MSG_CANT_TAKE_QUEST         = 1,
    QUEST_PARTY_MSG_ACCEPT_QUEST            = 2,
    QUEST_PARTY_MSG_DECLINE_QUEST           = 3,
    QUEST_PARTY_MSG_BUSY                    = 4,
    QUEST_PARTY_MSG_LOG_FULL                = 5,
    QUEST_PARTY_MSG_HAVE_QUEST              = 6,
    QUEST_PARTY_MSG_FINISH_QUEST            = 7,
    QUEST_PARTY_MSG_CANT_BE_SHARED_TODAY    = 8,
    QUEST_PARTY_MSG_SHARING_TIMER_EXPIRED   = 9,
    QUEST_PARTY_MSG_NOT_IN_PARTY            = 10
};

enum __QuestTradeSkill
{
    QUEST_TRSKILL_NONE              = 0,
    QUEST_TRSKILL_ALCHEMY           = 1,
    QUEST_TRSKILL_BLACKSMITHING     = 2,
    QUEST_TRSKILL_COOKING           = 3,
    QUEST_TRSKILL_ENCHANTING        = 4,
    QUEST_TRSKILL_ENGINEERING       = 5,
    QUEST_TRSKILL_FIRSTAID          = 6,
    QUEST_TRSKILL_HERBALISM         = 7,
    QUEST_TRSKILL_LEATHERWORKING    = 8,
    QUEST_TRSKILL_POISONS           = 9,
    QUEST_TRSKILL_TAILORING         = 10,
    QUEST_TRSKILL_MINING            = 11,
    QUEST_TRSKILL_FISHING           = 12,
    QUEST_TRSKILL_SKINNING          = 13,
    QUEST_TRSKILL_JEWELCRAFTING     = 14
};

enum QUESTGIVER_QUEST_TYPE
{
    QUESTGIVER_QUEST_START  = 0x01,
    QUESTGIVER_QUEST_END    = 0x02,
};

enum QUEST_TYPE
{
    QUEST_GATHER    = 0x01,
    QUEST_SLAY  = 0x02,
};

enum QuestFlags
{
    // Flags used at server and sent to client
    QUEST_FLAGS_NONE            = 0x00000000,
    QUEST_FLAGS_STAY_ALIVE      = 0x00000001,       // Not used currently
    QUEST_FLAGS_PARTY_ACCEPT    = 0x00000002,       // Not used currently. If player in party, all players that can accept this quest will receive confirmation box to accept quest CMSG_QUEST_CONFIRM_ACCEPT/SMSG_QUEST_CONFIRM_ACCEPT
    QUEST_FLAGS_EXPLORATION     = 0x00000004,       // Not used currently
    QUEST_FLAGS_SHARABLE        = 0x00000008,       // Can be shared: Player::CanShareQuest()
    //QUEST_FLAGS_NONE2         = 0x00000010,       // Not used currently
    QUEST_FLAGS_EPIC            = 0x00000020,       // Not used currently: Unsure of content
    QUEST_FLAGS_RAID            = 0x00000040,       // Not used currently
    QUEST_FLAGS_TBC             = 0x00000080,       // Not used currently: Available if TBC expansion enabled only
    QUEST_FLAGS_DELIVER_MORE    = 0x00000100,       // Not used currently: _DELIVER_MORE Quest needs more than normal _q-item_ drops from mobs
    QUEST_FLAGS_HIDDEN_REWARDS  = 0x00000200,       // Items and money rewarded only sent in SMSG_QUESTGIVER_OFFER_REWARD (not in SMSG_QUESTGIVER_QUEST_DETAILS or in client quest log(SMSG_QUEST_QUERY_RESPONSE))
    QUEST_FLAGS_AUTO_REWARDED   = 0x00000400,       // These quests are automatically rewarded on quest complete and they will never appear in quest log client side.
    QUEST_FLAGS_TBC_RACES       = 0x00000800,       // Not used currently: Blood elf/Draenei starting zone quests
    QUEST_FLAGS_DAILY           = 0x00001000,       // Used to know quest is Daily one
    QUEST_FLAGS_REPEATABLE      = 0x00002000,       // Used on repeatable quests (3.0.0+)
    QUEST_FLAGS_UNAVAILABLE     = 0x00004000,       // Used on quests that are not generically available
    QUEST_FLAGS_WEEKLY          = 0x00008000,
    QUEST_FLAGS_AUTOCOMPLETE    = 0x00010000,       // auto complete
    QUEST_FLAGS_SPECIAL_ITEM    = 0x00020000,       // has something to do with ReqItemId and SrcItemId
    QUEST_FLAGS_OBJ_TEXT        = 0x00040000,       // use Objective text as Complete text
    QUEST_FLAGS_AUTO_ACCEPT     = 0x00080000,       // The client recognizes this flag as auto-accept.
};

enum QuestSpecialFlags
{
    // SkyFire flags for set SpecialFlags in DB if required but used only at server
    QUEST_SPECIAL_FLAG_REPEATABLE           = 0x001,  // Set by 1 in SpecialFlags from DB
    QUEST_SPECIAL_FLAG_EXPLORATION_OR_EVENT = 0x002,  // Set by 2 in SpecialFlags from DB (if reequired area explore, spell SPELL_EFFECT_QUEST_COMPLETE casting, table `*_script` command SCRIPT_COMMAND_QUEST_EXPLORED use, set from script)
    //QUEST_SPECIAL_FLAG_MONTHLY            = 0x004,  // Set by 4 in SpecialFlags. Quest reset for player at beginning of month.
    QUEST_SPECIAL_FLAG_AUTO_ACCEPT          = 0x400,  // Set by 4 in SpecialFlags in DB if the quest is to be auto-accepted.
    QUEST_SPECIAL_FLAG_DF_QUEST             = 0x800,  // Set by 8 in SpecialFlags in DB if the quest is used by Dungeon Finder.
};

enum QUEST_SHARE
{
    QUEST_SHARE_MSG_SHARING_QUEST           = 0,
    QUEST_SHARE_MSG_CANT_TAKE_QUEST         = 1,
    QUEST_SHARE_MSG_ACCEPT_QUEST            = 2,
    QUEST_SHARE_MSG_REFUSE_QUEST            = 3,
    //QUEST_SHARE_MSG_TOO_FAR               = 4,    /// This message seems to be non-existent as of 3.2.x, plus it isn't used in ArcEmu, so it is safe to get rid of it.
    QUEST_SHARE_MSG_BUSY                    = 4,
    QUEST_SHARE_MSG_LOG_FULL                = 5,
    QUEST_SHARE_MSG_HAVE_QUEST              = 6,
    QUEST_SHARE_MSG_FINISH_QUEST            = 7,
    QUEST_SHARE_MSG_CANT_BE_SHARED_TODAY    = 8,    /// The following 4 messages (from 8 to 11) are unused on ArcEmu, but for completeness I have included them here, maybe we'll need them later...
    QUEST_SHARE_MSG_SHARING_TIMER_EXPIRED   = 9,
    QUEST_SHARE_MSG_NOT_IN_PARTY            = 10,
    QUEST_SHARE_MSG_DIFFERENT_SERVER_DAILY  = 11
};

enum QUEST_MOB_TYPES
{
    QUEST_MOB_TYPE_CREATURE     = 1,
    QUEST_MOB_TYPE_GAMEOBJECT   = 2
};

enum QuestCompletionStatus
{
    QUEST_INCOMPLETE    = 0,
    QUEST_COMPLETE      = 1,
    QUEST_FAILED        = 2
};

// values based at QuestInfo.dbc
enum QuestTypes
{
    QUEST_TYPE_ELITE        = 1,
    QUEST_TYPE_LIFE         = 21,
    QUEST_TYPE_PVP          = 41,
    QUEST_TYPE_RAID         = 62,
    QUEST_TYPE_DUNGEON      = 81,
    QUEST_TYPE_WORLD_EVENT  = 82,
    QUEST_TYPE_LEGENDARY    = 83,
    QUEST_TYPE_ESCORT       = 84,
    QUEST_TYPE_HEROIC       = 85,
    QUEST_TYPE_RAID_10      = 88,
    QUEST_TYPE_RAID_25      = 89
};

// values based at QuestSort.dbc
enum QuestSort
{
    QUEST_SORT_EPIC                 = 1,
    QUEST_SORT_WAILING_CAVERNS_OLD  = 21,
    QUEST_SORT_SEASONAL             = 22,
    QUEST_SORT_UNDERCITY_OLD        = 23,
    QUEST_SORT_HERBALISM            = 24,
    QUEST_SORT_BATTLEGROUNDS        = 25,
    QUEST_SORT_ULDAMN_OLD           = 41,
    QUEST_SORT_WARLOCK              = 61,
    QUEST_SORT_WARRIOR              = 81,
    QUEST_SORT_SHAMAN               = 82,
    QUEST_SORT_FISHING              = 101,
    QUEST_SORT_BLACKSMITHING        = 121,
    QUEST_SORT_PALADIN              = 141,
    QUEST_SORT_MAGE                 = 161,
    QUEST_SORT_ROGUE                = 162,
    QUEST_SORT_ALCHEMY              = 181,
    QUEST_SORT_LEATHERWORKING       = 182,
    QUEST_SORT_ENGINERING           = 201,
    QUEST_SORT_TREASURE_MAP         = 221,
    QUEST_SORT_SUNKEN_TEMPLE_OLD    = 241,
    QUEST_SORT_HUNTER               = 261,
    QUEST_SORT_PRIEST               = 262,
    QUEST_SORT_DRUID                = 263,
    QUEST_SORT_TAILORING            = 264,
    QUEST_SORT_SPECIAL              = 284,
    QUEST_SORT_COOKING              = 304,
    QUEST_SORT_FIRST_AID            = 324,
    QUEST_SORT_LEGENDARY            = 344,
    QUEST_SORT_DARKMOON_FAIRE       = 364,
    QUEST_SORT_AHN_QIRAJ_WAR        = 365,
    QUEST_SORT_LUNAR_FESTIVAL       = 366,
    QUEST_SORT_REPUTATION           = 367,
    QUEST_SORT_INVASION             = 368,
    QUEST_SORT_MIDSUMMER            = 369,
    QUEST_SORT_BREWFEST             = 370,
    QUEST_SORT_INSCRIPTION          = 371,
    QUEST_SORT_DEATH_KNIGHT         = 372,
    QUEST_SORT_JEWELCRAFTING        = 373,
    QUEST_SORT_NOBLEGARDEN          = 374,
    QUEST_SORT_PILGRIMS_BOUNTY      = 375,
    QUEST_SORT_LOVE_IS_IN_THE_AIR   = 376,
    QUEST_SORT_ARCHAEOLOGY          = 377
};

#endif // _QUEST_DEFINES_HPP
