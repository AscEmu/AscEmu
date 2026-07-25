/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgCriteriaUpdate : public ManagedPacket
    {
    public:
        uint32_t criteriaId;
        uint32_t counter;
        WoWGuid guid;
        uint32_t secsBitField;

        SmsgCriteriaUpdate() : SmsgCriteriaUpdate(0, 0, WoWGuid(), 0)
        {
        }

        SmsgCriteriaUpdate(uint32_t criteriaId, uint32_t counter, WoWGuid guid, uint32_t secsBitField) :
            ManagedPacket(SMSG_CRITERIA_UPDATE, 0),
            criteriaId(criteriaId), counter(counter), guid(guid), secsBitField(secsBitField)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 4 + 8 + 4 + 4 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            packet << criteriaId;

            packet.appendPackGuid(counter);

            packet << guid;
            packet << uint32_t(0);
            packet << secsBitField;
            packet << uint32_t(0);
            packet << uint32_t(0);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
