/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgRequestPlayedTime : public ManagedPacket
    {
    public:
        uint8_t displayInChatFrame;

        CmsgRequestPlayedTime() : CmsgRequestPlayedTime(0)
        {
        }

        CmsgRequestPlayedTime(uint8_t displayInChatFrame) :
            ManagedPacket(CMSG_REQUEST_PLAYED_TIME, 0),
            displayInChatFrame(displayInChatFrame)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                packet >> displayInChatFrame;
                return true;
            }

            return false;
        }
    };
}
