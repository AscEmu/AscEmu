/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgGmTicketCreate : public ManagedPacket
    {
    public:
        uint32_t map;
        LocationVector location;
        std::string message;
#if VERSION_STRING < Cata
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
#else
        uint32_t responseNeeded;
        bool moreHelpNeeded;
        uint32_t ticketCount;
        std::list<uint32_t> timesList {};
        uint32_t decompressedSize;

        CmsgGmTicketCreate() : CmsgGmTicketCreate(0, { 0.0f, 0.0f, 0.0f }, "", 0, false, 0, 0)
        {
        }

        CmsgGmTicketCreate(uint32_t map, LocationVector location, std::string message, uint32_t responseNeeded,
            bool moreHelpNeeded, uint32_t ticketCount, uint32_t decompressedSize) :
            ManagedPacket(CMSG_GMTICKET_CREATE, 4 + 4 * 3 + 2),
            map(map),
            location(location),
            message(message),
            responseNeeded(responseNeeded),
            moreHelpNeeded(moreHelpNeeded),
            ticketCount(ticketCount),
            decompressedSize(decompressedSize)
        {
        }
#endif

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> map >> location.x >> location.y >> location.z >> message;
#if VERSION_STRING < Cata
            packet >> message2;
#else
            packet >> responseNeeded >> moreHelpNeeded >> ticketCount;

            for (uint32_t i = 0; i < ticketCount; ++i)
            {
                uint32_t time;
                packet >> time;
                timesList.push_back(time);
            }

            packet >> decompressedSize;
#endif

            return true;
        }
    };
}
