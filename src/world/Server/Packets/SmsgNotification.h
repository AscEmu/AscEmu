/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <string>

namespace AscEmu::Packets
{
    class SmsgNotification : public ManagedPacket
    {
    public:
        std::string text;

        SmsgNotification() : SmsgNotification("")
        {
        }

        SmsgNotification(std::string text) :
            ManagedPacket(SMSG_NOTIFICATION, text.size() + 2),
            text(text)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << text;
            }
            else if (m_protocol.expansion == WoW::Expansion::_Cata)
            {
                packet.writeBits(text.size(), 13);
                packet.flushBits();
                packet.append(text.c_str(), text.size());
            }
            else
            {
                packet.writeBits(text.size(), 12);
                packet.flushBits();
                packet.append(text.c_str(), text.size());
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
