/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

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
        uint32_t amount;

        //cata specific
        uint8_t itemType = 0;
        uint32_t vendorSlot = 0;
        WoWGuid containerGuid;

        //mop specific
        uint8_t bagSlot = 0;

        CmsgBuyItem() : CmsgBuyItem(0, 0, 0, 0)
        {
        }

        CmsgBuyItem(uint64_t sourceGuid, uint32_t itemEntry, int32_t slot, uint32_t amount) :
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
            if (m_protocol.isForever())
            {
                auto readModernGuid = [&packet](WoWGuid& guid) { WoWGuid modernGuid; std::size_t consumed = 0; if (!WoWGuid::unpackModern(packet.contents() + packet.rpos(), packet.remaining(), modernGuid, consumed)) return false; packet.rpos(packet.rpos() + consumed); guid.init(modernGuid.toLegacyRaw()); return true; };
                if (!readModernGuid(sourceGuid) || !readModernGuid(containerGuid) || packet.remaining() < sizeof(int32_t) + 1) return false;

                int32_t modernItemEntry = 0;
                packet >> modernItemEntry;
                if (modernItemEntry <= 0) return false;
                itemEntry = static_cast<uint32_t>(modernItemEntry);

                const uint32_t modificationCount = packet.readBits(7);
                packet.readBit();
                if (modificationCount > 64 || packet.remaining() < modificationCount * 5U + 1U) return false;
                for (uint32_t i = 0; i < modificationCount; ++i) { uint8_t modificationType = 0; int32_t modificationValue = 0; packet >> modificationType >> modificationValue; }

                const bool hasItemBonus = packet.readBit();
                for (uint8_t i = 1; i < 8; ++i) packet.readBit();
                if (hasItemBonus)
                {
                    uint8_t context = 0;
                    uint32_t bonusCount = 0;
                    packet >> context >> bonusCount;
                    if (bonusCount > 32 || packet.remaining() < bonusCount * sizeof(int32_t)) return false;
                    for (uint32_t i = 0; i < bonusCount; ++i) { int32_t bonusId = 0; packet >> bonusId; }
                }

                if (packet.remaining() != sizeof(int32_t) * 4) return false;
                int32_t quantity = 0;
                int32_t modernItemType = 0;
                uint32_t modernSlot = 0;
                packet >> quantity >> vendorSlot >> modernSlot >> modernItemType;
                if (quantity <= 0 || modernItemType < 0 || modernItemType > UINT8_MAX) return false;
                amount = static_cast<uint32_t>(quantity);
                slot = static_cast<int32_t>(modernSlot);
                itemType = static_cast<uint8_t>(modernItemType);
                return !packet.hadReadFailure() && packet.remaining() == 0;
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
                uint8_t amountByte = 0; packet >> rawGuid >> itemType >> itemEntry >> slot >> amountByte; amount = amountByte;
                sourceGuid.init(rawGuid);
                return true;
            }
            else if (m_protocol.expansion == WoW::Expansion::_WotLK)
            {
                uint8_t amountByte = 0; packet >> rawGuid >> itemEntry >> slot >> amountByte; amount = amountByte;
                sourceGuid.init(rawGuid);
                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_TBC)
            {
                uint8_t amountByte = 0; packet >> rawGuid >> itemEntry >> amountByte; amount = amountByte;
                sourceGuid.init(rawGuid);
                return true;
            }

            return false;
        }
    };
}
