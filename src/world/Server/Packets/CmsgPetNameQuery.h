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
    class CmsgPetNameQuery : public ManagedPacket
    {
    public:
        uint32_t petNumber;
        WoWGuid guid;

        CmsgPetNameQuery() : CmsgPetNameQuery(0, 0)
        {
        }

        CmsgPetNameQuery(uint64_t guid, uint32_t petNumber) :
            ManagedPacket(CMSG_PET_NAME_QUERY, 8),
            petNumber(petNumber),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> petNumber >> unpacked_guid;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
