/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
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
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> message;

            return true;
        }
    };
}
