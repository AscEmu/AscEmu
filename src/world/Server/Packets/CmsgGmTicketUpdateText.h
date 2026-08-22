/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <string>

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
            if (m_protocol.isMop())
            {
                packet.flushBits();
                const uint32_t messageLen = packet.readBits(11);
                message = packet.readString(messageLen);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> message;

                return true;
            }

            return false;
        }
    };
}
