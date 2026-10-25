/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgNewTaxiPath : public ManagedPacket
    {
    public:

        int32_t node;       // the learned node, 11.x and later

        SmsgNewTaxiPath(int32_t node = 0) :
            ManagedPacket(SMSG_NEW_TAXI_PATH, 0),
            node(node)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isTheWarWithinOrLater())
                packet << node;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
