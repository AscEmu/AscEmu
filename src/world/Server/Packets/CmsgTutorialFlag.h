/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgTutorialFlag : public ManagedPacket
    {
    public:
        uint32_t flag;

        CmsgTutorialFlag() : CmsgTutorialFlag(0)
        {
        }

        CmsgTutorialFlag(uint32_t flag) :
            ManagedPacket(CMSG_TUTORIAL_FLAG, 1),
            flag(flag)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight())
            {
                // one opcode for update, clear and reset; only the update carries a flag
                constexpr uint32_t actionUpdate = 0;

                if (packet.readBits(2) != actionUpdate)
                    return false;

                packet >> flag;
                return true;
            }

            packet >> flag;
            return true;
        }
    };
}
