/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLoadEquipmentSet : public ManagedPacket
    {
    public:
        SmsgLoadEquipmentSet() :
            ManagedPacket(SMSG_LOAD_EQUIPMENT_SET, 3)
        {
        }

    protected:
        size_t expectedSize() const override { return 3; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBits(0, 19);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
