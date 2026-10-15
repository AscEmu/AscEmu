/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgGameobjectPagetext : public ManagedPacket
    {
    public:
        uint64_t guid;

        SmsgGameobjectPagetext() : SmsgGameobjectPagetext(0)
        {
        }

        SmsgGameobjectPagetext(uint64_t guid) :
            ManagedPacket(SMSG_GAMEOBJECT_PAGETEXT, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                packet.setOpcode(SMSG_PAGE_TEXT);
                packet << WoWGuid(guid).toGuid128(m_protocol.realmId, m_receiverMapId);
                return true;
            }

            packet << guid;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
