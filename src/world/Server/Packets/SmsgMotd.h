/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgMotd : public ManagedPacket
    {
#if VERSION_STRING > Classic
    public:
        std::vector<std::string> motdLines;
        uint32_t lineCount;

        SmsgMotd() : SmsgMotd({""})
        {
        }

        SmsgMotd(std::vector<std::string> motdLines) :
            ManagedPacket(SMSG_MOTD, 50),
            motdLines(motdLines),
            lineCount(static_cast<uint32_t>(motdLines.size()))
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop

            packet << lineCount;
            for (const auto& line : motdLines)
                packet << line;

#else

            packet.writeBits(lineCount, 4);

            ByteBuffer stringBuffer;

            for (const auto& line : motdLines)
            {
                packet.writeBits(strlen(line.c_str()), 7);
                stringBuffer.WriteString(line);
            }

            packet.flushBits();
            packet.append(stringBuffer);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
