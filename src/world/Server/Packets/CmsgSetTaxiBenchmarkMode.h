/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
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
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet >> mode;
                return true;
            }

            return false;
        }
    };
}
