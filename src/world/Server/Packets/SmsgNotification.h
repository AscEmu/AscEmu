/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

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
#if VERSION_STRING < Cata
            packet << text;

#elif VERSION_STRING == Cata
            packet.writeBits(text.size(), 13);
            packet.flushBits();
            packet.append(text.c_str(), text.size());

#else
            packet.writeBits(text.size(), 12);
            packet.flushBits();
            packet.append(text.c_str(), text.size());
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
