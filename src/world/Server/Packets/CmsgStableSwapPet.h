/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgStableSwapPet : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t petNumber;

        CmsgStableSwapPet() : CmsgStableSwapPet(0, 0)
        {
        }

        CmsgStableSwapPet(uint64_t guid, uint32_t petNumber) :
            ManagedPacket(CMSG_STABLE_SWAP_PET, 12),
            guid(guid),
            petNumber(petNumber)
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
            packet >> unpacked_guid >> petNumber;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
