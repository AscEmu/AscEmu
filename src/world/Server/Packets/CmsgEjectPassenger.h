/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgEjectPassenger : public ManagedPacket
    {
    public:
        uint64_t guid;

        CmsgEjectPassenger() : CmsgEjectPassenger(0)
        {
        }

        CmsgEjectPassenger(uint64_t guid) :
            ManagedPacket(CMSG_EJECT_PASSENGER, 0),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet >> guid;
                return true;
            }
            else
            {
                return false;
            }
        }
    };
}
