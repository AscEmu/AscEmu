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
    class CmsgCancelTempEnchantment : public ManagedPacket
    {
    public:
        uint32_t inventorySlot;

        CmsgCancelTempEnchantment() : CmsgCancelTempEnchantment(0)
        {
        }

        CmsgCancelTempEnchantment(uint32_t inventorySlot) :
            ManagedPacket(CMSG_CANCEL_TEMP_ENCHANTMENT, 0),
            inventorySlot(inventorySlot)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> inventorySlot;
            return true;
        }
    };
}}
