/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Generated for version 1.12.1 5875

enum EObjectFields
{
    OBJECT_FIELD_GUID                                = 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    OBJECT_FIELD_TYPE                                = 0x0002, // Size: 1, Type: INT, Flags: PUBLIC
    OBJECT_FIELD_ENTRY                               = 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
    OBJECT_FIELD_SCALE_X                             = 0x0004, // Size: 1, Type: FLOAT, Flags: PUBLIC
    OBJECT_FIELD_PADDING                             = 0x0005, // Size: 1, Type: INT, Flags: NONE
    OBJECT_END                                       = 0x0006,
};

enum EItemFields
{
    ITEM_FIELD_OWNER                                 = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_CONTAINED                             = OBJECT_END + 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_CREATOR                               = OBJECT_END + 0x0004, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_GIFTCREATOR                           = OBJECT_END + 0x0006, // Size: 2, Type: LONG, Flags: PUBLIC
    ITEM_FIELD_STACK_COUNT                           = OBJECT_END + 0x0008, // Size: 1, Type: INT, Flags: OWNER_ONLY, UNK2
    ITEM_FIELD_DURATION                              = OBJECT_END + 0x0009, // Size: 1, Type: INT, Flags: OWNER_ONLY, UNK2
    ITEM_FIELD_SPELL_CHARGES                         = OBJECT_END + 0x000A, // Size: 5, Type: INT, Flags: OWNER_ONLY, UNK2
    ITEM_FIELD_FLAGS                                 = OBJECT_END + 0x000F, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ENCHANTMENT                           = OBJECT_END + 0x0010, // Size: 21, Type: INT, Flags: PUBLIC
    ITEM_FIELD_PROPERTY_SEED                         = OBJECT_END + 0x0025, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_RANDOM_PROPERTIES_ID                  = OBJECT_END + 0x0026, // Size: 1, Type: INT, Flags: PUBLIC
    ITEM_FIELD_ITEM_TEXT_ID                          = OBJECT_END + 0x0027, // Size: 1, Type: INT, Flags: OWNER_ONLY
    ITEM_FIELD_DURABILITY                            = OBJECT_END + 0x0028, // Size: 1, Type: INT, Flags: OWNER_ONLY, UNK2
    ITEM_FIELD_MAXDURABILITY                         = OBJECT_END + 0x0029, // Size: 1, Type: INT, Flags: OWNER_ONLY, UNK2
    ITEM_END                                         = OBJECT_END + 0x002A,
};

enum EContainerFields
{
    CONTAINER_FIELD_NUM_SLOTS                        = ITEM_END + 0x0000, // Size: 1, Type: INT, Flags: PUBLIC
    CONTAINER_ALIGN_PAD                              = ITEM_END + 0x0001, // Size: 1, Type: BYTES, Flags: NONE
    CONTAINER_FIELD_SLOT_1                           = ITEM_END + 0x0002, // Size: 58, Type: LONG, Flags: PUBLIC
    CONTAINER_END                                    = ITEM_END + 0x003A,
};

enum EUnitFields
{
    UNIT_FIELD_CHARM                                 = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_SUMMON                                = OBJECT_END + 0x0002, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_CHARMEDBY                             = OBJECT_END + 0x0004, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_SUMMONEDBY                            = OBJECT_END + 0x0006, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_CREATEDBY                             = OBJECT_END + 0x0008, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_TARGET                                = OBJECT_END + 0x000A, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_PERSUADED                             = OBJECT_END + 0x000C, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_CHANNEL_OBJECT                        = OBJECT_END + 0x000E, // Size: 2, Type: LONG, Flags: PUBLIC
    UNIT_FIELD_HEALTH                                = OBJECT_END + 0x0010, // Size: 1, Type: INT, Flags: DYNAMIC
    UNIT_FIELD_POWER1                                = OBJECT_END + 0x0011, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWER2                                = OBJECT_END + 0x0012, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWER3                                = OBJECT_END + 0x0013, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWER4                                = OBJECT_END + 0x0014, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_POWER5                                = OBJECT_END + 0x0015, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXHEALTH                             = OBJECT_END + 0x0016, // Size: 1, Type: INT, Flags: DYNAMIC
    UNIT_FIELD_MAXPOWER1                             = OBJECT_END + 0x0017, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXPOWER2                             = OBJECT_END + 0x0018, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXPOWER3                             = OBJECT_END + 0x0019, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXPOWER4                             = OBJECT_END + 0x001A, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MAXPOWER5                             = OBJECT_END + 0x001B, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_LEVEL                                 = OBJECT_END + 0x001C, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_FACTIONTEMPLATE                       = OBJECT_END + 0x001D, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_BYTES_0                               = OBJECT_END + 0x001E, // Size: 1, Type: BYTES, Flags: PUBLIC
    UNIT_VIRTUAL_ITEM_SLOT_ID                        = OBJECT_END + 0x001F, // Size: 3, Type: INT, Flags: PUBLIC
    UNIT_VIRTUAL_ITEM_INFO                           = OBJECT_END + 0x0022, // Size: 6, Type: BYTES, Flags: PUBLIC
    UNIT_FIELD_FLAGS                                 = OBJECT_END + 0x0028, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_AURA                                  = OBJECT_END + 0x0029, // Size: 49, Type: INT, Flags: PUBLIC
    UNIT_FIELD_AURAFLAGS                             = OBJECT_END + 0x0059, // Size: 6, Type: BYTES, Flags: PUBLIC
    UNIT_FIELD_AURALEVELS                            = OBJECT_END + 0x005F, // Size: 12, Type: BYTES, Flags: PUBLIC
    UNIT_FIELD_AURAAPPLICATIONS                      = OBJECT_END + 0x006B, // Size: 12, Type: BYTES, Flags: PUBLIC
    UNIT_FIELD_AURASTATE                             = OBJECT_END + 0x0077, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_BASEATTACKTIME                        = OBJECT_END + 0x0078, // Size: 2, Type: INT, Flags: PUBLIC
    UNIT_FIELD_RANGEDATTACKTIME                      = OBJECT_END + 0x007A, // Size: 1, Type: INT, Flags: PRIVATE
    UNIT_FIELD_BOUNDINGRADIUS                        = OBJECT_END + 0x007B, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_FIELD_COMBATREACH                           = OBJECT_END + 0x007C, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_FIELD_DISPLAYID                             = OBJECT_END + 0x007D, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_NATIVEDISPLAYID                       = OBJECT_END + 0x007E, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MOUNTDISPLAYID                        = OBJECT_END + 0x007F, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_MINDAMAGE                             = OBJECT_END + 0x0080, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER_ONLY, UNK3
    UNIT_FIELD_MAXDAMAGE                             = OBJECT_END + 0x0081, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER_ONLY, UNK3
    UNIT_FIELD_MINOFFHANDDAMAGE                      = OBJECT_END + 0x0082, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER_ONLY, UNK3
    UNIT_FIELD_MAXOFFHANDDAMAGE                      = OBJECT_END + 0x0083, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER_ONLY, UNK3
    UNIT_FIELD_BYTES_1                               = OBJECT_END + 0x0084, // Size: 1, Type: BYTES, Flags: PUBLIC
    UNIT_FIELD_PETNUMBER                             = OBJECT_END + 0x0085, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_PET_NAME_TIMESTAMP                    = OBJECT_END + 0x0086, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_PETEXPERIENCE                         = OBJECT_END + 0x0087, // Size: 1, Type: INT, Flags: OWNER_ONLY
    UNIT_FIELD_PETNEXTLEVELEXP                       = OBJECT_END + 0x0088, // Size: 1, Type: INT, Flags: OWNER_ONLY
    UNIT_DYNAMIC_FLAGS                               = OBJECT_END + 0x0089, // Size: 1, Type: INT, Flags: DYNAMIC
    UNIT_CHANNEL_SPELL                               = OBJECT_END + 0x008A, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_MOD_CAST_SPEED                              = OBJECT_END + 0x008B, // Size: 1, Type: FLOAT, Flags: PUBLIC
    UNIT_CREATED_BY_SPELL                            = OBJECT_END + 0x008C, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_NPC_FLAGS                                   = OBJECT_END + 0x008D, // Size: 1, Type: INT, Flags: DYNAMIC
    UNIT_NPC_EMOTESTATE                              = OBJECT_END + 0x008E, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_TRAINING_POINTS                             = OBJECT_END + 0x008F, // Size: 1, Type: TWO_SHORT, Flags: OWNER_ONLY
    UNIT_FIELD_STAT0                                 = OBJECT_END + 0x0090, // Size: 1, Type: INT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_STAT1                                 = OBJECT_END + 0x0091, // Size: 1, Type: INT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_STAT2                                 = OBJECT_END + 0x0092, // Size: 1, Type: INT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_STAT3                                 = OBJECT_END + 0x0093, // Size: 1, Type: INT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_STAT4                                 = OBJECT_END + 0x0094, // Size: 1, Type: INT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_RESISTANCES                           = OBJECT_END + 0x0095, // Size: 7, Type: INT, Flags: PRIVATE, OWNER_ONLY, UNK3
    UNIT_FIELD_BASE_MANA                             = OBJECT_END + 0x009C, // Size: 1, Type: INT, Flags: PUBLIC
    UNIT_FIELD_BASE_HEALTH                           = OBJECT_END + 0x009D, // Size: 1, Type: INT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_BYTES_2                               = OBJECT_END + 0x009E, // Size: 1, Type: BYTES, Flags: PUBLIC
    UNIT_FIELD_ATTACK_POWER                          = OBJECT_END + 0x009F, // Size: 1, Type: INT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_ATTACK_POWER_MODS                     = OBJECT_END + 0x00A0, // Size: 1, Type: TWO_SHORT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_ATTACK_POWER_MULTIPLIER               = OBJECT_END + 0x00A1, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_RANGED_ATTACK_POWER                   = OBJECT_END + 0x00A2, // Size: 1, Type: INT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_RANGED_ATTACK_POWER_MODS              = OBJECT_END + 0x00A3, // Size: 1, Type: TWO_SHORT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER        = OBJECT_END + 0x00A4, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_MINRANGEDDAMAGE                       = OBJECT_END + 0x00A5, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_MAXRANGEDDAMAGE                       = OBJECT_END + 0x00A6, // Size: 1, Type: FLOAT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_POWER_COST_MODIFIER                   = OBJECT_END + 0x00A7, // Size: 7, Type: INT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_POWER_COST_MULTIPLIER                 = OBJECT_END + 0x00AE, // Size: 7, Type: FLOAT, Flags: PRIVATE, OWNER_ONLY
    UNIT_FIELD_PADDING                               = OBJECT_END + 0x00B5, // Size: 1, Type: INT, Flags: NONE
    UNIT_END                                         = OBJECT_END + 0x00B6,
};

enum EPlayerFields
{
    PLAYER_DUEL_ARBITER                              = UNIT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_FLAGS                                     = UNIT_END + 0x0002, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_GUILDID                                   = UNIT_END + 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_GUILDRANK                                 = UNIT_END + 0x0004, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_BYTES                                     = UNIT_END + 0x0005, // Size: 1, Type: BYTES, Flags: PUBLIC
    PLAYER_BYTES_2                                   = UNIT_END + 0x0006, // Size: 1, Type: BYTES, Flags: PUBLIC
    PLAYER_BYTES_3                                   = UNIT_END + 0x0007, // Size: 1, Type: BYTES, Flags: PUBLIC
    PLAYER_DUEL_TEAM                                 = UNIT_END + 0x0008, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_GUILD_TIMESTAMP                           = UNIT_END + 0x0009, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_QUEST_LOG_1_1                             = UNIT_END + 0x000A, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_1_2                             = UNIT_END + 0x000B, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_1_3                             = UNIT_END + 0x000C, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_1_4                             = UNIT_END + 0x000D, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_2_1                             = UNIT_END + 0x000E, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_2_2                             = UNIT_END + 0x000F, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_2_3                             = UNIT_END + 0x0010, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_2_4                             = UNIT_END + 0x0011, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_3_1                             = UNIT_END + 0x0012, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_3_2                             = UNIT_END + 0x0013, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_3_3                             = UNIT_END + 0x0014, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_3_4                             = UNIT_END + 0x0015, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_4_1                             = UNIT_END + 0x0016, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_4_2                             = UNIT_END + 0x0017, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_4_3                             = UNIT_END + 0x0018, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_4_4                             = UNIT_END + 0x0019, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_5_1                             = UNIT_END + 0x001A, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_5_2                             = UNIT_END + 0x001B, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_5_3                             = UNIT_END + 0x001C, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_5_4                             = UNIT_END + 0x001D, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_6_1                             = UNIT_END + 0x001E, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_6_2                             = UNIT_END + 0x001F, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_6_3                             = UNIT_END + 0x0020, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_6_4                             = UNIT_END + 0x0021, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_7_1                             = UNIT_END + 0x0022, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_7_2                             = UNIT_END + 0x0023, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_7_3                             = UNIT_END + 0x0024, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_7_4                             = UNIT_END + 0x0025, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_8_1                             = UNIT_END + 0x0026, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_8_2                             = UNIT_END + 0x0027, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_8_3                             = UNIT_END + 0x0028, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_8_4                             = UNIT_END + 0x0029, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_9_1                             = UNIT_END + 0x002A, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_9_2                             = UNIT_END + 0x002B, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_9_3                             = UNIT_END + 0x002C, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_9_4                             = UNIT_END + 0x002D, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_10_1                            = UNIT_END + 0x002E, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_10_2                            = UNIT_END + 0x002F, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_10_3                            = UNIT_END + 0x0030, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_10_4                            = UNIT_END + 0x0031, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_11_1                            = UNIT_END + 0x0032, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_11_2                            = UNIT_END + 0x0033, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_11_3                            = UNIT_END + 0x0034, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_11_4                            = UNIT_END + 0x0035, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_12_1                            = UNIT_END + 0x0036, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_12_2                            = UNIT_END + 0x0037, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_12_3                            = UNIT_END + 0x0038, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_12_4                            = UNIT_END + 0x0039, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_13_1                            = UNIT_END + 0x003A, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_13_2                            = UNIT_END + 0x003B, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_13_3                            = UNIT_END + 0x003C, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_13_4                            = UNIT_END + 0x003D, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_14_1                            = UNIT_END + 0x003E, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_14_2                            = UNIT_END + 0x003F, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_14_3                            = UNIT_END + 0x0040, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_14_4                            = UNIT_END + 0x0041, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_15_1                            = UNIT_END + 0x0042, // Size: 1, Type: INT, Flags: GROUP_ONLY
    PLAYER_QUEST_LOG_15_2                            = UNIT_END + 0x0043, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_QUEST_LOG_15_3                            = UNIT_END + 0x0044, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_QUEST_LOG_15_4                            = UNIT_END + 0x0045, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_VISIBLE_ITEM_1_CREATOR                    = UNIT_END + 0x0046, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_1_0                          = UNIT_END + 0x0048, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_1_PROPERTIES                 = UNIT_END + 0x0050, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_1_PAD                        = UNIT_END + 0x0051, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_2_CREATOR                    = UNIT_END + 0x0052, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_2_0                          = UNIT_END + 0x0054, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_2_PROPERTIES                 = UNIT_END + 0x005C, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_2_PAD                        = UNIT_END + 0x005D, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_3_CREATOR                    = UNIT_END + 0x005E, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_3_0                          = UNIT_END + 0x0060, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_3_PROPERTIES                 = UNIT_END + 0x0068, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_3_PAD                        = UNIT_END + 0x0069, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_4_CREATOR                    = UNIT_END + 0x006A, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_4_0                          = UNIT_END + 0x006C, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_4_PROPERTIES                 = UNIT_END + 0x0074, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_4_PAD                        = UNIT_END + 0x0075, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_5_CREATOR                    = UNIT_END + 0x0076, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_5_0                          = UNIT_END + 0x0078, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_5_PROPERTIES                 = UNIT_END + 0x0080, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_5_PAD                        = UNIT_END + 0x0081, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_6_CREATOR                    = UNIT_END + 0x0082, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_6_0                          = UNIT_END + 0x0084, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_6_PROPERTIES                 = UNIT_END + 0x008C, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_6_PAD                        = UNIT_END + 0x008D, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_7_CREATOR                    = UNIT_END + 0x008E, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_7_0                          = UNIT_END + 0x0090, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_7_PROPERTIES                 = UNIT_END + 0x0098, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_7_PAD                        = UNIT_END + 0x0099, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_8_CREATOR                    = UNIT_END + 0x009A, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_8_0                          = UNIT_END + 0x009C, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_8_PROPERTIES                 = UNIT_END + 0x00A4, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_8_PAD                        = UNIT_END + 0x00A5, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_9_CREATOR                    = UNIT_END + 0x00A6, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_9_0                          = UNIT_END + 0x00A8, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_9_PROPERTIES                 = UNIT_END + 0x00B0, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_9_PAD                        = UNIT_END + 0x00B1, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_10_CREATOR                   = UNIT_END + 0x00B2, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_10_0                         = UNIT_END + 0x00B4, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_10_PROPERTIES                = UNIT_END + 0x00BC, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_10_PAD                       = UNIT_END + 0x00BD, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_11_CREATOR                   = UNIT_END + 0x00BE, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_11_0                         = UNIT_END + 0x00C0, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_11_PROPERTIES                = UNIT_END + 0x00C8, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_11_PAD                       = UNIT_END + 0x00C9, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_12_CREATOR                   = UNIT_END + 0x00CA, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_12_0                         = UNIT_END + 0x00CC, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_12_PROPERTIES                = UNIT_END + 0x00D4, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_12_PAD                       = UNIT_END + 0x00D5, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_13_CREATOR                   = UNIT_END + 0x00D6, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_13_0                         = UNIT_END + 0x00D8, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_13_PROPERTIES                = UNIT_END + 0x00E0, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_13_PAD                       = UNIT_END + 0x00E1, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_14_CREATOR                   = UNIT_END + 0x00E2, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_14_0                         = UNIT_END + 0x00E4, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_14_PROPERTIES                = UNIT_END + 0x00EC, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_14_PAD                       = UNIT_END + 0x00ED, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_15_CREATOR                   = UNIT_END + 0x00EE, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_15_0                         = UNIT_END + 0x00F0, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_15_PROPERTIES                = UNIT_END + 0x00F8, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_15_PAD                       = UNIT_END + 0x00F9, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_16_CREATOR                   = UNIT_END + 0x00FA, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_16_0                         = UNIT_END + 0x00FC, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_16_PROPERTIES                = UNIT_END + 0x0104, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_16_PAD                       = UNIT_END + 0x0105, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_17_CREATOR                   = UNIT_END + 0x0106, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_17_0                         = UNIT_END + 0x0108, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_17_PROPERTIES                = UNIT_END + 0x0110, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_17_PAD                       = UNIT_END + 0x0111, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_18_CREATOR                   = UNIT_END + 0x0112, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_18_0                         = UNIT_END + 0x0114, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_18_PROPERTIES                = UNIT_END + 0x011C, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_18_PAD                       = UNIT_END + 0x011D, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_19_CREATOR                   = UNIT_END + 0x011E, // Size: 2, Type: LONG, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_19_0                         = UNIT_END + 0x0120, // Size: 8, Type: INT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_19_PROPERTIES                = UNIT_END + 0x0128, // Size: 1, Type: TWO_SHORT, Flags: PUBLIC
    PLAYER_VISIBLE_ITEM_19_PAD                       = UNIT_END + 0x0129, // Size: 1, Type: INT, Flags: PUBLIC
    PLAYER_FIELD_INV_SLOT_HEAD                       = UNIT_END + 0x012A, // Size: 46, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_PACK_SLOT_1                         = UNIT_END + 0x0158, // Size: 32, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_BANK_SLOT_1                         = UNIT_END + 0x0178, // Size: 48, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_BANKBAG_SLOT_1                      = UNIT_END + 0x01A8, // Size: 12, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_VENDORBUYBACK_SLOT_1                = UNIT_END + 0x01B4, // Size: 24, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_KEYRING_SLOT_1                      = UNIT_END + 0x01CC, // Size: 64, Type: LONG, Flags: PRIVATE
    PLAYER_FARSIGHT                                  = UNIT_END + 0x020C, // Size: 2, Type: LONG, Flags: PRIVATE
    PLAYER_FIELD_KNOWN_TITLES                        = UNIT_END + 0x020E, // Size: 2, Type: LONG, Flags: PRIVATE
    PLAYER_XP                                        = UNIT_END + 0x0210, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_NEXT_LEVEL_XP                             = UNIT_END + 0x0211, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_SKILL_INFO_1_1                            = UNIT_END + 0x0212, // Size: 384, Type: TWO_SHORT, Flags: PRIVATE
    PLAYER_CHARACTER_POINTS1                         = UNIT_END + 0x0392, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_CHARACTER_POINTS2                         = UNIT_END + 0x0393, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_TRACK_CREATURES                           = UNIT_END + 0x0394, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_TRACK_RESOURCES                           = UNIT_END + 0x0395, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_BLOCK_PERCENTAGE                          = UNIT_END + 0x0396, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_DODGE_PERCENTAGE                          = UNIT_END + 0x0397, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_PARRY_PERCENTAGE                          = UNIT_END + 0x0398, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_CRIT_PERCENTAGE                           = UNIT_END + 0x0399, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_RANGED_CRIT_PERCENTAGE                    = UNIT_END + 0x039A, // Size: 1, Type: FLOAT, Flags: PRIVATE
    PLAYER_EXPLORED_ZONES_1                          = UNIT_END + 0x039B, // Size: 64, Type: BYTES, Flags: PRIVATE
    PLAYER_REST_STATE_EXPERIENCE                     = UNIT_END + 0x03DB, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_COINAGE                             = UNIT_END + 0x03DC, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_POSSTAT0                            = UNIT_END + 0x03DD, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_POSSTAT1                            = UNIT_END + 0x03DE, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_POSSTAT2                            = UNIT_END + 0x03DF, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_POSSTAT3                            = UNIT_END + 0x03E0, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_POSSTAT4                            = UNIT_END + 0x03E1, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_NEGSTAT0                            = UNIT_END + 0x03E2, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_NEGSTAT1                            = UNIT_END + 0x03E3, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_NEGSTAT2                            = UNIT_END + 0x03E4, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_NEGSTAT3                            = UNIT_END + 0x03E5, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_NEGSTAT4                            = UNIT_END + 0x03E6, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE          = UNIT_END + 0x03E7, // Size: 7, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE          = UNIT_END + 0x03EE, // Size: 7, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_DAMAGE_DONE_POS                 = UNIT_END + 0x03F5, // Size: 7, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_DAMAGE_DONE_NEG                 = UNIT_END + 0x03FC, // Size: 7, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_MOD_DAMAGE_DONE_PCT                 = UNIT_END + 0x0403, // Size: 7, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BYTES                               = UNIT_END + 0x040A, // Size: 1, Type: BYTES, Flags: PRIVATE
    PLAYER_AMMO_ID                                   = UNIT_END + 0x040B, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_SELF_RES_SPELL                            = UNIT_END + 0x040C, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_PVP_MEDALS                          = UNIT_END + 0x040D, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BUYBACK_PRICE_1                     = UNIT_END + 0x040E, // Size: 12, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BUYBACK_TIMESTAMP_1                 = UNIT_END + 0x041A, // Size: 12, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_SESSION_KILLS                       = UNIT_END + 0x0426, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_YESTERDAY_KILLS                     = UNIT_END + 0x0427, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_LAST_WEEK_KILLS                     = UNIT_END + 0x0428, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_THIS_WEEK_KILLS                     = UNIT_END + 0x0429, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_THIS_WEEK_CONTRIBUTION              = UNIT_END + 0x042a, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_LIFETIME_HONORABLE_KILLS            = UNIT_END + 0x042b, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_LIFETIME_DISHONORABLE_KILLS         = UNIT_END + 0x042c, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_YESTERDAY_CONTRIBUTION              = UNIT_END + 0x042d, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_LAST_WEEK_CONTRIBUTION              = UNIT_END + 0x042e, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_LAST_WEEK_RANK                      = UNIT_END + 0x042f, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_BYTES2                              = UNIT_END + 0x0430, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_WATCHED_FACTION_INDEX               = UNIT_END + 0x0431, // Size: 1, Type: INT, Flags: PRIVATE
    PLAYER_FIELD_COMBAT_RATING_1                     = UNIT_END + 0x0432, // Size: 20, Type: INT, Flags: PRIVATE
    PLAYER_END                                       = UNIT_END + 0x0446,
};

enum EGameObjectFields
{
    OBJECT_FIELD_CREATED_BY                          = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    GAMEOBJECT_DISPLAYID                             = OBJECT_END + 0x0002, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_FLAGS                                 = OBJECT_END + 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_PARENTROTATION                        = OBJECT_END + 0x0004, // Size: 4, Type: FLOAT, Flags: PUBLIC
    GAMEOBJECT_STATE                                 = OBJECT_END + 0x0008, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_POS_X                                 = OBJECT_END + 0x0009, // Size: 1, Type: FLOAT, Flags: PUBLIC
    GAMEOBJECT_POS_Y                                 = OBJECT_END + 0x000A, // Size: 1, Type: FLOAT, Flags: PUBLIC
    GAMEOBJECT_POS_Z                                 = OBJECT_END + 0x000B, // Size: 1, Type: FLOAT, Flags: PUBLIC
    GAMEOBJECT_FACING                                = OBJECT_END + 0x000C, // Size: 1, Type: FLOAT, Flags: PUBLIC
    GAMEOBJECT_DYNAMIC                               = OBJECT_END + 0x000D, // Size: 1, Type: INT, Flags: DYNAMIC
    GAMEOBJECT_FACTION                               = OBJECT_END + 0x000E, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_TYPE_ID                               = OBJECT_END + 0x000F, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_LEVEL                                 = OBJECT_END + 0x0010, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_ARTKIT                                = OBJECT_END + 0x0011, // Size: 1, Type: INT, Flags: PUBLIC
    GAMEOBJECT_ANIMPROGRESS                          = OBJECT_END + 0x0012, // Size: 1, Type: INT, Flags: DYNAMIC
    GAMEOBJECT_PADDING                               = OBJECT_END + 0x0013, // Size: 1, Type: INT, Flags: NONE
    GAMEOBJECT_END                                   = OBJECT_END + 0x0014,
};

enum EDynamicObjectFields
{
    DYNAMICOBJECT_CASTER                             = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    DYNAMICOBJECT_BYTES                              = OBJECT_END + 0x0002, // Size: 1, Type: BYTES, Flags: PUBLIC
    DYNAMICOBJECT_SPELLID                            = OBJECT_END + 0x0003, // Size: 1, Type: INT, Flags: PUBLIC
    DYNAMICOBJECT_RADIUS                             = OBJECT_END + 0x0004, // Size: 1, Type: FLOAT, Flags: PUBLIC
    DYNAMICOBJECT_POS_X                              = OBJECT_END + 0x0005, // Size: 1, Type: FLOAT, Flags: PUBLIC
    DYNAMICOBJECT_POS_Y                              = OBJECT_END + 0x0006, // Size: 1, Type: FLOAT, Flags: PUBLIC
    DYNAMICOBJECT_POS_Z                              = OBJECT_END + 0x0007, // Size: 1, Type: FLOAT, Flags: PUBLIC
    DYNAMICOBJECT_FACING                             = OBJECT_END + 0x0008, // Size: 1, Type: FLOAT, Flags: PUBLIC
    DYNAMICOBJECT_CASTTIME                           = OBJECT_END + 0x0009, // Size: 1, Type: INT, Flags: PUBLIC
    DYNAMICOBJECT_END                                = OBJECT_END + 0x000A,
};

enum ECorpseFields
{
    CORPSE_FIELD_OWNER                               = OBJECT_END + 0x0000, // Size: 2, Type: LONG, Flags: PUBLIC
    CORPSE_FIELD_FACING                              = OBJECT_END + 0x0002, // Size: 1, Type: FLOAT, Flags: PUBLIC
    CORPSE_FIELD_POS_X                               = OBJECT_END + 0x0003, // Size: 1, Type: FLOAT, Flags: PUBLIC
    CORPSE_FIELD_POS_Y                               = OBJECT_END + 0x0004, // Size: 1, Type: FLOAT, Flags: PUBLIC
    CORPSE_FIELD_POS_Z                               = OBJECT_END + 0x0005, // Size: 1, Type: FLOAT, Flags: PUBLIC
    CORPSE_FIELD_DISPLAY_ID                          = OBJECT_END + 0x0006, // Size: 1, Type: INT, Flags: PUBLIC
    CORPSE_FIELD_ITEM                                = OBJECT_END + 0x0007, // Size: 19, Type: INT, Flags: PUBLIC
    CORPSE_FIELD_BYTES_1                             = OBJECT_END + 0x001A, // Size: 1, Type: BYTES, Flags: PUBLIC
    CORPSE_FIELD_BYTES_2                             = OBJECT_END + 0x001B, // Size: 1, Type: BYTES, Flags: PUBLIC
    CORPSE_FIELD_GUILD                               = OBJECT_END + 0x001C, // Size: 1, Type: INT, Flags: PUBLIC
    CORPSE_FIELD_FLAGS                               = OBJECT_END + 0x001D, // Size: 1, Type: INT, Flags: PUBLIC
    CORPSE_FIELD_DYNAMIC_FLAGS                       = OBJECT_END + 0x001E, // Size: 1, Type: INT, Flags: DYNAMIC
    CORPSE_FIELD_PAD                                 = OBJECT_END + 0x001F, // Size: 1, Type: INT, Flags: NONE
    CORPSE_END                                       = OBJECT_END + 0x0020,
};
