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
        bool entered;
        bool fromClient;

        CmsgAreatrigger() : CmsgAreatrigger(0)
        {
        }

        CmsgAreatrigger(uint32_t triggerId) :
            ManagedPacket(CMSG_AREATRIGGER, 4),
            triggerId(triggerId),
            entered(true),
            fromClient(true)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                // Forever 1.60.x: uint32 AreaTriggerID followed by the
                // Entered and FromClient bits. Captured payloads are 5 bytes.
                if (packet.remaining() < 5)
                    return false;

                packet >> triggerId;
                entered = packet.readBit();
                fromClient = packet.readBit();
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet >> triggerId;
                entered = true;
                fromClient = true;
                return true;
            }

            return false;
        }
    };
}
