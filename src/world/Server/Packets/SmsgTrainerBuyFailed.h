/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ItemInstanceHelpers.h"
#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgTrainerBuyFailed : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t spellId;
        uint32_t reason;

        SmsgTrainerBuyFailed() : SmsgTrainerBuyFailed(0, 0, 0)
        {
        }

        // reason 0 = service unavailable, 1 = not enough money
        SmsgTrainerBuyFailed(uint64_t guid, uint32_t spellId, uint32_t reason) :
            ManagedPacket(SMSG_TRAINER_BUY_FAILED, 8 + 4 + 4),
            guid(guid),
            spellId(spellId),
            reason(reason)
        {
        }

    protected:
        size_t expectedSize() const override { return 18 + 4 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // 6.x+: guid, spell id, reason
                writeClientGuid(packet, m_protocol, m_receiverMapId, guid);
                packet << static_cast<int32_t>(spellId);
                packet << static_cast<int32_t>(reason);
                return true;
            }

            if (m_protocol.isMop())
            {
                const WoWGuid wowGuid = guid;
                packet.writeBit(wowGuid[3]);
                packet.writeBit(wowGuid[0]);
                packet.writeBit(wowGuid[4]);
                packet.writeBit(wowGuid[7]);
                packet.writeBit(wowGuid[6]);
                packet.writeBit(wowGuid[1]);
                packet.writeBit(wowGuid[5]);
                packet.writeBit(wowGuid[2]);
                packet.flushBits();

                packet.writeByteSeq(wowGuid[1]);
                packet.writeByteSeq(wowGuid[2]);
                packet.writeByteSeq(wowGuid[0]);
                packet.writeByteSeq(wowGuid[3]);
                packet.writeByteSeq(wowGuid[4]);
                packet << reason;
                packet.writeByteSeq(wowGuid[5]);
                packet.writeByteSeq(wowGuid[6]);
                packet.writeByteSeq(wowGuid[7]);
                packet << spellId;
                return true;
            }

            if (m_protocol.isCata())
            {
                packet << guid;
                packet << static_cast<int32_t>(spellId);
                packet << static_cast<int32_t>(reason);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
