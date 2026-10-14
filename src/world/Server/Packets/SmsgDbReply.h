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
    class SmsgDbReply : public ManagedPacket
    {
    public:
        uint32_t entry;
        uint32_t type;
        ByteBuffer buffer;
        uint32_t hotfixTime;
        uint32_t bufferSize;
        bool found{true};

        SmsgDbReply() : SmsgDbReply(0, 0, 0)
        {
        }

        SmsgDbReply(uint32_t entry, uint32_t type, ByteBuffer buffer, bool found = true) :
            ManagedPacket(SMSG_DB_REPLY, 0),
            entry(entry),
            type(type),
            buffer(buffer),
            found(found)
        {
            hotfixTime = uint32_t(time(nullptr));
            bufferSize = uint32_t(buffer.size());
        }

    protected:
        size_t expectedSize() const override { return (m_protocol.isForever() ? 17U : 16U) + buffer.size(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                packet << type << entry << hotfixTime;
                packet.writeBits<uint8_t>(found ? 1U : 4U, 3);
                packet.flushBits();
                packet << bufferSize;
                if (bufferSize != 0)
                    packet.append(buffer);
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet << entry;

                if (m_protocol.isMop())
                    packet << hotfixTime << type;
                else
                    packet << type << hotfixTime;

                packet << bufferSize;
                packet.append(buffer);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
