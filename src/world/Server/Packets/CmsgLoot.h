/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "ForeverSpellPacketUtils.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgLoot : public ManagedPacket
    {
    public:
        uint64_t guid;

        CmsgLoot() : CmsgLoot(0)
        {
        }

        CmsgLoot(uint64_t guid) :
            ManagedPacket(CMSG_LOOT, 8),
            guid(guid)
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
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
                return m_minimum_size;
            else if (m_protocol.isMop() || m_protocol.isForever())
                return 1; // packed guid: mask byte + only the non-zero guid bytes (1..9 bytes)
            return 0;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            { 
                WoWGuid modernGuid; 
                if (!ForeverSpellPacket::readPackedGuid(packet, modernGuid)) 
                    return false;

                guid = modernGuid.toLegacyRaw(); 
                return guid != 0 && packet.remaining() == 0; 
            }

            if (m_protocol.isMop())
            {
                WoWGuid targetGuid;
                targetGuid[4] = packet.readBit();
                targetGuid[5] = packet.readBit();
                targetGuid[2] = packet.readBit();
                targetGuid[7] = packet.readBit();
                targetGuid[0] = packet.readBit();
                targetGuid[1] = packet.readBit();
                targetGuid[3] = packet.readBit();
                targetGuid[6] = packet.readBit();

                packet.readByteSeq(targetGuid[3]);
                packet.readByteSeq(targetGuid[5]);
                packet.readByteSeq(targetGuid[0]);
                packet.readByteSeq(targetGuid[6]);
                packet.readByteSeq(targetGuid[4]);
                packet.readByteSeq(targetGuid[1]);
                packet.readByteSeq(targetGuid[7]);
                packet.readByteSeq(targetGuid[2]);

                guid = targetGuid.getRawGuid();

                return guid != 0;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> guid;

                return guid != 0;
            }

            return false;
        }
    };
}
