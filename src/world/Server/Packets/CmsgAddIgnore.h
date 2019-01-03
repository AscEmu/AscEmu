/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgAddIgnore : public ManagedPacket
    {
    public:
        std::string name;

        CmsgAddIgnore() : CmsgAddIgnore("")
        {
        }

        CmsgAddIgnore(std::string name) :
            ManagedPacket(CMSG_ADD_IGNORE, 4),
            name(name)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << name;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name;
            return true;
        }
    };
}}
