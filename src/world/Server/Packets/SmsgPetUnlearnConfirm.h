/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgPetUnlearnConfirm : public ManagedPacket
    {
    public:
        uint64_t petGuid;
        uint32_t unlearnCost;

        SmsgPetUnlearnConfirm() : SmsgPetUnlearnConfirm(0, 0)
        {
        }

        SmsgPetUnlearnConfirm(uint64_t petGuid, uint32_t unlearnCost) :
            ManagedPacket(SMSG_PET_UNLEARN_CONFIRM, 0),
            petGuid(petGuid),
            unlearnCost(unlearnCost)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << petGuid << unlearnCost;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
