/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgRequestRatedBgInfo : public ManagedPacket
    {
#if VERSION_STRING > WotLK
    public:
        uint8_t type;

        CmsgRequestRatedBgInfo() : CmsgRequestRatedBgInfo(0)
        {
        }

        CmsgRequestRatedBgInfo(uint8_t type) :
            ManagedPacket(CMSG_REQUEST_RATED_BG_INFO, 1),
            type(type)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> type;
            return true;
        }
#endif
    };
}
