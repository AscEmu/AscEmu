/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgRealmSplit : public ManagedPacket
    {
    public:
        uint32_t unknown;

        CmsgRealmSplit() : CmsgRealmSplit(0)
        {
        }

        CmsgRealmSplit(uint32_t unknown) :
            ManagedPacket(CMSG_REALM_SPLIT, 4),
            unknown(unknown)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> unknown;
            return true;
        }
    };
}}
