/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgSetSheathed : public ManagedPacket
    {
    public:
        uint32_t type;

        CmsgSetSheathed() : CmsgSetSheathed(0)
        {
        }

        CmsgSetSheathed(uint32_t type) :
            ManagedPacket(CMSG_SETSHEATHED, sizeof(uint32_t)),
            type(type)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << type;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> type;
            return true;
        }
    };
}}
