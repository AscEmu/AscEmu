/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

enum ChannelMemberFlags : uint8_t
{
    CHANNEL_MEMBER_FLAG_NONE            = 0x00,
    CHANNEL_MEMBER_FLAG_OWNER           = 0x01,
    CHANNEL_MEMBER_FLAG_MODERATOR       = 0x02,
    CHANNEL_MEMBER_FLAG_VOICED          = 0x04,
    CHANNEL_MEMBER_FLAG_MUTED           = 0x08,
    CHANNEL_MEMBER_FLAG_CUSTOM          = 0x10,
    CHANNEL_MEMBER_FLAG_MIC_MUTED       = 0x20
};

enum ChannelNotifyFlags : uint8_t
{
    CHANNEL_NOTIFY_FLAG_JOINED          = 0x00,
    CHANNEL_NOTIFY_FLAG_LEFT            = 0x01,
    CHANNEL_NOTIFY_FLAG_YOUJOINED       = 0x02,
    CHANNEL_NOTIFY_FLAG_YOULEFT         = 0x03,
    CHANNEL_NOTIFY_FLAG_WRONGPASS       = 0x04,
    CHANNEL_NOTIFY_FLAG_NOTON           = 0x05,
    CHANNEL_NOTIFY_FLAG_NOTMOD          = 0x06,
    CHANNEL_NOTIFY_FLAG_SETPASS         = 0x07,
    CHANNEL_NOTIFY_FLAG_CHGOWNER        = 0x08,
    CHANNEL_NOTIFY_FLAG_NOT_ON_2        = 0x09,
    CHANNEL_NOTIFY_FLAG_NOT_OWNER       = 0x0A,
    CHANNEL_NOTIFY_FLAG_WHO_OWNER       = 0x0B,
    CHANNEL_NOTIFY_FLAG_MODE_CHG        = 0x0C,
    CHANNEL_NOTIFY_FLAG_ENABLE_ANN      = 0x0D,
    CHANNEL_NOTIFY_FLAG_DISABLE_ANN     = 0x0E,
    CHANNEL_NOTIFY_FLAG_MODERATED       = 0x0F,
    CHANNEL_NOTIFY_FLAG_UNMODERATED     = 0x10,
    CHANNEL_NOTIFY_FLAG_YOUCANTSPEAK    = 0x11,
    CHANNEL_NOTIFY_FLAG_KICKED          = 0x12,
    CHANNEL_NOTIFY_FLAG_YOURBANNED      = 0x13,
    CHANNEL_NOTIFY_FLAG_BANNED          = 0x14,
    CHANNEL_NOTIFY_FLAG_UNBANNED        = 0x15,
    CHANNEL_NOTIFY_FLAG_NOT_BANNED      = 0x16,
    CHANNEL_NOTIFY_FLAG_ALREADY_ON      = 0x17,
    CHANNEL_NOTIFY_FLAG_INVITED         = 0x18,
    CHANNEL_NOTIFY_FLAG_WRONG_FACT      = 0x19,
    CHANNEL_NOTIFY_FLAG_UNK_2           = 0x1A,
    CHANNEL_NOTIFY_FLAG_UNK_3           = 0x1B,
    CHANNEL_NOTIFY_FLAG_UNK_4           = 0x1C,
    CHANNEL_NOTIFY_FLAG_YOU_INVITED     = 0x1D,
    CHANNEL_NOTIFY_FLAG_INVITED_BANNED  = 0x1E,
    CHANNEL_NOTIFY_FLAG_UNK_6           = 0x1F,
    CHANNEL_NOTIFY_FLAG_UNK_7           = 0x20,
    CHANNEL_NOTIFY_FLAG_NOT_IN_LFG      = 0x21,
    CHANNEL_NOTIFY_FLAG_VOICE_ON        = 0x22,
    CHANNEL_NOTIFY_FLAG_VOICE_OFF       = 0x23
};

// ChatChannel.dbc column 1
enum ChannelDBCFlags : uint32_t
{
    CHANNEL_DBC_FLAG_NONE               = 0x000000,
    CHANNEL_DBC_UNK_1                   = 0x000001,
    CHANNEL_DBC_HAS_ZONENAME            = 0x000002,
    CHANNEL_DBC_GLOBAL                  = 0x000004,
    CHANNEL_DBC_TRADE                   = 0x000008,
    CHANNEL_DBC_CITY_ONLY_1             = 0x000010,
    CHANNEL_DBC_CITY_ONLY_2             = 0x000020,     // 2 identical columns, who knows?
    CHANNEL_DBC_UNUSED_1                = 0x000040,
    CHANNEL_DBC_UNUSED_2                = 0x000080,
    CHANNEL_DBC_UNUSED_3                = 0x000100,
    CHANNEL_DBC_UNUSED_4                = 0x000200,
    CHANNEL_DBC_UNUSED_5                = 0x000400,
    CHANNEL_DBC_UNUSED_6                = 0x000800,
    CHANNEL_DBC_UNUSED_7                = 0x001000,
    CHANNEL_DBC_UNUSED_8                = 0x002000,
    CHANNEL_DBC_UNUSED_9                = 0x004000,
    CHANNEL_DBC_UNUSED_10               = 0x008000,
    CHANNEL_DBC_DEFENSE                 = 0x010000,
    CHANNEL_DBC_GUILD_RECRUIT           = 0x020000,
    CHANNEL_DBC_LFG                     = 0x040000,
    CHANNEL_DBC_UNUSED_11               = 0x080000,
    CHANNEL_DBC_UNUSED_12               = 0x100000,
    CHANNEL_DBC_UNK_2                   = 0x200000
};

enum ChannelFlags : uint8_t
{
    CHANNEL_FLAGS_NONE                  = 0x00,
    CHANNEL_FLAGS_CUSTOM                = 0x01,     // Channels created by players
    CHANNEL_FLAGS_UNK1                  = 0x02,     // not seen yet, perhaps related to worlddefense
    CHANNEL_FLAGS_TRADE                 = 0x04,
    CHANNEL_FLAGS_NOT_LFG               = 0x08,
    CHANNEL_FLAGS_GENERAL               = 0x10,
    CHANNEL_FLAGS_CITY                  = 0x20,
    CHANNEL_FLAGS_LFG                   = 0x40,
    CHANNEL_FLAGS_VOICE                 = 0x80
};
