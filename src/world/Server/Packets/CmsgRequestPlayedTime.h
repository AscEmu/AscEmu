/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

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
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << displayInChatFrame;
            return true;
        }

        bool internalDeserialise([[maybe_unused]]WorldPacket& packet) override
        {
#if VERSION_STRING > TBC
            packet >> displayInChatFrame;
#endif
            return true;
        }
    };
}
