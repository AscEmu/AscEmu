/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgSummonResponse : public ManagedPacket
    {
    public:
        uint64_t summonGuid;
        uint8_t isClickOn;

        CmsgSummonResponse() : CmsgSummonResponse(0, 0)
        {
        }

        CmsgSummonResponse(uint64_t summonGuid, uint8_t isClickOn) :
            ManagedPacket(CMSG_SUMMON_RESPONSE, 8 + 1),
            summonGuid(summonGuid),
            isClickOn(isClickOn)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> summonGuid >> isClickOn;
            return true;
        }
    };
}}
