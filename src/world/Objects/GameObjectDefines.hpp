/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once


struct TransportAnimation;

enum LootState : uint8_t
{
    GO_NOT_READY                = 0,
    GO_READY                    = 1,
    GO_ACTIVATED                = 2,
    GO_JUST_DEACTIVATED         = 3
};

enum GameObjectFlags : uint32_t
{
    GO_FLAG_NONE = 0x00000000,
    GO_FLAG_NONSELECTABLE = 0x00000001,   // Object is currently in use / animates
    GO_FLAG_LOCKED = 0x00000002,   // Requires key, spell, or event to open
    GO_FLAG_UNTARGETABLE = 0x00000004,   // Cannot be targeted / conditional interaction
    GO_FLAG_TRANSPORT = 0x00000008,   // Object can transport units (elevator/boat)
    GO_FLAG_NOT_SELECTABLE = 0x00000010,   // Not selectable at all (even for GMs without flag removal)
    GO_FLAG_NEVER_DESPAWN = 0x00000020,   // Never despawns automatically
    GO_FLAG_TRIGGERED = 0x00000040,   // Summoned / triggered by spell or script
    GO_FLAG_FREEZE_ANIMATION = 0x00000080,
    GO_FLAG_UNK1 = 0x00000080,   // Legacy alias
    GO_FLAG_UNK2 = 0x00000100,
    GO_FLAG_DAMAGED = 0x00000200,   // Siege damage taken
    GO_FLAG_DESTROYED = 0x00000400,   // Siege destroyed
    GO_FLAG_MAP_OBJECT = 0x00100000    // 7.0+: Model is a WMO rather than an M2
};

enum GameObjectDestructibleState : uint8_t
{
    GO_DESTRUCTIBLE_INTACT      = 0,
    GO_DESTRUCTIBLE_DAMAGED     = 1,
    GO_DESTRUCTIBLE_DESTROYED   = 2,
    GO_DESTRUCTIBLE_REBUILDING  = 3
};

// TODO: move these under proper Gameobject subclasses
struct GameObjectValue
{
    //11 GAMEOBJECT_TYPE_TRANSPORT
    uint32_t PathProgress = 0;
    TransportAnimation const* AnimationInfo = nullptr;
    uint32_t CurrentSeg = 0;
};

enum GameObjectTypes
{
    GAMEOBJECT_TYPE_DOOR                    = 0,
    GAMEOBJECT_TYPE_BUTTON                  = 1,
    GAMEOBJECT_TYPE_QUESTGIVER              = 2,
    GAMEOBJECT_TYPE_CHEST                   = 3,
    GAMEOBJECT_TYPE_BINDER                  = 4,
    GAMEOBJECT_TYPE_GENERIC                 = 5,
    GAMEOBJECT_TYPE_TRAP                    = 6,
    GAMEOBJECT_TYPE_CHAIR                   = 7,
    GAMEOBJECT_TYPE_SPELL_FOCUS             = 8,
    GAMEOBJECT_TYPE_TEXT                    = 9,
    GAMEOBJECT_TYPE_GOOBER                  = 10,
    GAMEOBJECT_TYPE_TRANSPORT               = 11,
    GAMEOBJECT_TYPE_AREADAMAGE              = 12,
    GAMEOBJECT_TYPE_CAMERA                  = 13,
    GAMEOBJECT_TYPE_MAP_OBJECT              = 14,
    GAMEOBJECT_TYPE_MO_TRANSPORT            = 15,
    GAMEOBJECT_TYPE_DUEL_ARBITER            = 16,
    GAMEOBJECT_TYPE_FISHINGNODE             = 17,
    GAMEOBJECT_TYPE_RITUAL                  = 18,
    GAMEOBJECT_TYPE_MAILBOX                 = 19,
    GAMEOBJECT_TYPE_AUCTIONHOUSE            = 20,
    GAMEOBJECT_TYPE_GUARDPOST               = 21,
    GAMEOBJECT_TYPE_SPELLCASTER             = 22,
    GAMEOBJECT_TYPE_MEETINGSTONE            = 23,
    GAMEOBJECT_TYPE_FLAGSTAND               = 24,
    GAMEOBJECT_TYPE_FISHINGHOLE             = 25,
    GAMEOBJECT_TYPE_FLAGDROP                = 26,
    GAMEOBJECT_TYPE_MINI_GAME               = 27,
    GAMEOBJECT_TYPE_LOTTERY_KIOSK           = 28,
    GAMEOBJECT_TYPE_CAPTURE_POINT           = 29,
    GAMEOBJECT_TYPE_AURA_GENERATOR          = 30,
    GAMEOBJECT_TYPE_DUNGEON_DIFFICULTY      = 31,
    GAMEOBJECT_TYPE_BARBER_CHAIR            = 32,
    GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING   = 33,
    GAMEOBJECT_TYPE_GUILD_BANK              = 34,
    GAMEOBJECT_TYPE_TRAPDOOR                = 35,

    // MoP+
    GAMEOBJECT_TYPE_NEW_FLAG = 36,
    GAMEOBJECT_TYPE_NEW_FLAG_DROP = 37,

    // WoD+
    GAMEOBJECT_TYPE_GARRISON_BUILDING = 38,
    GAMEOBJECT_TYPE_GARRISON_PLOT = 39,
    GAMEOBJECT_TYPE_CLIENT_CREATURE = 40,
    GAMEOBJECT_TYPE_CLIENT_ITEM = 41,
    GAMEOBJECT_TYPE_CAPTURE_POINT_NEW = 42,
    GAMEOBJECT_TYPE_PHASEABLE_MO = 43,
    GAMEOBJECT_TYPE_GARRISON_MONUMENT = 44,
    GAMEOBJECT_TYPE_GARRISON_SHIPMENT = 45,
    GAMEOBJECT_TYPE_GARRISON_MONUMENT_PLAQUE = 46,

    // Legion+
    GAMEOBJECT_TYPE_ITEM_FORGE = 47,
    GAMEOBJECT_TYPE_UI_LINK = 48,
    GAMEOBJECT_TYPE_KEYSTONE_RECEPTACLE = 49,
    GAMEOBJECT_TYPE_GATHERING_NODE = 50,
    GAMEOBJECT_TYPE_CHALLENGE_MODE_REWARD = 51,

    // BfA+
    GAMEOBJECT_TYPE_MULTI = 52,
    GAMEOBJECT_TYPE_SIEGEABLE_MULTI = 53,
    GAMEOBJECT_TYPE_SIEGEABLE_MO = 54,
    GAMEOBJECT_TYPE_PVP_REWARD = 55,

    // Shadowlands+
    GAMEOBJECT_TYPE_PLAYER_CHOICE_CHEST = 56,
    GAMEOBJECT_TYPE_LEGENDARY_FORGE = 57,
    GAMEOBJECT_TYPE_GARR_TALENT_TREE = 58,
    GAMEOBJECT_TYPE_WEEKLY_REWARD_CHEST = 59,

    // Dragonflight / TWW+
    GAMEOBJECT_TYPE_CLIENT_MODEL = 60,
    GAMEOBJECT_TYPE_CRAFTING_TABLE = 61,
    GAMEOBJECT_TYPE_PERKS_PROGRAM_CHEST = 62,

    // The War Within (11.x) / Midnight (12.x)
    GAMEOBJECT_TYPE_DELVE_REWARD = 63,
    GAMEOBJECT_TYPE_HOUSING_DECOR = 64,

    MAX_GAMEOBJECT_TYPE
};

//\todo check this enum - do we (ae) really handle this stuff this way?
enum GameObjectOverrides : uint32_t
{
    GAMEOBJECT_NORMAL_DISTANCE = 0x00,
    GAMEOBJECT_INFVIS = 0x01,                   // Makes the gameobject forever visible on the map after you saw it at least once - for various transports; actually it just doesn't erase it while you're on the same map.
    GAMEOBJECT_MAPWIDE = 0x02,                  // Legacy name: published grid-wide by the visibility system, not across the entire map.
    GAMEOBJECT_AREAWIDE = 0x04,                 // Published with an extended cell radius by the visibility system.
    GAMEOBJECT_ONMOVEWIDE = 0x08,               // When this gameobject moves and sends updates about it's position, do so in the second range - MapMgr::ChangeObjectLocation, +/- 6 units wide instead of +/- 1.
    GAMEOBJECT_OVERRIDE_FLAGS = 0x10,           //\todo UNIMPLEMENTED, Let the core decide about the flags sent in the A9 - example: 252 instead of 352 for Deeprun Tram.
    GAMEOBJECT_OVERRIDE_BYTES1 = 0x20,          //\todo UNIMPLEMENTED, Let the core use the full field instead an uint8_t in GAMEOBJECT_BYTES_1, if the database creator knows what to do with it.
    GAMEOBJECT_OVERRIDE_PARENTROT = 0x40,       // Makes it possible for the core to skip calculating these fields and use whatever was specified in the spawn.
    // Later other types might folow, or the upper bytes might get used for the AREAWIDE option in the overrides variable...
};

enum GameObjectBytes : uint8_t
{
    GAMEOBJECT_BYTES_STATE          = 0,
    GAMEOBJECT_BYTES_TYPE_ID        = 1,
    GAMEOBJECT_BYTES_ARTKIT         = 2, // Visual artkit (or destructible health percent in MoP+)
    GAMEOBJECT_BYTES_UNK            = 2, // Legacy alias
    GAMEOBJECT_BYTES_ANIMPROGRESS   = 3,
};

enum GameObjectState : uint8_t
{
    GO_STATE_OPEN = 0,
    GO_STATE_CLOSED = 1,
    GO_STATE_ALTERNATIVE_OPEN = 2,
    GO_STATE_DESTROYED = 3
};

enum GameObjectSummonType : uint8_t
{
    GO_SUMMON_TIMED_OR_CORPSE_DESPAWN = 0, // Despawns after a specified time OR when the summoner dies
    GO_SUMMON_TIMED_DESPAWN = 1 // Despawns after a specified time
};

enum GameObjectDynamicFlags : uint16_t
{
    GO_DYN_FLAG_NONE = 0x0000,
    GO_DYN_FLAG_INTERACTABLE = 0x0001,
    GO_DYN_FLAG_LOOTABLE = 0x0002,
    GO_DYN_FLAG_HIGHLIGHT = 0x0004,
    GO_DYN_FLAG_SPARKLE = 0x0008,
    GO_DYN_FLAG_TRANSPORT_STOPPED = 0x0010,
    GO_DYN_FLAG_INVERTED_MOVEMENT = 0x0020,
    GO_DYN_FLAG_LOOTED = 0x0040
};
