/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // the hotfix ids the client may request on the glue screen (9.x and 10.x), none are offered
    class SmsgAvailableHotfixes : public ManagedPacket
    {
    public:
        SmsgAvailableHotfixes() : ManagedPacket(SMSG_AVAILABLE_HOTFIXES, 4 + 4) {}

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isShadowlands() && !m_protocol.isDragonflight())
                return false;

            packet << int32_t(m_protocol.getVirtualRealmAddress());
            packet << uint32_t(0);                  // hotfix ids
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
