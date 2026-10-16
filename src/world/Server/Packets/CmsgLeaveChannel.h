/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgLeaveChannel : public ManagedPacket
    {
    public:
        uint32_t code;
        std::string name;

        CmsgLeaveChannel() : CmsgLeaveChannel(0, "")
        {
        }

        CmsgLeaveChannel(uint32_t code, std::string name) :
            ManagedPacket(CMSG_LEAVE_CHANNEL, 0),
            code(code),
            name(name)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                packet >> code;

                packet.resetBitPos();
                name = packet.readString(packet.readBits(7));
                return true;
            }

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> code >> name;
            }
            else // Mop
            {
                packet >> code;
                const uint32_t nameLength = packet.readBits(7);
                name = packet.readString(nameLength);
            }
            return true;
        }
    };
}
