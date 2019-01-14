/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgTaxinodeStatus : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint8_t status;

        SmsgTaxinodeStatus() : SmsgTaxinodeStatus(0, 0)
        {
        }

        SmsgTaxinodeStatus(uint64_t guid, uint8_t status) :
            ManagedPacket(SMSG_TAXINODE_STATUS, 0),
            guid(guid),
            status(status)
        {
        }

    protected:

        size_t expectedSize() const override { return 9; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << status;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
