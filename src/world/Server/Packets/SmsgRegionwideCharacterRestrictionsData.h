/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    // 1.60: the restrictions of the region wide characters, the client expects them behind a filled character list
    class SmsgRegionwideCharacterRestrictionsData : public ManagedPacket
    {
    public:
        std::vector<uint32_t> characters;

        SmsgRegionwideCharacterRestrictionsData() : SmsgRegionwideCharacterRestrictionsData(std::vector<uint32_t>()) {}

        explicit SmsgRegionwideCharacterRestrictionsData(std::vector<uint32_t> lowGuids) :
            ManagedPacket(SMSG_REGIONWIDE_CHARACTER_RESTRICTIONS_DATA, 4 + lowGuids.size() * 27),
            characters(std::move(lowGuids))
        {}

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            // per character: two flags, the guid and two values, none of them restricts anything
            packet << uint32_t(characters.size());
            for (const uint32_t lowGuid : characters)
            {
                packet.writeBit(0);
                packet.writeBit(0);
                packet.flushBits();
                packet << WoWGuid128::realmSpecific(HighGuid128::Player, m_protocol.realmId, lowGuid);
                packet << uint32_t(0);
                packet << uint32_t(10);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
