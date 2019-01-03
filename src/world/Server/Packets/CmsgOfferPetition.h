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
    class CmsgOfferPetition : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        WoWGuid playerGuid;

        CmsgOfferPetition() : CmsgOfferPetition(0, 0)
        {
        }

        CmsgOfferPetition(uint64_t itemGuid, uint64_t playerGuid) :
            ManagedPacket(CMSG_OFFER_PETITION, 20),
            itemGuid(itemGuid),
            playerGuid(playerGuid)
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
            packet.read_skip<uint32_t>();
            packet >> itemGuid >> unpackedGuid;
            playerGuid.Init(unpackedGuid);
            return true;
        }
    };
}}
