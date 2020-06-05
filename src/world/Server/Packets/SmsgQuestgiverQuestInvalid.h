/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

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
#if VERSION_STRING == Mop
            packet.writeBit(1);
            packet.flushBits();
#endif
            packet << reason;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
