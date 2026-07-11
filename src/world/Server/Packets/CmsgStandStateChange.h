/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgStandStateChange : public ManagedPacket
    {
    public:
        uint8_t state;

        CmsgStandStateChange() : CmsgStandStateChange(0)
        {
        }

        CmsgStandStateChange(uint8_t state) :
            ManagedPacket(CMSG_STANDSTATECHANGE, 4),
            state(state)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> state;
            }
            else //Mop
            {
                uint32_t mopState;
                packet >> mopState;

                state = static_cast<uint8_t>(mopState);
            }
            return true;
        }
    };
}
