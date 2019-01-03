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
    class CmsgAddFriend : public ManagedPacket
    {
    public:
        std::string name;
        std::string note;

        CmsgAddFriend() : CmsgAddFriend("", "")
        {
        }

        CmsgAddFriend(std::string name, std::string note) :
            ManagedPacket(CMSG_ADD_FRIEND, 4),
            name(name),
            note(note)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << name << note;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name >> note;
            return true;
        }
    };
}}
