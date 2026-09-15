/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLootRemoved : public ManagedPacket
    {
    public:
        uint8_t slot;
        // guid of the looted object, only the Mop client needs it to find the loot window
        WoWGuid guid;

        SmsgLootRemoved() : SmsgLootRemoved(0)
        {
        }

        SmsgLootRemoved(uint8_t slot, WoWGuid guid = WoWGuid()) :
            ManagedPacket(SMSG_LOOT_REMOVED, 1),
            slot(slot),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_protocol.isMop() ? size_t(19) : m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << slot;
                return true;
            }
            else if (m_protocol.isMop())
            {
                // the client reads two guids, the looted object and the loot owner, both are the looted object here
                WoWGuid lootGuid = guid;

                packet.writeBit(guid[7]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[2]);
                packet.writeBit(lootGuid[0]);
                packet.writeBit(lootGuid[1]);
                packet.writeBit(lootGuid[2]);
                packet.writeBit(lootGuid[7]);
                packet.writeBit(lootGuid[6]);
                packet.writeBit(lootGuid[5]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[6]);
                packet.writeBit(lootGuid[3]);
                packet.writeBit(lootGuid[4]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[4]);
                packet.flushBits();

                packet.writeByteSeq(lootGuid[1]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(lootGuid[7]);
                packet.writeByteSeq(lootGuid[0]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(lootGuid[5]);
                packet.writeByteSeq(lootGuid[3]);
                packet.writeByteSeq(lootGuid[2]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[1]);
                packet << slot;
                packet.writeByteSeq(lootGuid[6]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(lootGuid[4]);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
