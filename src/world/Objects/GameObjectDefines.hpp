/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

enum GameObject_Flags
{
    GO_FLAG_NONE                = 0x000,
    GO_FLAG_NONSELECTABLE       = 0x001,
    GO_FLAG_LOCKED              = 0x002,
    GO_FLAG_UNTARGETABLE        = 0x004,
    GO_FLAG_TRANSPORT           = 0x008,
    GO_FLAG_NOT_SELECTABLE      = 0x010,
    GO_FLAG_NEVER_DESPAWN       = 0x020,
    GO_FLAG_TRIGGERED           = 0x040,
    GO_FLAG_UNK1                = 0x080,
    GO_FLAG_UNK2                = 0x100,
    GO_FLAG_DAMAGED             = 0x200,
    GO_FLAG_DESTROYED           = 0x400
};

enum GameObjectDestructibleState
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
    GAMEOBJECT_TYPE_TRAPDOOR                = 35
};

//\todo check this enum - do we (ae) really handle this stuff this way?
enum GameObjectOverrides
{
    GAMEOBJECT_NORMAL_DISTANCE = 0x00,
    GAMEOBJECT_INFVIS = 0x01,                   // Makes the gameobject forever visible on the map after you saw it at least once - for various transports; actually it just doesn't erase it while you're on the same map.
    GAMEOBJECT_MAPWIDE = 0x02,                  // When you enter its map, the gameobject gets pushed to you no matter how far it is (but only for players), especially for Deeprun and Ulduar Trams.
    GAMEOBJECT_AREAWIDE = 0x04,                 //\todo UNIMPLEMENTED, but will work like this: the Map will get marked that it contains an object like this, and on player movement these objects will get distance-checked to spawn them from a greater distance than normal if needed - for few objects on smaller maps, like on battlegrounds; maybe they'll get area-triggered, haven't decided yet.
    GAMEOBJECT_ONMOVEWIDE = 0x08,               // When this gameobject moves and sends updates about it's position, do so in the second range - MapMgr::ChangeObjectLocation, +/- 6 units wide instead of +/- 1.
    GAMEOBJECT_OVERRIDE_FLAGS = 0x10,           //\todo UNIMPLEMENTED, Let the core decide about the flags sent in the A9 - example: 252 instead of 352 for Deeprun Tram.
    GAMEOBJECT_OVERRIDE_BYTES1 = 0x20,          //\todo UNIMPLEMENTED, Let the core use the full field instead an uint8_t in GAMEOBJECT_BYTES_1, if the database creator knows what to do with it.
    GAMEOBJECT_OVERRIDE_PARENTROT = 0x40,       // Makes it possible for the core to skip calculating these fields and use whatever was specified in the spawn.
    // Later other types might folow, or the upper bytes might get used for the AREAWIDE option in the overrides variable...
};

enum GameObjectBytes
{
    GAMEOBJECT_BYTES_STATE          = 0,
    GAMEOBJECT_BYTES_TYPE_ID        = 1,
    GAMEOBJECT_BYTES_UNK            = 2,        //\todo unknown atm
    GAMEOBJECT_BYTES_ANIMPROGRESS   = 3,
};
