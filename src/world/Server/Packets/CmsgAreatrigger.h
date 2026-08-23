/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgAreatrigger : public ManagedPacket
    {
    public:
        uint32_t triggerId;

        CmsgAreatrigger() : CmsgAreatrigger(0)
        {
        }

        CmsgAreatrigger(uint32_t triggerId) :
            ManagedPacket(CMSG_AREATRIGGER, 4),
            triggerId(triggerId)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet >> triggerId;
                return true;
            }

            return false;
        }
    };
}
