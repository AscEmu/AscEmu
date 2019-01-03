/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgDestroyObject : public ManagedPacket
    {
    public:

        uint64_t guid;

        SmsgDestroyObject() : SmsgDestroyObject(0)
        {
        }

        SmsgDestroyObject(uint64_t guid) : 
        ManagedPacket(SMSG_DESTROY_OBJECT, 9),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid;
#if VERSION_STRING >= WotLK
            packet << uint8_t(0);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
