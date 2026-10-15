/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgQuestgiverQuestInvalid : public ManagedPacket
    {
    public:
        uint32_t reason;

        SmsgQuestgiverQuestInvalid() : SmsgQuestgiverQuestInvalid(0)
        {}

        SmsgQuestgiverQuestInvalid(uint32_t reason) :
            ManagedPacket(SMSG_QUESTGIVER_QUEST_INVALID, 4),
            reason(reason)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // reason, 7.x and later the contribution reward, error message bit and an empty reason text
                packet << uint32_t(reason);
                if (!m_protocol.isWoD())
                    packet << int32_t(0);
                packet.writeBit(true);
                packet.writeBits(0, 9);
                packet.flushBits();
                return true;
            }

            if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                packet.writeBit(1);
                packet.flushBits();
            }
            packet << reason;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
