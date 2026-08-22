/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgMinimapPing : public ManagedPacket
    {
    public:
        float posX = 0.0f;
        float posY = 0.0f;

        CmsgMinimapPing() : ManagedPacket(CMSG_MINIMAP_PING, 8)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet >> posY >> posX;
                packet.readSkip<uint8_t>();

                return true;
            }

            return false;
        }
    };
}
