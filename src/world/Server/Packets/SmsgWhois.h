/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <string>

namespace AscEmu::Packets
{
    class SmsgWhois : public ManagedPacket
    {
    public:
        std::string message;

        SmsgWhois() : SmsgWhois("") {}

        explicit SmsgWhois(std::string message) :
            ManagedPacket(SMSG_WHOIS, message.size() + 1),
            message(std::move(message))
        {}

    protected:
        size_t expectedSize() const override { return message.size() + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << message;

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBits(message.size(), 11);
                packet.flushBits();
                if (message.size())
                    packet.writeString(message);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
