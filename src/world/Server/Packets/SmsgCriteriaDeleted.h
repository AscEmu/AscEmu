/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgCriteriaDeleted : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t criteriaId;

        SmsgCriteriaDeleted() : SmsgCriteriaDeleted(0)
        {
        }

        SmsgCriteriaDeleted(uint32_t criteriaId) :
            ManagedPacket(SMSG_CRITERIA_DELETED, 0),
            criteriaId(criteriaId)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << criteriaId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
