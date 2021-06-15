/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgQuestgiverStatus : public ManagedPacket
    {
    public:
        uint64_t questgiverGuid;
#if VERSION_STRING < Cata
        uint8_t status;
#else
        uint32_t status;
#endif

        SmsgQuestgiverStatus() : SmsgQuestgiverStatus(0, 0)
        {
        }

        SmsgQuestgiverStatus(uint64_t questgiverGuid, uint32_t status) :
            ManagedPacket(SMSG_QUESTGIVER_STATUS, 0),
            questgiverGuid(questgiverGuid),
            status(status)
        {
        }

    protected:

        size_t expectedSize() const override
        {
#if VERSION_STRING < Cata
            return 8 + 1;
#else
            return 8 + 4;
#endif
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << questgiverGuid << status;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
