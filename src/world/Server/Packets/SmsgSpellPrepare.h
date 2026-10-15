/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // links the cast id of the client to the cast id SMSG_SPELL_START, SMSG_SPELL_GO and the failure packets carry
    class SmsgSpellPrepare : public ManagedPacket
    {
    public:
        WoWGuid128 clientCastGuid;
        WoWGuid casterGuid;
        uint32_t spellId;

        SmsgSpellPrepare() : SmsgSpellPrepare(WoWGuid128(), WoWGuid(), 0)
        {
        }

        SmsgSpellPrepare(WoWGuid128 clientCastGuid, WoWGuid casterGuid, uint32_t spellId) :
            ManagedPacket(SMSG_SPELL_PREPARE, 2 * 18),
            clientCastGuid(clientCastGuid),
            casterGuid(casterGuid),
            spellId(spellId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                packet << clientCastGuid;
                packet << WoWGuid128::cast(m_protocol.realmId, m_receiverMapId, spellId, casterGuid.getLowGuid());
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
