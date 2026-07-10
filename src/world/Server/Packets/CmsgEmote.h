/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgEmote : public ManagedPacket
    {
    public:
        uint32_t emote;

        CmsgEmote() : CmsgEmote(0)
        {
        }

        CmsgEmote(uint32_t emote) :
            ManagedPacket(CMSG_EMOTE, 4),
            emote(emote)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> emote;
            return true;
        }
    };
}
