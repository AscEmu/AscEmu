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
