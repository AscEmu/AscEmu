/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSupercededSpell : public ManagedPacket
    {
    public:
        uint32_t spellId;
        uint32_t supercedeSpellId;

        SmsgSupercededSpell() : SmsgSupercededSpell(0, 0)
        {
        }

        SmsgSupercededSpell(uint32_t spellId, uint32_t supercedeSpellId) :
            ManagedPacket(SMSG_SUPERCEDED_SPELL, 0),
            spellId(spellId),
            supercedeSpellId(supercedeSpellId)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_protocol.expansion < WoW::Expansion::_WotLK ? 4 : 8;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isDragonflight())
            {
                // 10.x: the new spell with its bits, the superceded spell behind them
                packet << uint32_t(1);
                packet << int32_t(supercedeSpellId);
                packet.writeBit(false);                 // favorite
                packet.writeBit(false);
                packet.writeBit(true);                  // has a superceded spell
                packet.writeBit(false);                 // trait definition
                packet.flushBits();
                packet << int32_t(spellId);
                return true;
            }

            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight())
            {
                // new spells, superceded spells, favorite spells (7.x)
                packet << uint32_t(1);
                packet << uint32_t(1);
                if (m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight())
                    packet << uint32_t(0);
                packet << int32_t(supercedeSpellId);
                packet << int32_t(spellId);
                return true;
            }

            if (m_protocol.expansion < WoW::Expansion::_WotLK)
            {
                packet << static_cast<uint16_t>(spellId);
                packet << static_cast<uint16_t>(supercedeSpellId);
            }
            else if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << spellId << supercedeSpellId;
            }
            else
            {
                packet << supercedeSpellId << spellId;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
