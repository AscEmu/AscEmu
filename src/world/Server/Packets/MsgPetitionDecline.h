/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class MsgPetitionDecline : public ManagedPacket
    {
    public:
        uint64_t itemGuid = 0;
        uint64_t playerGuid;

        MsgPetitionDecline() : MsgPetitionDecline(0)
        {
        }

        MsgPetitionDecline(uint64_t playerGuid) :
            ManagedPacket(MSG_PETITION_DECLINE, 8),
            playerGuid(playerGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // the 6.x and later clients do not handle a decline
                return false;
            }

            packet << playerGuid;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                WoWGuid128 item;
                packet >> item;
                itemGuid = WoWGuid::fromGuid128(item).getRawGuid();
                return true;
            }

            packet >> itemGuid;
            return true;
        }
    };
}
