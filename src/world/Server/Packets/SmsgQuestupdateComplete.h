/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgQuestupdateComplete : public ManagedPacket
    {
    public:
        uint32_t questId;

        SmsgQuestupdateComplete() : SmsgQuestupdateComplete(0)
        {}

        SmsgQuestupdateComplete(uint32_t questId) :
            ManagedPacket(SMSG_QUESTUPDATE_COMPLETE, 4),
            questId(questId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 12.x: hide credit message bit
                packet << int32_t(questId);
                if (m_protocol.isMidnight())
                {
                    packet.writeBit(false);
                    packet.flushBits();
                }
                return true;
            }

            packet << questId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
