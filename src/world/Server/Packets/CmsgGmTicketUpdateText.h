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
    class CmsgGmTicketUpdateText : public ManagedPacket
    {
    public:
        std::string message;

        CmsgGmTicketUpdateText() : CmsgGmTicketUpdateText("")
        {
        }

        CmsgGmTicketUpdateText(std::string message) :
            ManagedPacket(CMSG_GMTICKET_UPDATETEXT, 0),
            message(message)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> message;

            return true;
        }
    };
}}
