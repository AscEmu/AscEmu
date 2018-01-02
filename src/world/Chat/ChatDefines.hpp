/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum ChatMsg
{
    CHAT_MSG_ADDON                      = -1,
    CHAT_MSG_SYSTEM                     = 0,    // 28, CHAT_MSG_SYSTEM = 0x00
    CHAT_MSG_SAY                        = 1,
    CHAT_MSG_PARTY                      = 2,
    CHAT_MSG_RAID                       = 3,
    CHAT_MSG_GUILD                      = 4,
    CHAT_MSG_OFFICER                    = 5,
    CHAT_MSG_YELL                       = 6,
    CHAT_MSG_WHISPER                    = 7,
    CHAT_MSG_WHISPER_MOB                = 8,    // CHAT_MSG_WHISPER_INFORM
    CHAT_MSG_WHISPER_INFORM             = 9,    // CHAT_MSG_REPLY
    CHAT_MSG_EMOTE                      = 10,
    CHAT_MSG_TEXT_EMOTE                 = 11,
    CHAT_MSG_MONSTER_SAY                = 12,
    CHAT_MSG_MONSTER_PARTY              = 13,
    CHAT_MSG_MONSTER_YELL               = 14,
    CHAT_MSG_MONSTER_WHISPER            = 15,
    CHAT_MSG_MONSTER_EMOTE              = 16,
    CHAT_MSG_CHANNEL                    = 17,
    CHAT_MSG_CHANNEL_JOIN               = 18,
    CHAT_MSG_CHANNEL_LEAVE              = 19,
    CHAT_MSG_CHANNEL_LIST               = 20,
    CHAT_MSG_CHANNEL_NOTICE             = 21,
    CHAT_MSG_CHANNEL_NOTICE_USER        = 22,
    CHAT_MSG_AFK                        = 23,
    CHAT_MSG_DND                        = 24,
    CHAT_MSG_IGNORED                    = 25,
    CHAT_MSG_SKILL                      = 26,
    CHAT_MSG_LOOT                       = 27,
    CHAT_MSG_MONEY                      = 28,
    CHAT_MSG_OPENING                    = 29,
    CHAT_MSG_TRADESKILLS                = 30,
    CHAT_MSG_PET_INFO                   = 31,
    CHAT_MSG_COMBAT_MISC_INFO           = 32,
    CHAT_MSG_COMBAT_XP_GAIN             = 33,
    CHAT_MSG_COMBAT_HONOR_GAIN          = 34,
    CHAT_MSG_COMBAT_FACTION_CHANGE      = 35,
    CHAT_MSG_BG_EVENT_NEUTRAL           = 36,
    CHAT_MSG_BG_EVENT_ALLIANCE          = 37,
    CHAT_MSG_BG_EVENT_HORDE             = 38,
    CHAT_MSG_RAID_LEADER                = 39,
    CHAT_MSG_RAID_WARNING               = 40,
    CHAT_MSG_RAID_WARNING_WIDESCREEN    = 41,
    CHAT_MSG_RAID_BOSS_EMOTE            = 42,
    CHAT_MSG_FILTERED                   = 43,
    CHAT_MSG_BATTLEGROUND               = 44,
    CHAT_MSG_BATTLEGROUND_LEADER        = 45,
    CHAT_MSG_RESTRICTED                 = 46,
    CHAT_MSG_ACHIEVEMENT                = 48,
    CHAT_MSG_GUILD_ACHIEVEMENT          = 49,
    CHAT_MSG_PARTY_LEADER               = 51
};

enum Languages
{
    LANG_ADDON          = -1,
    LANG_UNIVERSAL      = 0x00,
    LANG_ORCISH         = 0x01,
    LANG_DARNASSIAN     = 0x02,
    LANG_TAURAHE        = 0x03,
    LANG_DWARVISH       = 0x06,
    LANG_COMMON         = 0x07,
    LANG_DEMONIC        = 0x08,
    LANG_TITAN          = 0x09,
    LANG_THELASSIAN     = 0x0A,
    LANG_DRACONIC       = 0x0B,
    LANG_KALIMAG        = 0x0C,
    LANG_GNOMISH        = 0x0D,
    LANG_TROLL          = 0x0E,
    LANG_GUTTERSPEAK    = 0x21,
    LANG_DRAENEI        = 0x23,
#if VERSION_STRING != Cata
    NUM_LANGUAGES       = 0x24
#else
    LANG_ZOMBIE         = 0x24,
    LANG_GNOMISH_BINARY = 0x25,
    LANG_GOBLIN_BINARY  = 0x26,
    LANG_WORGEN         = 0x27,
    LANG_GOBLIN         = 0x28,
    NUM_LANGUAGES       = 0x29
#endif
};


#define MSG_COLOR_LIGHTRED          "|cffff6060"
#define MSG_COLOR_LIGHTBLUE         "|cff00ccff"
#define MSG_COLOR_TORQUISEBLUE      "|cff00C78C"
#define MSG_COLOR_SPRINGGREEN       "|cff00FF7F"
#define MSG_COLOR_GREENYELLOW       "|cffADFF2F"
#define MSG_COLOR_BLUE              "|cff0000ff"
#define MSG_COLOR_PURPLE            "|cffDA70D6"
#define MSG_COLOR_GREEN             "|cff00ff00"
#define MSG_COLOR_RED               "|cffff0000"
#define MSG_COLOR_GOLD              "|cffffcc00"
#define MSG_COLOR_GOLD2             "|cffFFC125"
#define MSG_COLOR_GREY              "|cff888888"
#define MSG_COLOR_WHITE             "|cffffffff"
#define MSG_COLOR_SUBWHITE          "|cffbbbbbb"
#define MSG_COLOR_MAGENTA           "|cffff00ff"
#define MSG_COLOR_YELLOW            "|cffffff00"
#define MSG_COLOR_ORANGEY           "|cffFF4500"
#define MSG_COLOR_CHOCOLATE         "|cffCD661D"
#define MSG_COLOR_CYAN              "|cff00ffff"
#define MSG_COLOR_IVORY             "|cff8B8B83"
#define MSG_COLOR_LIGHTYELLOW       "|cffFFFFE0"
#define MSG_COLOR_SEXGREEN          "|cff71C671"
#define MSG_COLOR_SEXTEAL           "|cff388E8E"
#define MSG_COLOR_SEXPINK           "|cffC67171"
#define MSG_COLOR_SEXBLUE           "|cff00E5EE"
#define MSG_COLOR_SEXHOTPINK        "|cffFF6EB4"
