/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WoWGuid.hpp"

namespace AscEmu::Packets
{
    class SmsgLootClearMoney : public ManagedPacket
    {
    public:
        // guid of the looted object, only the Mop client needs it to find the loot window
        WoWGuid guid;

        SmsgLootClearMoney() : SmsgLootClearMoney(WoWGuid())
        {
        }

        explicit SmsgLootClearMoney(WoWGuid guid) :
            ManagedPacket(SMSG_LOOT_CLEAR_MONEY, 0),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_protocol.isMop() ? size_t(9) : size_t(16); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 6.x and later (SMSG_COIN_REMOVED): the looted object
                packet << WoWGuid(guid.getRawGuid()).toGuid128(m_protocol.realmId, m_receiverMapId);
                return true;
            }

            if (m_protocol.isMop())
            {
                packet.writeBit(guid[6]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[7]);

                packet.flushBits();

                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[6]);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
