/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgEnabletaxi : public ManagedPacket
    {
    public:
        WoWGuid creatureGuid;

        CmsgEnabletaxi() : CmsgEnabletaxi(0)
        {
        }

        CmsgEnabletaxi(uint64_t creatureGuid) :
            ManagedPacket(CMSG_ENABLETAXI, 8),
            creatureGuid(creatureGuid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                creatureGuid.init(unpackedGuid);
                return true;
            }

            return false;
        }
    };
}
