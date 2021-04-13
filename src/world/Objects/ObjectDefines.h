/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once


//\brief: TypeFlags, mostly used to define updatePackets.
//\note: check out class inheritance. it is not true that every unit is "just" a creature.

/*
 object
- unit
- - player
- - creature
- - - pet   <- probably wrong inheritance from summon not from creature
- - - totem <- probably wrong inheritance from summon not from creature
- - - vehicle
- - - summon (pets and totems are always summons)
- - - - pet
- - - - totem
- gameobject
- dynamicobject
- corpse
- item
- - container
 */

enum TYPE
{
    TYPE_OBJECT = 1,
    TYPE_ITEM = 2,
    TYPE_CONTAINER = 4,
    TYPE_UNIT = 8,
    TYPE_PLAYER = 16,
    TYPE_GAMEOBJECT = 32,
    TYPE_DYNAMICOBJECT = 64,
    TYPE_CORPSE = 128,
    //TYPE_AIGROUP = 256,       not used
    //TYPE_AREATRIGGER = 512,   not used
    //TYPE_IN_GUILD = 1024      not used
};

//\todo: remove these typeIds and use flags instead. No reason to use two different enums to define a object type.
enum TYPEID
{
    TYPEID_OBJECT = 0,
    TYPEID_ITEM = 1,
    TYPEID_CONTAINER = 2,
    TYPEID_UNIT = 3,
    TYPEID_PLAYER = 4,
    TYPEID_GAMEOBJECT = 5,
    TYPEID_DYNAMICOBJECT = 6,
    TYPEID_CORPSE = 7,
    // TYPEID_AIGROUP = 8,      // Not used
    TYPEID_AREATRIGGER = 9      // WoWTrigger is a thing on Cata
};

#if VERSION_STRING <= WotLK
enum OBJECT_UPDATE_TYPE
{
    UPDATETYPE_VALUES = 0,
    UPDATETYPE_MOVEMENT = 1,
    UPDATETYPE_CREATE_OBJECT = 2,
    UPDATETYPE_CREATE_OBJECT2 = 3,
    UPDATETYPE_OUT_OF_RANGE_OBJECTS = 4
};
#elif VERSION_STRING >= Cata
enum OBJECT_UPDATE_TYPE
{
    UPDATETYPE_VALUES = 0,
    UPDATETYPE_CREATE_OBJECT = 1,
    UPDATETYPE_CREATE_OBJECT2 = 2,
    UPDATETYPE_OUT_OF_RANGE_OBJECTS = 3
};
#endif

enum PHASECOMMANDS
{
    PHASE_SET = 0,      /// overwrites the phase value with the supplied one
    PHASE_ADD = 1,      /// adds the new bits to the current phase value
    PHASE_DEL = 2,      /// removes the given bits from the current phase value
    PHASE_RESET = 3     /// sets the default phase of 1, same as PHASE_SET with 1 as the new value
};