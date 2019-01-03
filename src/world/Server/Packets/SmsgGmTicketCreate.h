/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgGmTicketCreate : public ManagedPacket
    {
    public:
        uint32_t error;

        SmsgGmTicketCreate() : SmsgGmTicketCreate(0)
        {
        }

        SmsgGmTicketCreate(uint32_t error) :
            ManagedPacket(SMSG_GMTICKET_CREATE, 0),
            error(error)
        {
        }

    protected:

        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << error;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
