/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
#define ITEM_NO_SLOT_AVAILABLE -1   /// works for all kind of slots now

/// -
#define INVENTORY_SLOT_NOT_SET -1

/// Count of visible items (equippable items + bags)
/// i.e arrow quivers are also visible on character screen
#define DBC_PLAYER_ITEMS 23 // INVENTORY_SLOT_BAG_END

/// -
#define RANDOM_SUFFIX_MAGIC_CALCULATION(__suffix, __scale) Util::float2int32(float(__suffix) * float(__scale) / 10000.0f);

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
