/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgQuestupdateFailed : public ManagedPacket
    {
    public:
        uint32_t questId;

        SmsgQuestupdateFailed() : SmsgQuestupdateFailed(0)
        {}

        SmsgQuestupdateFailed(uint32_t questId) :
            ManagedPacket(SMSG_QUESTUPDATE_FAILED, 4),
            questId(questId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << questId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
