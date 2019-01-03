/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgSetTaxiBenchmarkMode : public ManagedPacket
    {
    public:
        uint8_t mode;

        CmsgSetTaxiBenchmarkMode() : CmsgSetTaxiBenchmarkMode(0)
        {
        }

        CmsgSetTaxiBenchmarkMode(uint8_t mode) :
            ManagedPacket(CMSG_SET_TAXI_BENCHMARK_MODE, 1),
            mode(mode)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> mode;
            return true;
        }
    };
}}
