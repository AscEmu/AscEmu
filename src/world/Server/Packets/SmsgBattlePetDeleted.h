/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "BattlePetPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // a pet left the journal
    class SmsgBattlePetDeleted : public ManagedPacket
    {
    public:
        uint32_t petId = 0;

        explicit SmsgBattlePetDeleted(uint32_t petId) : ManagedPacket(SMSG_BATTLE_PET_DELETED, 18), petId(petId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                BattlePetWire::writeGuid(packet, petId);
                return true;
            }

            if (m_protocol.isMop())
            {
                const WoWGuid guid(static_cast<uint64_t>(petId));
                packet.writeBit(guid[0]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[3]);
                packet.flushBits();

                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[2]);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
