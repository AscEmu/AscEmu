/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgQuestgiverStatus : public ManagedPacket
    {
    public:
        uint64_t questgiverGuid;
        uint32_t status;

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

        size_t expectedSize() const override { return 12; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << questgiverGuid << status;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
