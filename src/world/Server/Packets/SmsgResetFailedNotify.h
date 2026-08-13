/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgResetFailedNotify : public ManagedPacket
    {
    public:
        uint32_t mapId;

        SmsgResetFailedNotify() : SmsgResetFailedNotify(0)
        {
        }

        SmsgResetFailedNotify(uint32_t mapId) :
            ManagedPacket(SMSG_RESET_FAILED_NOTIFY, 4),
            mapId(mapId)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << mapId;
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
