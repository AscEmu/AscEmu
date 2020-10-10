/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

/// -
#define MAX_ITEM_PROTO_DAMAGES 2

/// -
#define MAX_ITEM_PROTO_SOCKETS 3

/// -
#define MAX_ITEM_PROTO_SPELLS  5

/// -
#define MAX_ITEM_PROTO_STATS  10


/// -
#define MAX_INVENTORY_SLOT 150

/// -
#define MAX_BUYBACK_SLOT 13

/// -
//#define MAX_BUYBACK_SLOT ((PLAYER_FIELD_KEYRING_SLOT_1 - PLAYER_FIELD_VENDORBUYBACK_SLOT_1) >> 1)

/// -
#define ITEM_NO_SLOT_AVAILABLE -1   /// works for all kind of slots now

/// -
#define INVENTORY_SLOT_NOT_SET -1

//\todo verfify this!
#if VERSION_STRING <= TBC
    #define DBC_PLAYER_ITEMS 20
#else
    #define DBC_PLAYER_ITEMS 23 // INVENTORY_SLOT_BAG_END
#endif

/// -
#define RANDOM_SUFFIX_MAGIC_CALCULATION(__suffix, __scale) float2int32(float(__suffix) * float(__scale) / 10000.0f);

/// -
#define EQUIPMENT_SLOT_START        0 // EquipmentSlots
#define EQUIPMENT_SLOT_HEAD         0
#define EQUIPMENT_SLOT_NECK         1
#define EQUIPMENT_SLOT_SHOULDERS    2
#define EQUIPMENT_SLOT_BODY         3
#define EQUIPMENT_SLOT_CHEST        4
#define EQUIPMENT_SLOT_WAIST        5
#define EQUIPMENT_SLOT_LEGS         6
#define EQUIPMENT_SLOT_FEET         7
#define EQUIPMENT_SLOT_WRISTS       8
#define EQUIPMENT_SLOT_HANDS        9
#define EQUIPMENT_SLOT_FINGER1      10
#define EQUIPMENT_SLOT_FINGER2      11
#define EQUIPMENT_SLOT_TRINKET1     12
#define EQUIPMENT_SLOT_TRINKET2     13
#define EQUIPMENT_SLOT_BACK         14
#define EQUIPMENT_SLOT_MAINHAND     15
#define EQUIPMENT_SLOT_OFFHAND      16
#define EQUIPMENT_SLOT_RANGED       17
#define EQUIPMENT_SLOT_TABARD       18
#define EQUIPMENT_SLOT_END          19

/// -
#define INVENTORY_SLOT_BAG_START    19 // InventorySlots
#define INVENTORY_SLOT_BAG_1        19
#define INVENTORY_SLOT_BAG_2        20
#define INVENTORY_SLOT_BAG_3        21
#define INVENTORY_SLOT_BAG_4        22
#define INVENTORY_SLOT_BAG_END      23

/// -
#define INVENTORY_SLOT_ITEM_START   23 // InventoryPackSlots
#define INVENTORY_SLOT_ITEM_1       23
#define INVENTORY_SLOT_ITEM_2       24
#define INVENTORY_SLOT_ITEM_3       25
#define INVENTORY_SLOT_ITEM_4       26
#define INVENTORY_SLOT_ITEM_5       27
#define INVENTORY_SLOT_ITEM_6       28
#define INVENTORY_SLOT_ITEM_7       29
#define INVENTORY_SLOT_ITEM_8       30
#define INVENTORY_SLOT_ITEM_9       31
#define INVENTORY_SLOT_ITEM_10      32
#define INVENTORY_SLOT_ITEM_11      33
#define INVENTORY_SLOT_ITEM_12      34
#define INVENTORY_SLOT_ITEM_13      35
#define INVENTORY_SLOT_ITEM_14      36
#define INVENTORY_SLOT_ITEM_15      37
#define INVENTORY_SLOT_ITEM_16      38
#define INVENTORY_SLOT_ITEM_END     39

/// -
#define BANK_SLOT_ITEM_START        39 // BankItemSlots
#define BANK_SLOT_ITEM_1            39
#define BANK_SLOT_ITEM_2            40
#define BANK_SLOT_ITEM_3            41
#define BANK_SLOT_ITEM_4            42
#define BANK_SLOT_ITEM_5            43
#define BANK_SLOT_ITEM_6            44
#define BANK_SLOT_ITEM_7            45
#define BANK_SLOT_ITEM_8            46
#define BANK_SLOT_ITEM_9            47
#define BANK_SLOT_ITEM_10           48
#define BANK_SLOT_ITEM_11           49
#define BANK_SLOT_ITEM_12           50
#define BANK_SLOT_ITEM_13           51
#define BANK_SLOT_ITEM_14           52
#define BANK_SLOT_ITEM_15           53
#define BANK_SLOT_ITEM_16           54
#define BANK_SLOT_ITEM_17           55
#define BANK_SLOT_ITEM_18           56
#define BANK_SLOT_ITEM_19           57
#define BANK_SLOT_ITEM_20           58
#define BANK_SLOT_ITEM_21           59
#define BANK_SLOT_ITEM_22           60
#define BANK_SLOT_ITEM_23           61
#define BANK_SLOT_ITEM_24           62
#define BANK_SLOT_ITEM_25           63
#define BANK_SLOT_ITEM_26           64
#define BANK_SLOT_ITEM_27           65
#define BANK_SLOT_ITEM_28           66
#define BANK_SLOT_ITEM_END          67

/// -
#define BANK_SLOT_BAG_START         67 // BankBagSlots
#define BANK_SLOT_BAG_1             67
#define BANK_SLOT_BAG_2             68
#define BANK_SLOT_BAG_3             69
#define BANK_SLOT_BAG_4             70
#define BANK_SLOT_BAG_5             71
#define BANK_SLOT_BAG_6             72
#define BANK_SLOT_BAG_7             73
#define BANK_SLOT_BAG_END           74

/// -
#define INVENTORY_KEYRING_START     86 // InventoryKeyRingSlots
#define INVENTORY_KEYRING_1         86
#define INVENTORY_KEYRING_2         87
#define INVENTORY_KEYRING_3         88
#define INVENTORY_KEYRING_4         89
#define INVENTORY_KEYRING_5         90
#define INVENTORY_KEYRING_6         91
#define INVENTORY_KEYRING_7         92
#define INVENTORY_KEYRING_8         93
#define INVENTORY_KEYRING_9         94
#define INVENTORY_KEYRING_10        95
#define INVENTORY_KEYRING_11        96
#define INVENTORY_KEYRING_12        97
#define INVENTORY_KEYRING_13        98
#define INVENTORY_KEYRING_14        99
#define INVENTORY_KEYRING_15        100
#define INVENTORY_KEYRING_16        101
#define INVENTORY_KEYRING_17        102
#define INVENTORY_KEYRING_18        103
#define INVENTORY_KEYRING_19        104
#define INVENTORY_KEYRING_20        105
#define INVENTORY_KEYRING_21        106
#define INVENTORY_KEYRING_22        107
#define INVENTORY_KEYRING_23        108
#define INVENTORY_KEYRING_24        109
#define INVENTORY_KEYRING_25        110
#define INVENTORY_KEYRING_26        111
#define INVENTORY_KEYRING_27        112
#define INVENTORY_KEYRING_28        113
#define INVENTORY_KEYRING_29        114
#define INVENTORY_KEYRING_30        115
#define INVENTORY_KEYRING_31        116
#define INVENTORY_KEYRING_32        117
#define INVENTORY_KEYRING_END       118

/// -
#define CURRENCYTOKEN_SLOT_START    118 // CurrencyTokenSlots
#define CURRENCYTOKEN_SLOT_1        118
#define CURRENCYTOKEN_SLOT_2        119
#define CURRENCYTOKEN_SLOT_3        120
#define CURRENCYTOKEN_SLOT_4        121
#define CURRENCYTOKEN_SLOT_5        122
#define CURRENCYTOKEN_SLOT_6        123
#define CURRENCYTOKEN_SLOT_7        124
#define CURRENCYTOKEN_SLOT_8        125
#define CURRENCYTOKEN_SLOT_9        126
#define CURRENCYTOKEN_SLOT_10       127
#define CURRENCYTOKEN_SLOT_11       128
#define CURRENCYTOKEN_SLOT_12       129
#define CURRENCYTOKEN_SLOT_13       130
#define CURRENCYTOKEN_SLOT_14       131
#define CURRENCYTOKEN_SLOT_15       132
#define CURRENCYTOKEN_SLOT_16       133
#define CURRENCYTOKEN_SLOT_17       134
#define CURRENCYTOKEN_SLOT_18       135
#define CURRENCYTOKEN_SLOT_19       136
#define CURRENCYTOKEN_SLOT_20       137
#define CURRENCYTOKEN_SLOT_21       138
#define CURRENCYTOKEN_SLOT_22       139
#define CURRENCYTOKEN_SLOT_23       140
#define CURRENCYTOKEN_SLOT_24       141
#define CURRENCYTOKEN_SLOT_25       142
#define CURRENCYTOKEN_SLOT_26       143
#define CURRENCYTOKEN_SLOT_27       144
#define CURRENCYTOKEN_SLOT_28       145
#define CURRENCYTOKEN_SLOT_29       146
#define CURRENCYTOKEN_SLOT_30       147
#define CURRENCYTOKEN_SLOT_31       148
#define CURRENCYTOKEN_SLOT_32       149
#define CURRENCYTOKEN_SLOT_END      150
