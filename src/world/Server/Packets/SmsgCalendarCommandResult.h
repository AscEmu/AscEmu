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
            const bool hasParam = (error == 4  /* CALENDAR_ERROR_OTHER_INVITES_EXCEEDED */
                || error == 10 /* CALENDAR_ERROR_ALREADY_INVITED_TO_EVENT_S */
                || error == 13 /* CALENDAR_ERROR_IGNORING_YOU_S */);

            if (m_protocol.isMop())
            {
                const uint32_t length = hasParam ? static_cast<uint32_t>(param.size()) : 0;

                packet.writeBits(length / 2, 8);
                packet.writeBit((length % 2) != 0);
                packet.flushBits();

                packet << uint8_t(0); // command
                packet << uint8_t(error);

                if (hasParam)
                    packet.append(reinterpret_cast<const uint8_t*>(param.c_str()), param.size() + 1);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
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
