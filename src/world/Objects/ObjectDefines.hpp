/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include "Server/ClientProtocol.hpp"

/**
 * @file ObjectDefines.hpp
 * @brief Core object type definitions, update types, and state enums.
 *
 * AscEmu Object Class & Network Type Hierarchy:
 *
 * Object (TYPEID_OBJECT / TYPE_OBJECT)
 *  ├── Item (TYPEID_ITEM / TYPEMASK_ITEM)
 *  │    └── Container (TYPEID_CONTAINER / TYPEMASK_CONTAINER)
 *  ├── Unit (TYPE_UNIT)
 *  │    ├── Player (TYPEID_PLAYER / TYPEMASK_PLAYER)
 *  │    └── Creature (TYPEID_UNIT / TYPEMASK_UNIT)
 *  │         ├── Vehicle
 *  │         └── Summon
 *  │              ├── Pet
 *  │              └── Totem
 *  ├── GameObject (TYPEID_GAMEOBJECT / TYPEMASK_GAMEOBJECT)
 *  │    └── Transporter
 *  ├── DynamicObject (TYPEID_DYNAMICOBJECT / TYPEMASK_DYNAMICOBJECT)
 *  ├── Corpse (TYPEID_CORPSE / TYPEMASK_CORPSE)
 *  ├── AreaTrigger (TYPEID_AREATRIGGER / TYPEMASK_AREATRIGGER)       [Cata+]
 *  ├── SceneObject (TYPEID_SCENEOBJECT / TYPEMASK_SCENEOBJECT)       [MoP+]
 *  └── Conversation (TYPEID_CONVERSATION / TYPEMASK_CONVERSATION)    [Legion+]
 *
 * @note Creatures, Vehicles, Summons, Pets, and Totems all share TYPEID_UNIT on the network.
 *       Use virtual helpers (isPet(), isSummon(), isVehicle(), etc.) to distinguish C++ subclasses.
 */

// Internal unified Object Type IDs used across the WorldServer.
// Always translate via wireObjectTypeId() when writing to 6.x+ packets.
enum TypeId : uint8_t
{
    TYPEID_OBJECT = 0,
    TYPEID_ITEM = 1,
    TYPEID_CONTAINER = 2,
    TYPEID_UNIT = 3,
    TYPEID_PLAYER = 4,
    TYPEID_GAMEOBJECT = 5,
    TYPEID_DYNAMICOBJECT = 6,
    TYPEID_CORPSE = 7,
    TYPEID_AIGROUP = 8, // Unused, kept to preserve index 9 for AreaTrigger
    TYPEID_AREATRIGGER = 9, // Cata+ (wire value is 8 in WoD/Legion)
    TYPEID_SCENEOBJECT = 10, // MoP+
    TYPEID_CONVERSATION = 11, // Legion+

    MAX_OBJECT_TYPEID
};

// Bitmask flags representing the object inheritance hierarchy.
enum TYPE : uint16_t
{
    TYPE_NONE = 0x0000,
    TYPE_OBJECT = (1 << TYPEID_OBJECT), // 0x0001
    TYPE_ITEM = (1 << TYPEID_ITEM), // 0x0002
    TYPE_CONTAINER = (1 << TYPEID_CONTAINER), // 0x0004
    TYPE_UNIT = (1 << TYPEID_UNIT), // 0x0008
    TYPE_PLAYER = (1 << TYPEID_PLAYER), // 0x0010
    TYPE_GAMEOBJECT = (1 << TYPEID_GAMEOBJECT), // 0x0020
    TYPE_DYNAMICOBJECT = (1 << TYPEID_DYNAMICOBJECT), // 0x0040
    TYPE_CORPSE = (1 << TYPEID_CORPSE), // 0x0080
    TYPE_AIGROUP = (1 << TYPEID_AIGROUP), // 0x0100
    TYPE_AREATRIGGER = (1 << TYPEID_AREATRIGGER), // 0x0200
    TYPE_SCENEOBJECT = (1 << TYPEID_SCENEOBJECT), // 0x0400
    TYPE_CONVERSATION = (1 << TYPEID_CONVERSATION), // 0x0800

    // Combined hierarchy masks
    TYPEMASK_OBJECT = TYPE_OBJECT,
    TYPEMASK_ITEM = TYPEMASK_OBJECT | TYPE_ITEM,
    TYPEMASK_CONTAINER = TYPEMASK_ITEM | TYPE_CONTAINER,
    TYPEMASK_UNIT = TYPEMASK_OBJECT | TYPE_UNIT,
    TYPEMASK_PLAYER = TYPEMASK_UNIT | TYPE_PLAYER,
    TYPEMASK_GAMEOBJECT = TYPEMASK_OBJECT | TYPE_GAMEOBJECT,
    TYPEMASK_DYNAMICOBJECT = TYPEMASK_OBJECT | TYPE_DYNAMICOBJECT,
    TYPEMASK_CORPSE = TYPEMASK_OBJECT | TYPE_CORPSE,
    TYPEMASK_AREATRIGGER = TYPEMASK_OBJECT | TYPE_AREATRIGGER,
    TYPEMASK_SCENEOBJECT = TYPEMASK_OBJECT | TYPE_SCENEOBJECT,
    TYPEMASK_CONVERSATION = TYPEMASK_OBJECT | TYPE_CONVERSATION
};

[[nodiscard]] constexpr uint16_t getTypeMaskForTypeId(uint8_t objectTypeId) noexcept
{
    switch (objectTypeId)
    {
        case TYPEID_ITEM:
            return TYPEMASK_ITEM;
        case TYPEID_CONTAINER:
            return TYPEMASK_CONTAINER;
        case TYPEID_UNIT:
            return TYPEMASK_UNIT;
        case TYPEID_PLAYER:
            return TYPEMASK_PLAYER;
        case TYPEID_GAMEOBJECT:
            return TYPEMASK_GAMEOBJECT;
        case TYPEID_DYNAMICOBJECT:
            return TYPEMASK_DYNAMICOBJECT;
        case TYPEID_CORPSE:
            return TYPEMASK_CORPSE;
        case TYPEID_AREATRIGGER:
            return TYPEMASK_AREATRIGGER;
        case TYPEID_SCENEOBJECT:
            return TYPEMASK_SCENEOBJECT;
        case TYPEID_CONVERSATION:
            return TYPEMASK_CONVERSATION;
        default:
            return TYPEMASK_OBJECT;
    }
}


// Internal unified Object Update Types.
// Always translate via getWireUpdateType() when writing the block header to a ByteBuffer!
enum OBJECT_UPDATE_TYPE : uint8_t
{
    UPDATETYPE_VALUES = 0,
    UPDATETYPE_MOVEMENT = 1, // Classic - WotLK only
    UPDATETYPE_CREATE_OBJECT = 2,
    UPDATETYPE_CREATE_OBJECT2 = 3,
    UPDATETYPE_OUT_OF_RANGE_OBJECTS = 4
};

// Translates the internal OBJECT_UPDATE_TYPE to the wire byte expected by the active server expansion
[[nodiscard]] inline uint8_t getWireUpdateType(uint8_t updateType) noexcept
{
    if (WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata) && updateType >= UPDATETYPE_CREATE_OBJECT)
        return updateType - 1;

    return updateType;
}

enum PHASECOMMANDS : uint8_t
{
    PHASE_SET = 0, /// Overwrites the phase value with the supplied one
    PHASE_ADD = 1, /// Adds the new bits to the current phase value
    PHASE_DEL = 2, /// Removes the given bits from the current phase value
    PHASE_RESET = 3 /// Sets the default phase of 1, same as PHASE_SET with 1 as the new value
};

struct QuaternionData
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    constexpr QuaternionData() noexcept = default;

    constexpr QuaternionData(float inX, float inY, float inZ, float inW) noexcept
        : x(inX), y(inY), z(inZ), w(inW) {}

    constexpr bool operator==(const QuaternionData&) const noexcept = default;

    bool isUnit() const;
    void toEulerAnglesZYX(float& z, float& y, float& x) const;
    static QuaternionData fromEulerAnglesZYX(float z, float y, float x);
};

enum CreatureSummonDespawnType : uint8_t
{
    TIMED_OR_DEAD_DESPAWN = 1,
    TIMED_OR_CORPSE_DESPAWN = 2,
    TIMED_DESPAWN = 3,
    TIMED_DESPAWN_OUT_OF_COMBAT = 4,
    CORPSE_DESPAWN = 5,
    CORPSE_TIMED_DESPAWN = 6,
    DEAD_DESPAWN = 7,
    MANUAL_DESPAWN = 8
};
