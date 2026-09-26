/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgTrainerBuySpell : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t trainerId = 0;
        uint32_t spellId;

        CmsgTrainerBuySpell() : CmsgTrainerBuySpell(0, 0)
        {
        }

        CmsgTrainerBuySpell(uint64_t guid, uint32_t spellId) :
            ManagedPacket(CMSG_TRAINER_BUY_SPELL, 12),
            guid(guid),
            spellId(spellId)
        {
        }

        bool deserialise(WorldPacket& packet) override
        {
            if (packet.remaining() < expectedSize())
                return false;

            return internalDeserialise(packet);
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.isForever())
                return 9; // packed modern GUID + trainerId + spellId
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
                return m_minimum_size;
            else if (m_protocol.isMop())
                return 9; // uint32 spellId + uint32 trainerId + packed guid mask byte
            return 0;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                WoWGuid modernGuid;
                std::size_t consumed = 0;
                if (!WoWGuid::unpackModern(packet.contents() + packet.rpos(), packet.remaining(), modernGuid, consumed))
                    return false;
                packet.rpos(packet.rpos() + consumed);
                guid.init(modernGuid.toLegacyRaw());

                if (packet.remaining() != sizeof(uint32_t) * 2)
                    return false;
                packet >> trainerId >> spellId;
                return true;
            }

            if (m_protocol.isMop())
            {
                packet >> spellId;
                packet >> trainerId;

                WoWGuid unpackedGuid;
                unpackedGuid[1] = packet.readBit();
                unpackedGuid[4] = packet.readBit();
                unpackedGuid[0] = packet.readBit();
                unpackedGuid[6] = packet.readBit();
                unpackedGuid[3] = packet.readBit();
                unpackedGuid[2] = packet.readBit();
                unpackedGuid[5] = packet.readBit();
                unpackedGuid[7] = packet.readBit();

                packet.readByteSeq(unpackedGuid[3]);
                packet.readByteSeq(unpackedGuid[1]);
                packet.readByteSeq(unpackedGuid[4]);
                packet.readByteSeq(unpackedGuid[7]);
                packet.readByteSeq(unpackedGuid[0]);
                packet.readByteSeq(unpackedGuid[5]);
                packet.readByteSeq(unpackedGuid[6]);
                packet.readByteSeq(unpackedGuid[2]);

                guid = unpackedGuid;
                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;

                if (m_protocol.expansion > WoW::Expansion::_WotLK)
                {
                    packet >> trainerId;
                }

                packet >> spellId;
                guid.init(unpackedGuid);
                return true;
            }

            return false;
        }
    };
}
