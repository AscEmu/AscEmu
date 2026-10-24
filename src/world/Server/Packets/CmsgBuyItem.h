/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ItemInstanceHelpers.h"
#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgBuyItem : public ManagedPacket
    {
    public:
        WoWGuid sourceGuid;
        uint32_t itemEntry;
        int32_t slot;
        uint8_t amount;

        //cata specific
        uint8_t itemType = 0;

        //mop specific
        uint8_t bagSlot = 0;

        CmsgBuyItem() : CmsgBuyItem(0, 0, 0, 0)
        {
        }

        CmsgBuyItem(uint64_t sourceGuid, uint32_t itemEntry, int32_t slot, uint8_t amount) :
            ManagedPacket(CMSG_BUY_ITEM, 13),
            sourceGuid(sourceGuid),
            itemEntry(itemEntry),
            slot(slot),
            amount(amount)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // 6.x+: vendor, container, then quantity, vendor slot, bag slot and the item instance; the vendor type and
                // the instance move around between the versions
                const auto expansion = m_protocol.expansion;
                sourceGuid = readClientGuid(packet, m_protocol);
                [[maybe_unused]] const auto containerGuid = readClientGuid(packet, m_protocol);

                uint32_t quantity = 0;
                uint32_t muid = 0;
                uint32_t bag = 0;
                int32_t vendorType = 1;
                if (m_protocol.isWoD())
                {
                    readItemInstance(packet, m_protocol, itemEntry);
                    packet >> quantity >> muid >> bag;
                    vendorType = static_cast<int32_t>(packet.readBits(2));
                }
                else if (m_protocol.isMidnight())
                {
                    readItemInstance(packet, m_protocol, itemEntry);
                    packet >> quantity >> muid >> bag >> vendorType;
                }
                else if (expansion == WoW::Expansion::_TheWarWithin)
                {
                    packet >> quantity >> muid >> bag >> vendorType;
                    readItemInstance(packet, m_protocol, itemEntry);
                }
                else
                {
                    packet >> quantity >> muid >> bag;
                    readItemInstance(packet, m_protocol, itemEntry);
                    if (expansion <= WoW::Expansion::_Shadowlands)
                    {
                        packet.resetBitPos();
                        vendorType = static_cast<int32_t>(packet.readBits(expansion == WoW::Expansion::_Shadowlands ? 3 : 2));
                    }
                }

                slot = static_cast<int32_t>(muid);
                amount = static_cast<uint8_t>(quantity);
                bagSlot = static_cast<uint8_t>(bag);
                itemType = static_cast<uint8_t>(vendorType);
                return true;
            }

            if (m_protocol.isMop())
            {
                uint32_t amount32 = 0;
                packet >> bagSlot >> amount32 >> itemEntry >> slot;
                amount = static_cast<uint8_t>(amount32);

                WoWGuid bagGuid;

                sourceGuid[6] = packet.readBit();
                bagGuid[6] = packet.readBit();
                bagGuid[4] = packet.readBit();
                sourceGuid[4] = packet.readBit();
                itemType = static_cast<uint8_t>(packet.readBits(2));
                sourceGuid[0] = packet.readBit();
                sourceGuid[3] = packet.readBit();
                bagGuid[3] = packet.readBit();
                sourceGuid[7] = packet.readBit();
                sourceGuid[5] = packet.readBit();
                bagGuid[2] = packet.readBit();
                sourceGuid[1] = packet.readBit();
                bagGuid[7] = packet.readBit();
                sourceGuid[2] = packet.readBit();
                bagGuid[1] = packet.readBit();
                bagGuid[0] = packet.readBit();
                bagGuid[5] = packet.readBit();

                packet.readByteSeq(sourceGuid[5]);
                packet.readByteSeq(sourceGuid[0]);
                packet.readByteSeq(bagGuid[3]);
                packet.readByteSeq(bagGuid[1]);
                packet.readByteSeq(bagGuid[6]);
                packet.readByteSeq(sourceGuid[2]);
                packet.readByteSeq(sourceGuid[7]);
                packet.readByteSeq(sourceGuid[6]);
                packet.readByteSeq(bagGuid[0]);
                packet.readByteSeq(bagGuid[5]);
                packet.readByteSeq(sourceGuid[4]);
                packet.readByteSeq(bagGuid[2]);
                packet.readByteSeq(sourceGuid[3]);
                packet.readByteSeq(bagGuid[7]);
                packet.readByteSeq(sourceGuid[1]);
                packet.readByteSeq(bagGuid[4]);
                return true;
            }

            uint64_t rawGuid = 0;

            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet >> rawGuid >> itemType >> itemEntry >> slot >> amount;
                sourceGuid.init(rawGuid);
                return true;
            }
            else if (m_protocol.expansion == WoW::Expansion::_WotLK)
            {
                packet >> rawGuid >> itemEntry >> slot >> amount;
                sourceGuid.init(rawGuid);
                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_TBC)
            {
                packet >> rawGuid >> itemEntry >> amount;
                sourceGuid.init(rawGuid);
                return true;
            }

            return false;
        }
    };
}
