/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgDismissCritter : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgDismissCritter() : CmsgDismissCritter(0)
        {
        }

        CmsgDismissCritter(uint64_t guid) :
            ManagedPacket(CMSG_DISMISS_CRITTER, 8),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid;
            guid.init(unpacked_guid);
            return true;
        }
    };
}
