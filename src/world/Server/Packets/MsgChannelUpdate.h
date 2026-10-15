/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class MsgChannelUpdate : public ManagedPacket
    {
    public:
        WoWGuid casterGuid;
        uint32_t time;

        MsgChannelUpdate() : MsgChannelUpdate(WoWGuid(), 0)
        {
        }

        MsgChannelUpdate(WoWGuid casterGuid, uint32_t time) :
            ManagedPacket(MSG_CHANNEL_UPDATE, 0),
            casterGuid(casterGuid),
            time(time)
        {
        }

    protected:
        size_t expectedSize() const override { return static_cast<size_t>(8 + 4); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // caster, remaining time, 12.x: the unit that interrupted the channel
                packet.setOpcode(SMSG_SPELL_CHANNEL_UPDATE);
                packet << casterGuid.toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << int32_t(time);
                if (m_protocol.isMidnight())
                    packet << WoWGuid128();
                return true;
            }

            packet << casterGuid << time;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
