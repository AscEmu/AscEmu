/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgCriteriaDeleted : public ManagedPacket
    {
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
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            packet << criteriaId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
