/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgChatPlayerNotFound : public ManagedPacket
    {
    public:
        std::string destination;

        SmsgChatPlayerNotFound() : SmsgChatPlayerNotFound("")
        {
        }

        SmsgChatPlayerNotFound(std::string destination) :
            ManagedPacket(SMSG_CHAT_PLAYER_NOT_FOUND, 0),
            destination(destination)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << destination;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> destination;
            return true;
        }
    };
}}
