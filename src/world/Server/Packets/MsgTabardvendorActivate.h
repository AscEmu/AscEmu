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
    class MsgTabardvendorActivate : public ManagedPacket
    {
    public:
        WoWGuid guid;

        MsgTabardvendorActivate() : MsgTabardvendorActivate(0)
        {
        }

        MsgTabardvendorActivate(uint64_t guid) :
            ManagedPacket(MSG_TABARDVENDOR_ACTIVATE, 8),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid.getRawGuid();
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.Init(unpackedGuid);
            return true;
        }
    };
}
