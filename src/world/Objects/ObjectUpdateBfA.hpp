/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"

#if VERSION_STRING == BfA

#include <cstdint>

class ByteBuffer;
class Object;
class Player;
class UpdateMask;

// Object values of 8.x clients: the server keeps its values in the 7.3.5 layout, the client receives them as
// update field structures (object, unit, player, active player, item, container, gameobject, dynamic object,
// corpse, area trigger) with a change mask per structure.
namespace ObjectUpdateBfA
{
    // object type byte of the create block
    uint8_t wireObjectTypeId(Object const* object, Player const* target);

    // the values of a create block: size, visibility flags, then the structures of the object type
    void writeCreateValues(Object* object, Player* target, ByteBuffer& data);

    // the values of an update block: size, the changed object types, then the changed fields of every structure;
    // the update mask lists the changed values of the server layout
    void writeUpdateValues(Object* object, Player* target, UpdateMask const& mask, ByteBuffer& data);
}

#endif
