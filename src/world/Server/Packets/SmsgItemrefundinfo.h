/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgItemrefundinfo : public ManagedPacket
    {
    public:
        uint64_t itemGuid = 0;
        uint32_t buyPrice = 0;
        // WotLK specific
        uint32_t honorPoints = 0;
        uint32_t arenaPoints = 0;
        uint32_t extendedCostItem[5] = {};
        uint32_t extendedCostCount[5] = {};
        // Cata+ specific
        uint32_t extendedCostReqCurrCount[5] = {};
        uint32_t extendedCostReqCur[5] = {};
        uint32_t refundTime = 0;

        SmsgItemrefundinfo() : ManagedPacket(SMSG_ITEMREFUNDINFO, 68)
        {
        }

        // WotLK
        SmsgItemrefundinfo(uint64_t itemGuid, uint32_t buyPrice, uint32_t honorPoints, uint32_t arenaPoints,
            uint32_t const (&extendedCostItem)[5], uint32_t const (&extendedCostCount)[5], uint32_t refundTime) :
            ManagedPacket(SMSG_ITEMREFUNDINFO, 68),
            itemGuid(itemGuid), buyPrice(buyPrice), honorPoints(honorPoints), arenaPoints(arenaPoints), refundTime(refundTime)
        {
            for (uint8_t i = 0; i < 5; ++i)
            {
                this->extendedCostItem[i] = extendedCostItem[i];
                this->extendedCostCount[i] = extendedCostCount[i];
            }
        }

        // Cata+
        SmsgItemrefundinfo(uint64_t itemGuid, uint32_t buyPrice, uint32_t const (&extendedCostItem)[5], uint32_t const (&extendedCostCount)[5],
            uint32_t const (&extendedCostReqCurrCount)[5], uint32_t const (&extendedCostReqCur)[5], uint32_t refundTime) :
            ManagedPacket(SMSG_ITEMREFUNDINFO, 68),
            itemGuid(itemGuid), buyPrice(buyPrice), refundTime(refundTime)
        {
            for (uint8_t i = 0; i < 5; ++i)
            {
                this->extendedCostItem[i] = extendedCostItem[i];
                this->extendedCostCount[i] = extendedCostCount[i];
                this->extendedCostReqCurrCount[i] = extendedCostReqCurrCount[i];
                this->extendedCostReqCur[i] = extendedCostReqCur[i];
            }
        }

    protected:
        size_t expectedSize() const override { return 68; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWotlk())
            {
                packet << itemGuid;
                packet << buyPrice;
                packet << honorPoints;
                packet << arenaPoints;

                for (uint8_t i = 0; i < 5; ++i)
                {
                    packet << extendedCostItem[i];
                    packet << extendedCostCount[i];
                }

                packet << uint32_t(0);
                packet << refundTime;
            }
            else if (m_protocol.isCata())
            {
                WoWGuid objectGuid = itemGuid;
                packet.writeBit(objectGuid[3]);
                packet.writeBit(objectGuid[5]);
                packet.writeBit(objectGuid[7]);
                packet.writeBit(objectGuid[6]);
                packet.writeBit(objectGuid[2]);
                packet.writeBit(objectGuid[4]);
                packet.writeBit(objectGuid[0]);
                packet.writeBit(objectGuid[1]);
                packet.flushBits();
                packet.writeByteSeq(objectGuid[7]);

                packet << refundTime;

                for (uint8_t i = 0; i < 5; ++i)
                {
                    packet << extendedCostItem[i];
                    packet << extendedCostCount[i];
                }

                packet.writeByteSeq(objectGuid[6]);
                packet.writeByteSeq(objectGuid[4]);
                packet.writeByteSeq(objectGuid[3]);
                packet.writeByteSeq(objectGuid[2]);
                for (uint8_t i = 0; i < 5; ++i)
                {
                    packet << extendedCostReqCurrCount[i];
                    packet << extendedCostReqCur[i];
                }

                packet.writeByteSeq(objectGuid[1]);
                packet.writeByteSeq(objectGuid[5]);
                packet << uint32_t(0);
                packet.writeByteSeq(objectGuid[0]);
                packet << buyPrice;
            }
            else if (m_protocol.isMop())
            {
                WoWGuid objectGuid = itemGuid;
                packet.writeBit(objectGuid[1]);
                packet.writeBit(objectGuid[5]);
                packet.writeBit(objectGuid[3]);
                packet.writeBit(objectGuid[0]);
                packet.writeBit(objectGuid[7]);
                packet.writeBit(objectGuid[2]);
                packet.writeBit(objectGuid[3]);
                packet.writeBit(objectGuid[4]);
                packet.flushBits();

                packet << refundTime;

                // item cost data - order is count, item (swapped vs. pre-Mop)
                for (uint8_t i = 0; i < 5; ++i)
                {
                    packet << extendedCostCount[i];
                    packet << extendedCostItem[i];
                }

                packet.writeByteSeq(objectGuid[1]);
                packet.writeByteSeq(objectGuid[6]);
                packet.writeByteSeq(objectGuid[4]);
                packet << uint32_t(0);           // unknown flags, always 0 in reference
                packet.writeByteSeq(objectGuid[5]);
                packet << buyPrice;              // money cost

                for (uint8_t i = 0; i < 5; ++i)
                {
                    packet << extendedCostReqCurrCount[i];
                    packet << extendedCostReqCur[i];
                }

                packet.writeByteSeq(objectGuid[3]);
                packet.writeByteSeq(objectGuid[7]);
                packet.writeByteSeq(objectGuid[2]);
                packet.writeByteSeq(objectGuid[0]);
            }
            else
            {
                return false;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
