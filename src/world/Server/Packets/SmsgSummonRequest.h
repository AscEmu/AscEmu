/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgSummonRequest : public ManagedPacket
    {
    public:
        uint64_t requestGuid;
        uint32_t zoneId;
        uint32_t timerMs;

        SmsgSummonRequest() : SmsgSummonRequest(0, 0, 0)
        {
        }

        SmsgSummonRequest(uint64_t requestGuid, uint32_t zoneId, uint32_t timerMs) :
            ManagedPacket(SMSG_SUMMON_REQUEST, 0),
            requestGuid(requestGuid),
            zoneId(zoneId),
            timerMs(timerMs)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << requestGuid << zoneId << timerMs;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
