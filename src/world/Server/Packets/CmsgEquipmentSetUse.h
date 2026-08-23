/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/ItemDefines.hpp"
#include <array>
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgEquipmentSetUse : public ManagedPacket
    {
    public:
        std::array<int8_t, EQUIPMENT_SLOT_END> srcBag{};
        std::array<uint8_t, EQUIPMENT_SLOT_END> srcSlot{};
        std::array<WoWGuid, EQUIPMENT_SLOT_END> itemGuid{};

        CmsgEquipmentSetUse() :
            ManagedPacket(CMSG_EQUIPMENT_SET_USE, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                for (uint32_t i = 0; i < EQUIPMENT_SLOT_END; ++i)
                {
                    uint8_t slot;
                    int8_t bag;
                    packet >> slot >> bag;
                    srcSlot[i] = slot;
                    srcBag[i] = bag;
                }

                for (uint32_t i = 0; i < EQUIPMENT_SLOT_END; ++i)
                {
                    itemGuid[i][3] = packet.readBit();
                    itemGuid[i][1] = packet.readBit();
                    itemGuid[i][7] = packet.readBit();
                    itemGuid[i][4] = packet.readBit();
                    itemGuid[i][5] = packet.readBit();
                    itemGuid[i][6] = packet.readBit();
                    itemGuid[i][0] = packet.readBit();
                    itemGuid[i][2] = packet.readBit();
                }

                const uint32_t inventoryItemCounter = static_cast<uint32_t>(packet.readBits(2));
                for (uint32_t i = 0; i < inventoryItemCounter; ++i)
                {
                    packet.readBit(); // Container Slot, unused
                    packet.readBit(); // Slot, unused
                }

                for (uint32_t i = 0; i < EQUIPMENT_SLOT_END; ++i)
                {
                    packet.readByteSeq(itemGuid[i][4]);
                    packet.readByteSeq(itemGuid[i][7]);
                    packet.readByteSeq(itemGuid[i][0]);
                    packet.readByteSeq(itemGuid[i][3]);
                    packet.readByteSeq(itemGuid[i][2]);
                    packet.readByteSeq(itemGuid[i][5]);
                    packet.readByteSeq(itemGuid[i][1]);
                    packet.readByteSeq(itemGuid[i][6]);
                }

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                for (uint32_t i = 0; i < EQUIPMENT_SLOT_END; ++i)
                {
                    WoWGuid guid;
                    packet >> guid;
                    itemGuid[i] = guid;

                    int8_t bag;
                    uint8_t slot;
                    packet >> bag >> slot;
                    srcBag[i] = bag;
                    srcSlot[i] = slot;
                }

                return true;
            }

            return false;
        }
    };
}
