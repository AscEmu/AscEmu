/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WoWGuid.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLootReleaseResponse : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint8_t response;

        SmsgLootReleaseResponse() : SmsgLootReleaseResponse(0, 0)
        {
        }

        SmsgLootReleaseResponse(uint64_t guid, uint8_t response) :
            ManagedPacket(SMSG_LOOT_RELEASE_RESPONSE, 0),
            guid(guid),
            response(response)
        {
        }

    protected:
        size_t expectedSize() const override { return m_protocol.isMop() ? size_t(18) : size_t(9); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // Mop writes the released guid twice (as two interleaved packed-guid fields) and
                // has no separate success/fail byte - the packet's mere presence signals release.
                WoWGuid lootGuid = guid;
                WoWGuid objGuid = guid;

                packet.writeBit(lootGuid[0]);
                packet.writeBit(lootGuid[7]);
                packet.writeBit(lootGuid[5]);
                packet.writeBit(objGuid[0]);
                packet.writeBit(lootGuid[4]);
                packet.writeBit(lootGuid[6]);
                packet.writeBit(objGuid[1]);
                packet.writeBit(lootGuid[2]);
                packet.writeBit(objGuid[5]);
                packet.writeBit(lootGuid[3]);
                packet.writeBit(objGuid[3]);
                packet.writeBit(objGuid[2]);
                packet.writeBit(objGuid[4]);
                packet.writeBit(lootGuid[1]);
                packet.writeBit(objGuid[6]);
                packet.writeBit(objGuid[7]);

                packet.flushBits();

                packet.writeByteSeq(objGuid[1]);
                packet.writeByteSeq(lootGuid[1]);
                packet.writeByteSeq(objGuid[2]);
                packet.writeByteSeq(objGuid[5]);
                packet.writeByteSeq(lootGuid[5]);
                packet.writeByteSeq(lootGuid[7]);
                packet.writeByteSeq(lootGuid[3]);
                packet.writeByteSeq(objGuid[0]);
                packet.writeByteSeq(lootGuid[2]);
                packet.writeByteSeq(lootGuid[0]);
                packet.writeByteSeq(objGuid[3]);
                packet.writeByteSeq(objGuid[6]);
                packet.writeByteSeq(lootGuid[6]);
                packet.writeByteSeq(objGuid[4]);
                packet.writeByteSeq(lootGuid[4]);
                packet.writeByteSeq(objGuid[7]);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << guid << response;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
