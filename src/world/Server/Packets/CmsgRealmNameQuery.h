/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgRealmNameQuery : public ManagedPacket
    {
    public:
        uint32_t realmId = 0;

        CmsgRealmNameQuery() : ManagedPacket(CMSG_REALM_NAME_QUERY, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet >> realmId;
            return true;
        }
    };
}
