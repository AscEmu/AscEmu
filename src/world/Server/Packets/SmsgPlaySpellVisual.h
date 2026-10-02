/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPlaySpellVisual : public ManagedPacket
    {
    public:
        uint64_t rawGuid;
        uint32_t visualId;
        uint32_t type;
        uint16_t mapId;

        SmsgPlaySpellVisual() : SmsgPlaySpellVisual(0, 0, 0, 0)
        {
        }

        SmsgPlaySpellVisual(uint64_t rawGuid, uint32_t visualId, uint32_t type, uint16_t mapId = 0) :
            ManagedPacket(SMSG_PLAY_SPELL_VISUAL, 4 + 4 + 4 + 8),
            rawGuid(rawGuid),
            visualId(visualId),
            type(type),
            mapId(mapId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                const WoWGuid modernGuid = WoWGuid::createModernFromLegacy(rawGuid, m_protocol.realmId, mapId, 0);
                const auto packedGuid = modernGuid.packModern();
                packet.append(packedGuid.data(), packedGuid.size());
                packet << visualId << type << uint32_t(0);
                packet.writeBit(false);
                packet.flushBits();
                return true;
            }

            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << rawGuid << visualId;
            }
            else if (m_protocol.expansion == WoW::Expansion::_Cata)
            {
                packet << uint32_t(0) << visualId << type;

                WoWGuid guid = rawGuid;
                packet.writeBit(guid[4]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[6]);

                packet.flushBits();

                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[5]);
            }
            else if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                WoWGuid guid = rawGuid;
                packet.writeBit(guid[4]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[7]);

                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[7]);
                packet << uint32_t(0);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[6]);
                packet << type;
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[3]);
                packet << visualId;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
