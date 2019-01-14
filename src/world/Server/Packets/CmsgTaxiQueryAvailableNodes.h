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
    class CmsgTaxiQueryAvailableNodes : public ManagedPacket
    {
    public:
        WoWGuid creatureGuid;

        CmsgTaxiQueryAvailableNodes() : CmsgTaxiQueryAvailableNodes(0)
        {
        }

        CmsgTaxiQueryAvailableNodes(uint64_t creatureGuid) :
            ManagedPacket(CMSG_TAXIQUERYAVAILABLENODES, 8),
            creatureGuid(creatureGuid)
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
            packet >> unpackedGuid;
            creatureGuid.Init(unpackedGuid);
            return true;
        }
    };
}}
