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
    class CmsgGmTicketCreate : public ManagedPacket
    {
    public:
        uint32_t map;
        LocationVector location;
        std::string message;
        std::string message2;

        CmsgGmTicketCreate() : CmsgGmTicketCreate(0, { 0.0f, 0.0f, 0.0f }, "", "")
        {
        }

        CmsgGmTicketCreate(uint32_t map, LocationVector location, std::string message, std::string message2) :
            ManagedPacket(CMSG_GMTICKET_CREATE, 4 + 4 * 3 + 2),
            map(map),
            location(location),
            message(message),
            message2(message2)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> map >> location.x >> location.y >> location.z >> message >> message2;

            return true;
        }
    };
}}
