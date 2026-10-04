/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class SmsgResurrectRequest : public ManagedPacket
    {
    public:
        uint64_t casterGuid;
        uint32_t stringSize;
        std::string casterName;
        uint8_t isSicknessAffected;
        uint8_t overrideTimer;
        // 4.3.4
        uint32_t spellId;

        SmsgResurrectRequest() : SmsgResurrectRequest(0, "", 0, 0)
        {
        }

        SmsgResurrectRequest(uint64_t casterGuid, std::string casterName, uint8_t isSicknessAffected, uint8_t overrideTimer = 0, uint32_t spellId = 0) :
            ManagedPacket(SMSG_RESURRECT_REQUEST, 8 + 4 + casterName.size() + 1 + 1 + 1),
            casterGuid(casterGuid),
            stringSize(static_cast<uint32_t>(casterName.size() + 1)),
            casterName(casterName),
            isSicknessAffected(isSicknessAffected),
            overrideTimer(overrideTimer),
            spellId(spellId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion())
            {
                // caster, its realm, pet number, spell, name, use timer, sickness
                packet << WoWGuid(casterGuid).toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << uint32_t(m_protocol.getVirtualRealmAddress());
                packet << uint32_t(0);
                packet << int32_t(spellId);
                packet.writeBits(static_cast<uint32_t>(casterName.length()), m_protocol.isWoD() ? 6 : 11);
                packet.writeBit(overrideTimer == 0);
                packet.writeBit(isSicknessAffected != 0);
                packet.flushBits();
                packet.writeString(casterName);
                return true;
            }

            packet << casterGuid << stringSize << spellId << casterName;
            if (m_protocol.expansion == WoW::Expansion::_Cata)
            {
                packet << uint8_t(0);
            }
            packet << isSicknessAffected;

            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << overrideTimer;
            }
            else
            {
                packet << spellId;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
