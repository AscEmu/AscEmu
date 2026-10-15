/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgBinderConfirm : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t zoneId;

        SmsgBinderConfirm() : SmsgBinderConfirm(0, 0)
        {
        }

        SmsgBinderConfirm(uint64_t guid, uint32_t zoneId) :
            ManagedPacket(SMSG_BINDER_CONFIRM, 12),
            guid(guid),
            zoneId(zoneId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // only the binder, the zone comes from the bind point update
                WoWGuid wowGuid(guid);
                packet.writeBit(wowGuid[4]);
                packet.writeBit(wowGuid[6]);
                packet.writeBit(wowGuid[2]);
                packet.writeBit(wowGuid[1]);
                packet.writeBit(wowGuid[5]);
                packet.writeBit(wowGuid[3]);
                packet.writeBit(wowGuid[0]);
                packet.writeBit(wowGuid[7]);
                packet.flushBits();
                packet.writeByteSeq(wowGuid[6]);
                packet.writeByteSeq(wowGuid[2]);
                packet.writeByteSeq(wowGuid[5]);
                packet.writeByteSeq(wowGuid[0]);
                packet.writeByteSeq(wowGuid[4]);
                packet.writeByteSeq(wowGuid[7]);
                packet.writeByteSeq(wowGuid[1]);
                packet.writeByteSeq(wowGuid[3]);
                return true;
            }

            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 10.x and later open the window through the interaction result
                if (m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
                {
                    packet.setOpcode(SMSG_NPC_INTERACTION_OPEN_RESULT);
                    packet << WoWGuid(guid).toGuid128(m_protocol.realmId, m_receiverMapId);
                    packet << int32_t(20);
                    packet.writeBit(true);                              // success
                    packet.flushBits();
                    return true;
                }

                packet << WoWGuid(guid).toGuid128(m_protocol.realmId, m_receiverMapId);
                return true;
            }

            packet << guid << zoneId;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
