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
    class SmsgSetTimeZoneInformation : public ManagedPacket
    {
    public:
        std::string timeZone;

        SmsgSetTimeZoneInformation() : SmsgSetTimeZoneInformation("")
        {
        }

        SmsgSetTimeZoneInformation(std::string timeZone) :
            ManagedPacket(SMSG_SET_TIME_ZONE_INFORMATION, 2 + timeZone.length() * 2),
            timeZone(timeZone)
        {
        }

    protected:
        size_t expectedSize() const override { return 2 + timeZone.length() * 2; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBits(timeZone.length(), 7);
                packet.writeBits(timeZone.length(), 7);
                packet.flushBits();
                packet.writeString(timeZone);
                packet.writeString(timeZone);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
