/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLogXpGain : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t normalXp;
        uint32_t restedXp;
        bool isQuestXp;

        SmsgLogXpGain() : SmsgLogXpGain(0, 0, 0, false)
        {
        }

        SmsgLogXpGain(uint64_t guid, uint32_t normalXp, uint32_t restedXp, bool isQuestXp) :
            ManagedPacket(SMSG_LOG_XPGAIN, 1),
            guid(guid),
            normalXp(normalXp),
            restedXp(restedXp),
            isQuestXp(isQuestXp)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 4 + 4 + 4 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands())
            {
                // victim, original amount, reason (kill 0, quest 1), amount, group bonus, refer a friend bonus
                packet << WoWGuid(guid).toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << int32_t(normalXp);
                packet << uint8_t(isQuestXp ? 1 : 0);
                packet << int32_t(normalXp + restedXp);
                packet << float(1.0f);
                packet << uint8_t(0);
                return true;
            }

            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                if (isQuestXp == false)
                    packet << guid << normalXp << uint8_t(0) << restedXp << float(1.0f);
                else
                    packet << uint64_t(0) << normalXp << uint8_t(1) << uint8_t(0);

            }
            else
            {
                WoWGuid victim;
                victim.init(guid);

                packet.writeBit(0);
                packet.writeBit(victim[1]);
                packet.writeBit(victim[2]);
                packet.writeBit(victim[7]);
                packet.writeBit(victim[4]);
                packet.writeBit(victim[3]);
                packet.writeBit(0);
                packet.writeBit(victim[0]);
                packet.writeBit(victim[5]);
                packet.writeBit(victim[6]);
                packet.writeBit(0);
                packet.writeByteSeq(victim[4]);
                packet.writeByteSeq(victim[2]);
                packet << uint8_t(0);
                packet << float(1);
                packet.writeByteSeq(victim[7]);
                packet.writeByteSeq(victim[1]);
                packet.writeByteSeq(victim[3]);
                packet.writeByteSeq(victim[6]);
                packet << uint32_t(normalXp);

                if (!victim.isEmpty())
                    packet << uint32_t(normalXp);

                packet.writeByteSeq(victim[0]);
                packet.writeByteSeq(victim[5]);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
