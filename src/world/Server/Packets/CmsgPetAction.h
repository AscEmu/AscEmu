/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "Objects/Units/Creatures/PetDefines.hpp"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgPetAction : public ManagedPacket
    {
    public:
        WoWGuid guid;
        PetActionButtonData buttonData;
        uint64_t targetguid;

        CmsgPetAction() : CmsgPetAction(0, PetActionButtonData{ .raw = 0 }, 0)
        {
        }

        CmsgPetAction(uint64_t guid, PetActionButtonData buttonData, uint64_t targetguid) :
            ManagedPacket(CMSG_PET_ACTION, 20),
            guid(guid),
            buttonData(buttonData),
            targetguid(targetguid)
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
            packet >> unpacked_guid >> buttonData.raw >> targetguid;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}
