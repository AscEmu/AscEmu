/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "AEVersion.hpp"
#include "ManagedPacket.h"

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
#if VERSION_STRING == Classic
        size_t expectedSize() const override { return static_cast<size_t>(4 + 4); }
#elif VERSION_STRING >= TBC
        size_t expectedSize() const override { return static_cast<size_t>(8 + 4 + 4); }
#elif VERSION_STRING >= Cata
        size_t expectedSize() const override { return static_cast<size_t>(8 + 4 + 4 + 1 + 1); }
#endif

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING >= TBC
            packet << casterGuid;
#endif
            
            packet << spellId << duration;

#if VERSION_STRING >= Cata
            packet << uint8_t(0) << uint8_t(0);
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
