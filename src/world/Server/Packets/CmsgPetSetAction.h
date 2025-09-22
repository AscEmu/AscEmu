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
    class CmsgPetSetAction : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t srcSlot;
        PetActionButtonData srcButton;
        uint32_t dstSlot;
        PetActionButtonData dstButton;

        CmsgPetSetAction() : CmsgPetSetAction(0, 0, PetActionButtonData{ .raw = 0 })
        {
        }

        CmsgPetSetAction(uint64_t guid, uint32_t slot, PetActionButtonData buttonData) :
            ManagedPacket(CMSG_PET_SET_ACTION, 8),
            guid(guid),
            srcSlot(slot),
            srcButton(buttonData),
            dstSlot(0),
            dstButton{ .raw = 0 }
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
            packet >> unpacked_guid;
            guid.Init(unpacked_guid);

            // Swapping slots
            if (packet.size() == 24)
            {
                packet >> dstSlot;
                packet >> dstButton.raw;
            }

            packet >> srcSlot;
            packet >> srcButton.raw;

            return true;
        }
    };
}
