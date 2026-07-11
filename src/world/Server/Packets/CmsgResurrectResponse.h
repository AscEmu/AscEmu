/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgResurrectResponse : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t status;

        CmsgResurrectResponse() : CmsgResurrectResponse(0, 0)
        {
        }

        CmsgResurrectResponse(uint64_t guid, uint8_t status) :
            ManagedPacket(CMSG_RESURRECT_RESPONSE, 9),
            guid(guid),
            status(status)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid >> status;
            guid.init(unpackedGuid);
            return true;
        }
    };
}
