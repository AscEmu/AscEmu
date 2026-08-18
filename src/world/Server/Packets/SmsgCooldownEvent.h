/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
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
            return sizeof(spellId) + sizeof(playerGuid);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << spellId << playerGuid;

                return true;
            }
            else if (m_protocol.isMop())
            {
                const WoWGuid guid = playerGuid;

                packet.writeBit(guid[4]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[3]);

                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[7]);

                packet << spellId;

                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[0]);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
