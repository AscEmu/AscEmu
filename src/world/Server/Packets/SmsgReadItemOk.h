/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgReadItemOk : public ManagedPacket
    {
    public:
        uint64_t itemGuid;

        SmsgReadItemOk() : SmsgReadItemOk(0)
        {
        }

        SmsgReadItemOk(uint64_t itemGuid) :
            ManagedPacket(SMSG_READ_ITEM_OK, 0),
            itemGuid(itemGuid)
        {
        }

    protected:

        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                packet.setOpcode(SMSG_READ_ITEM_RESULT_OK);
                packet << WoWGuid(itemGuid).toGuid128(m_protocol.realmId, 0);
                return true;
            }

            packet << itemGuid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
