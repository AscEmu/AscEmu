/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgGmTicketSystemstatus : public ManagedPacket
    {
    public:
        uint32_t status;

        SmsgGmTicketSystemstatus() : SmsgGmTicketSystemstatus(0)
        {
        }

        SmsgGmTicketSystemstatus(uint32_t status) :
            ManagedPacket(SMSG_GMTICKET_SYSTEMSTATUS, 0),
            status(status)
        {
        }

    protected:

        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << status;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
