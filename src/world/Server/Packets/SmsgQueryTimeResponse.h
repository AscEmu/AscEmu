/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <ctime>

namespace AscEmu::Packets
{
    class SmsgQueryTimeResponse : public ManagedPacket
    {
    public:
        uint64_t time;

        SmsgQueryTimeResponse() : SmsgQueryTimeResponse(time_t(0))
        {
        }

        explicit SmsgQueryTimeResponse(time_t time) : SmsgQueryTimeResponse(static_cast<uint64_t>(time))
        {
        }

        explicit SmsgQueryTimeResponse(uint32_t time) : SmsgQueryTimeResponse(static_cast<uint64_t>(time))
        {
        }

        explicit SmsgQueryTimeResponse(uint64_t time) :
            ManagedPacket(SMSG_QUERY_TIME_RESPONSE, 0),
            time(time)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                return sizeof(uint64_t);
            }
            else
            {
                return sizeof(uint32_t);
            }
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet << time;
            }
            else
            {
                packet << static_cast<uint32_t>(time);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet >> time;
            }
            else
            {
                uint32_t time_32;
                packet >> time_32;
                time = static_cast<uint64_t>(time_32);
            }
            return true;
        }
    };
}
