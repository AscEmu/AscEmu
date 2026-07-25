/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgEquipmentSetUseResult : public ManagedPacket
    {
    public:
        uint8_t result;

        SmsgEquipmentSetUseResult() : SmsgEquipmentSetUseResult(0)
        {
        }

        SmsgEquipmentSetUseResult(uint8_t result) :
            ManagedPacket(SMSG_EQUIPMENT_SET_USE_RESULT, 0),
            result(result)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 1;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            packet << result;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
