/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgCooldownEvent : public ManagedPacket
    {
    public:
        uint32_t spellId;
        uint64_t playerGuid;

        SmsgCooldownEvent() : SmsgCooldownEvent(0, 0)
        {
        }

        SmsgCooldownEvent(uint32_t spellId, uint64_t playerGuid) :
            ManagedPacket(SMSG_COOLDOWN_EVENT, 0),
            spellId(spellId),
            playerGuid(playerGuid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 8;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << spellId << playerGuid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
