/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid >> status;
            guid.Init(unpackedGuid);
            return true;
        }
    };
}}
