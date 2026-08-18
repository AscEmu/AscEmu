/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgItemrefundrequest : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        uint32_t error;

        // only valid/sent when error == 0
        uint32_t buyPrice = 0;
        uint32_t honorPoints = 0;
        uint32_t arenaPoints = 0;
        uint32_t extendedCostItem[5] = {};
        uint32_t extendedCostCount[5] = {};
        // Mop specific (currency cost)
        uint32_t extendedCostReqCurrCount[5] = {};
        uint32_t extendedCostReqCur[5] = {};

        SmsgItemrefundrequest() : SmsgItemrefundrequest(0, 1)
        {
        }

        SmsgItemrefundrequest(uint64_t itemGuid, uint32_t error) :
            ManagedPacket(SMSG_ITEMREFUNDREQUEST, sizeof(itemGuid) + sizeof(error) + sizeof(buyPrice) + sizeof(honorPoints)
                + sizeof(arenaPoints) + sizeof(extendedCostItem) + sizeof(extendedCostCount)),
            itemGuid(itemGuid),
            error(error)
        {
        }

        SmsgItemrefundrequest(uint64_t itemGuid, uint32_t error, uint32_t buyPrice, uint32_t honorPoints, uint32_t arenaPoints,
            uint32_t const (&extendedCostItem)[5], uint32_t const (&extendedCostCount)[5]) :
            ManagedPacket(SMSG_ITEMREFUNDREQUEST, sizeof(itemGuid) + sizeof(error) + sizeof(buyPrice) + sizeof(honorPoints)
                + sizeof(arenaPoints) + sizeof(extendedCostItem) + sizeof(extendedCostCount)),
            itemGuid(itemGuid),
            error(error),
            buyPrice(buyPrice),
            honorPoints(honorPoints),
            arenaPoints(arenaPoints)
        {
            for (uint8_t i = 0; i < 5; ++i)
            {
                this->extendedCostItem[i] = extendedCostItem[i];
                this->extendedCostCount[i] = extendedCostCount[i];
            }
            // extendedCostReqCurrCount/extendedCostReqCur stay zero-initialised (Mop currency cost, unused pre-Mop)
        }

        // Mop: also carries the currency-based
        SmsgItemrefundrequest(uint64_t itemGuid, uint32_t error, uint32_t buyPrice, uint32_t honorPoints, uint32_t arenaPoints,
            uint32_t const (&extendedCostItem)[5], uint32_t const (&extendedCostCount)[5],
            uint32_t const (&extendedCostReqCurrCount)[5], uint32_t const (&extendedCostReqCur)[5]) :
            ManagedPacket(SMSG_ITEMREFUNDREQUEST, sizeof(itemGuid) + sizeof(error) + sizeof(buyPrice) + sizeof(honorPoints)
                + sizeof(arenaPoints) + sizeof(extendedCostItem) + sizeof(extendedCostCount)
                + sizeof(extendedCostReqCurrCount) + sizeof(extendedCostReqCur)),
            itemGuid(itemGuid),
            error(error),
            buyPrice(buyPrice),
            honorPoints(honorPoints),
            arenaPoints(arenaPoints)
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
        size_t expectedSize() const override
        {
            return sizeof(itemGuid) + sizeof(error) + sizeof(buyPrice) + sizeof(honorPoints)
                + sizeof(arenaPoints) + sizeof(extendedCostItem) + sizeof(extendedCostCount)
                + sizeof(extendedCostReqCurrCount) + sizeof(extendedCostReqCur);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                WoWGuid objectGuid = itemGuid;
                packet.writeBit(objectGuid[3]);
                packet.writeBit(objectGuid[1]);
                packet.writeBit(objectGuid[5]);
                packet.writeBit(objectGuid[6]);
                packet.writeBit(objectGuid[4]);
                packet.writeBit(objectGuid[0]);
                packet.writeBit(objectGuid[7]);
                packet.writeBit(error == 0);
                packet.writeBit(objectGuid[2]);
                packet.flushBits();

                packet << uint8_t(error);
                packet.writeByteSeq(objectGuid[5]);
                packet.writeByteSeq(objectGuid[3]);

                if (error == 0)
                {
                    // item cost data - order is count, item
                    for (uint8_t i = 0; i < 5; ++i)
                    {
                        packet << extendedCostCount[i];
                        packet << extendedCostItem[i];
                    }

                    // currency cost data - order is count, currency id
                    for (uint8_t i = 0; i < 5; ++i)
                    {
                        packet << extendedCostReqCurrCount[i];
                        packet << extendedCostReqCur[i];
                    }

                    packet << buyPrice;              // money cost
                }

                packet.writeByteSeq(objectGuid[1]);
                packet.writeByteSeq(objectGuid[7]);
                packet.writeByteSeq(objectGuid[4]);
                packet.writeByteSeq(objectGuid[6]);
                packet.writeByteSeq(objectGuid[0]);
                packet.writeByteSeq(objectGuid[2]);
            }
            else
            {
                packet << itemGuid;
                packet << error;

                if (error == 0)
                {
                    packet << buyPrice;
                    packet << honorPoints;
                    packet << arenaPoints;

                    for (uint8_t i = 0; i < 5; ++i)
                    {
                        packet << extendedCostItem[i];
                        packet << extendedCostCount[i];
                    }
                }
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
