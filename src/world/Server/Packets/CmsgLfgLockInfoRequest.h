/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgLfgLockInfoRequest : public ManagedPacket
    {
    public:
        bool requestFromPlayer = false;

        CmsgLfgLockInfoRequest() : ManagedPacket(CMSG_LFG_LOCK_INFO_REQUEST, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Dragonflight)
            {
                // 10.x+: player bit, party index bit, party index
                requestFromPlayer = packet.readBit();
                if (packet.readBit())
                    packet.readSkip<uint8_t>();
                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                requestFromPlayer = packet.readBit();
                packet.readSkip<uint8_t>();                     // party index
                return true;
            }
            if (m_protocol.isMop())
            {
                packet.readSkip<uint8_t>();
                requestFromPlayer = packet.readBit();

                return true;
            }
            else if (m_protocol.isCata())
            {
                requestFromPlayer = packet.readBit();

                return true;
            }

            return false;
        }
    };
}
