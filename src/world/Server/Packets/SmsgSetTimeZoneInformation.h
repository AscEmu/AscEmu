/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <string>
#include <utility>

namespace AscEmu::Packets
{
    class SmsgSetTimeZoneInformation : public ManagedPacket
    {
    public:
        std::string timeZone;
        std::string regionalTimeZone;

        SmsgSetTimeZoneInformation() : SmsgSetTimeZoneInformation("")
        {
        }

        SmsgSetTimeZoneInformation(std::string timeZone) :
            SmsgSetTimeZoneInformation(timeZone, timeZone)
        {
        }

        SmsgSetTimeZoneInformation(std::string timeZone, std::string regionalTimeZone) :
            ManagedPacket(SMSG_SET_TIME_ZONE_INFORMATION, 3 + timeZone.length() * 2 + regionalTimeZone.length()),
            timeZone(std::move(timeZone)),
            regionalTimeZone(std::move(regionalTimeZone))
        {
        }

    protected:
        size_t expectedSize() const override { return m_protocol.isForever() ? 3 + timeZone.length() * 2 + regionalTimeZone.length() : 2 + timeZone.length() * 2; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                packet.writeBits(timeZone.length(), 7);
                packet.writeBits(timeZone.length(), 7);
                packet.writeBits(regionalTimeZone.length(), 7);
                packet.flushBits();
                packet.writeString(timeZone);
                packet.writeString(timeZone);
                packet.writeString(regionalTimeZone);
                return true;
            }

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
