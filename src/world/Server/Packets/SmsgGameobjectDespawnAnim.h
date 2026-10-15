/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgGameobjectDespawnAnim : public ManagedPacket
    {
    public:
        uint64_t guid;

        SmsgGameobjectDespawnAnim() : SmsgGameobjectDespawnAnim(0)
        {
        }

        SmsgGameobjectDespawnAnim(uint64_t guid) :
            ManagedPacket(SMSG_GAMEOBJECT_DESPAWN_ANIM, 0),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                WoWGuid wowGuid(guid);
                packet.writeBit(wowGuid[0]);
                packet.writeBit(wowGuid[2]);
                packet.writeBit(wowGuid[4]);
                packet.writeBit(wowGuid[1]);
                packet.writeBit(wowGuid[7]);
                packet.writeBit(wowGuid[3]);
                packet.writeBit(wowGuid[6]);
                packet.writeBit(wowGuid[5]);
                packet.flushBits();
                packet.writeByteSeq(wowGuid[0]);
                packet.writeByteSeq(wowGuid[2]);
                packet.writeByteSeq(wowGuid[4]);
                packet.writeByteSeq(wowGuid[5]);
                packet.writeByteSeq(wowGuid[7]);
                packet.writeByteSeq(wowGuid[3]);
                packet.writeByteSeq(wowGuid[1]);
                packet.writeByteSeq(wowGuid[6]);
                return true;
            }

            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                packet << WoWGuid(guid).toGuid128(m_protocol.realmId, m_receiverMapId);
                return true;
            }

            packet << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
