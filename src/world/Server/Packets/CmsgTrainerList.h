/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgTrainerList : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgTrainerList() : CmsgTrainerList(0)
        {
        }

        CmsgTrainerList(uint64_t guid) :
            ManagedPacket(CMSG_TRAINER_LIST, 8),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.init(unpackedGuid);
            return true;
        }
    };
}
