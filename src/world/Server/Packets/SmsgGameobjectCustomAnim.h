/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgGameobjectCustomAnim : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t state;

        SmsgGameobjectCustomAnim() : SmsgGameobjectCustomAnim(0, 0)
        {
        }

        SmsgGameobjectCustomAnim(uint64_t guid, uint32_t state) :
            ManagedPacket(SMSG_GAMEOBJECT_CUSTOM_ANIM, 0),
            guid(guid),
            state(state)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // guid mask with a spare bit before the last part, animation, guid bytes
                WoWGuid wowGuid(guid);
                packet.writeBit(wowGuid[4]);
                packet.writeBit(wowGuid[7]);
                packet.writeBit(wowGuid[1]);
                packet.writeBit(wowGuid[0]);
                packet.writeBit(wowGuid[5]);
                packet.writeBit(wowGuid[3]);
                packet.writeBit(wowGuid[2]);
                packet.writeBit(false);
                packet.writeBit(wowGuid[6]);
                packet.flushBits();

                packet << uint32_t(state);
                packet.writeByteSeq(wowGuid[5]);
                packet.writeByteSeq(wowGuid[6]);
                packet.writeByteSeq(wowGuid[7]);
                packet.writeByteSeq(wowGuid[3]);
                packet.writeByteSeq(wowGuid[4]);
                packet.writeByteSeq(wowGuid[0]);
                packet.writeByteSeq(wowGuid[2]);
                packet.writeByteSeq(wowGuid[1]);
                return true;
            }

            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // object, animation, play as despawn bit
                packet << WoWGuid(guid).toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << uint32_t(state);
                packet.writeBit(false);
                packet.flushBits();
                return true;
            }

            packet << guid << state;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
