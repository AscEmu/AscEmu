/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgUiMapClosed : public ManagedPacket
    {
    public:
        CmsgUiMapClosed() : ManagedPacket(CMSG_UI_MAP_CLOSED, 0) { }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            return packet.remaining() == 0;
        }
    };
}
