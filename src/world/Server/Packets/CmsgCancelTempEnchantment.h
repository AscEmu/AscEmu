/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
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
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet >> inventorySlot;
                return true;
            }

            return false;
        }
    };
}
