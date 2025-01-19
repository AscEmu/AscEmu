/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgTaxinodeStatus : public ManagedPacket
    {
    public:
        uint64_t guid;
        bool isNodeKnown;

        SmsgTaxinodeStatus() : SmsgTaxinodeStatus(0, 0)
        {
        }

        SmsgTaxinodeStatus(uint64_t guid, bool isNodeKnown) :
            ManagedPacket(SMSG_TAXINODE_STATUS, 0),
            guid(guid),
            isNodeKnown(isNodeKnown)
        {
        }

    protected:

        size_t expectedSize() const override { return 9; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid;
#if VERSION_STRING < Cata
            packet << static_cast<uint8_t>(isNodeKnown ? 1 : 0);
#else
            packet << static_cast<uint8_t>(isNodeKnown ? 1 : 2);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
