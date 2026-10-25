/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CalendarPacketData.hpp"

#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class SmsgCalendarCommandResult : public ManagedPacket
    {
    public:
        uint32_t error = 0;
        std::string param;

        SmsgCalendarCommandResult() : SmsgCalendarCommandResult(0, "")
        {
        }

        SmsgCalendarCommandResult(uint32_t error, std::string param = "") :
            ManagedPacket(SMSG_CALENDAR_COMMAND_RESULT, 0),
            error(error),
            param(std::move(param))
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            // only these errors name a player
            const bool hasParam = error == CALENDAR_ERROR_OTHER_INVITES_EXCEEDED || error == CALENDAR_ERROR_ALREADY_INVITED_TO_EVENT_S
                || error == CALENDAR_ERROR_IGNORING_YOU_S;
            const std::string name = hasParam ? param : std::string();

            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                packet << uint8_t(1);                       // command
                packet << uint8_t(error);
                packet.writeBits(name.size(), 9);
                CalendarWire::writeRaw(packet, name);
                return true;
            }

            if (m_protocol.isMop())
            {
                const uint32_t length = static_cast<uint32_t>(name.size());
                packet.writeBits(length / 2, 8);
                packet.writeBit((length % 2) != 0);
                packet.flushBits();

                packet << uint8_t(0);                       // command
                packet << uint8_t(error);
                packet.writeString(name);
                return true;
            }

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << uint32_t(0);
                packet << uint8_t(0);
                if (hasParam)
                    packet << param;
                else
                    packet << uint8_t(0);

                packet << error;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
