/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "BattlePetPacketData.hpp"

#include <array>
#include <cstdint>

namespace AscEmu::Packets
{
    // the client packets that name only a pet: delete, summon, cage, clear fanfare, update notify
    class CmsgBattlePetGuid : public ManagedPacket
    {
    public:
        uint32_t petId = 0;

        explicit CmsgBattlePetGuid(uint16_t opcode) : ManagedPacket(opcode, 1)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                petId = BattlePetWire::readGuid(packet);
                return true;
            }

            if (!m_protocol.isMop())
                return false;

            // 5.x: every opcode masks the guid in its own order
            using Order = std::array<uint8_t, 8>;
            Order bits;
            Order bytes;
            switch (m_opcode)
            {
                case CMSG_BATTLE_PET_DELETE_PET:        bits = { 3, 5, 6, 2, 4, 0, 7, 1 }; bytes = { 4, 1, 5, 0, 7, 2, 3, 6 }; break;
                case CMSG_BATTLE_PET_SUMMON_COMPANION:  bits = { 3, 2, 5, 0, 7, 1, 6, 4 }; bytes = { 6, 7, 3, 5, 0, 4, 1, 2 }; break;
                case CMSG_CAGE_BATTLE_PET:              bits = { 5, 1, 4, 3, 0, 6, 2, 7 }; bytes = { 5, 4, 2, 7, 1, 3, 6, 0 }; break;
                default:
                    return false;
            }

            WoWGuid guid;
            for (const uint8_t bit : bits)
                guid[bit] = packet.readBit();
            for (const uint8_t byte : bytes)
                packet.readByteSeq(guid[byte]);

            petId = static_cast<uint32_t>(guid.getRawGuid());
            return true;
        }
    };
}
