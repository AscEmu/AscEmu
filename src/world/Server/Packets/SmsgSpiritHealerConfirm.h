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
    class SmsgSpiritHealerConfirm : public ManagedPacket
    {
    public:
        WoWGuid guid;

        SmsgSpiritHealerConfirm() : SmsgSpiritHealerConfirm(0)
        {
        }

        SmsgSpiritHealerConfirm(uint64_t guid) :
            ManagedPacket(SMSG_SPIRIT_HEALER_CONFIRM, 8),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 10.x and later ask through the gossip of the spirit healer
                if (m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
                    return false;

                packet << guid.toGuid128(m_protocol.realmId, m_receiverMapId);
                return true;
            }

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << guid.getRawGuid();
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(guid[6]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[0]);

                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[1]);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
