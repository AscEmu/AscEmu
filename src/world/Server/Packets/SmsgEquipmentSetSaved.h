/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgEquipmentSetSaved : public ManagedPacket
    {
    public:
        uint32_t setId;
        uint32_t setGuid;

        SmsgEquipmentSetSaved() : SmsgEquipmentSetSaved(0, 0)
        {
        }

        SmsgEquipmentSetSaved(uint32_t setId, uint32_t setGuid) :
            ManagedPacket(SMSG_EQUIPMENT_SET_SAVED, 4 + 4),
            setId(setId),
            setGuid(setGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion > WoW::Expansion::_TBC && !m_protocol.isMop())
            {
                packet << setId;
                packet << WoWGuid(uint64_t(setGuid));

                return true;
            }
            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
