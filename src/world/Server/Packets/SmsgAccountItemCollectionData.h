/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // 11.x: one account wide item collection (toys, heirlooms, transmogs, warband scenes, ...); the client expects the
    // warband scenes (type 7) behind the character list, nothing is collected yet
    class SmsgAccountItemCollectionData : public ManagedPacket
    {
    public:
        uint8_t collectionType;

        SmsgAccountItemCollectionData() : SmsgAccountItemCollectionData(0) {}

        explicit SmsgAccountItemCollectionData(uint8_t collectionType) :
            ManagedPacket(SMSG_ACCOUNT_ITEM_COLLECTION_DATA, 10),
            collectionType(collectionType)
        {}

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isTheWarWithin())
                return false;

            packet << uint32_t(0);
            packet << collectionType;
            packet << uint32_t(0);                  // items: id, type, 64 bit value, flags
            packet.writeBit(0);
            packet.flushBits();
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
