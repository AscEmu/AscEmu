/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGmTicketCreate : public ManagedPacket
    {
    public:
        uint32_t map;
        LocationVector location;
        std::string message;

        std::string message2;   // Cata

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

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet >> map;
                packet >> location.z;
                packet >> location.y;
                packet.readSkip<uint8_t>();    // flags, unused
                packet >> location.x;
                packet >> ticketCount;

                if (ticketCount > 0)
                {
                    uint8_t textCount = 0;
                    packet >> textCount;

                    for (uint8_t i = 0; i < textCount; ++i)
                    {
                        uint32_t time;
                        packet >> time;
                        timesList.push_back(time);
                    }

                    packet >> decompressedSize;

                    // A compressed log follows here. Its compressed length isn't encoded
                    // anywhere in the packet, so we can't reliably skip past it to reach the
                    // bit-packed message below - matches the handler, which already discards
                    // this log after decompressing it.
                    return true;
                }

                packet.flushBits();
                responseNeeded = packet.readBit() ? 1 : 0;
                moreHelpNeeded = packet.readBit();

                const uint32_t messageLen = packet.readBits(11);
                message = packet.readString(messageLen);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> map >> location.x >> location.y >> location.z >> message;

                if (m_protocol.expansion < WoW::Expansion::_Cata)
                {
                    packet >> message2;
                }
                else
                {
                    packet >> responseNeeded >> moreHelpNeeded >> ticketCount;

                    for (uint32_t i = 0; i < ticketCount; ++i)
                    {
                        uint32_t time;
                        packet >> time;
                        timesList.push_back(time);
                    }

                    packet >> decompressedSize;
                }

                return true;
            }

            return false;
        }
    };
}
