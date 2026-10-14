/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgEnumCharacters : public ManagedPacket
    {
    public:
        CmsgEnumCharacters() : ManagedPacket(CMSG_ENUM_CHARACTERS, 0) {}

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            return packet.remaining() == 0;
        }
    };
}
