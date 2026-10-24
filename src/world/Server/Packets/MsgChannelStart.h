/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class MsgChannelStart : public ManagedPacket
    {
    public:
        WoWGuid casterGuid;
        uint32_t spellId;
        uint32_t duration;
        uint32_t spellVisualId {0};             // SpellXSpellVisual row of the channel (7.x and later clients with own data)

        MsgChannelStart() : MsgChannelStart(WoWGuid(), 0, 0)
        {
        }

        MsgChannelStart(WoWGuid casterGuid, uint32_t spellId, uint32_t duration) :
            ManagedPacket(MSG_CHANNEL_START, 0),
            casterGuid(casterGuid),
            spellId(spellId),
            duration(duration)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // caster, spell, visual (7.x and 8.x one value, 9.x and later visual and script visual), duration,
                // no interrupt immunities, no heal prediction
                packet.setOpcode(SMSG_SPELL_CHANNEL_START);
                packet << casterGuid.toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << int32_t(spellId);
                if (!m_protocol.isWoD())
                    packet << int32_t(spellVisualId);
                if (m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
                    packet << int32_t(0);
                packet << uint32_t(duration);
                packet.writeBit(false);
                packet.writeBit(false);
                packet.flushBits();
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_TBC)
                packet << casterGuid;

            packet << spellId << duration;

            if (m_protocol.expansion >= WoW::Expansion::_Cata)
                packet << uint8_t(0) << uint8_t(0);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
