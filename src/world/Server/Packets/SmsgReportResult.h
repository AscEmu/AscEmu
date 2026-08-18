/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgReportResult : public ManagedPacket
    {
    public:
        SmsgReportResult() :
            ManagedPacket(SMSG_REPORT_RESULT, 2)
        {
        }

    protected:
        size_t expectedSize() const override { return 2; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
            {
                packet << uint8_t(0);                                     // 1 reset reported player 0 ignore
                packet << uint8_t(0);
                return true;
            }
            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
