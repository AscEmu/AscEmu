/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPetitionSignResult : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        uint64_t playerGuid;
        uint32_t result;

        SmsgPetitionSignResult() : SmsgPetitionSignResult(0, 0, 0)
        {
        }

        SmsgPetitionSignResult(uint64_t itemGuid, uint64_t playerGuid, uint32_t result) :
            ManagedPacket(SMSG_PETITION_SIGN_RESULTS, 20),
            itemGuid(itemGuid),
            playerGuid(playerGuid),
            result(result)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                packet << WoWGuid(itemGuid).toGuid128(m_protocol.realmId, 0);
                packet << WoWGuid(playerGuid).toGuid128(m_protocol.realmId, 0);
                packet.writeBits(result, 4);
                packet.flushBits();
                return true;
            }

            packet << itemGuid << playerGuid << result;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
