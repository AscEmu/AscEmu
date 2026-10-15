/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgQuestupdateFailedTimer : public ManagedPacket
    {
    public:
        uint32_t questId;

        SmsgQuestupdateFailedTimer() : SmsgQuestupdateFailedTimer(0)
        {}

        SmsgQuestupdateFailedTimer(uint32_t questId) :
            ManagedPacket(SMSG_QUESTUPDATE_FAILEDTIMER, 4),
            questId(questId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                packet << uint32_t(questId);
                return true;
            }

            packet << questId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
