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
    // the client sets or clears flags of a pet (favorite, ability selection)
    class CmsgBattlePetSetFlags : public ManagedPacket
    {
    public:
        uint32_t petId = 0;
        uint32_t flags = 0;
        bool apply = false;

        CmsgBattlePetSetFlags() : ManagedPacket(CMSG_BATTLE_PET_SET_FLAGS, 5)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                petId = BattlePetWire::readGuid(packet);

                // the flags take 16 bit from 10.x on
                if (m_protocol.expansion >= WoW::Expansion::_Dragonflight)
                {
                    uint16_t shortFlags;
                    packet >> shortFlags;
                    flags = shortFlags;
                }
                else
                {
                    packet >> flags;
                }

                apply = packet.readBits(2) == 1;        // 1 applies, 2 removes
                return true;
            }

            if (m_protocol.isMop())
            {
                packet >> flags;

                WoWGuid guid;
                guid[5] = packet.readBit();
                guid[4] = packet.readBit();
                apply = packet.readBits(2) == 3;        // 3 applies, 0 removes
                for (const uint8_t bit : std::array<uint8_t, 6>{ 1, 2, 6, 3, 7, 0 })
                    guid[bit] = packet.readBit();
                for (const uint8_t byte : std::array<uint8_t, 8>{ 4, 0, 7, 3, 1, 6, 2, 5 })
                    packet.readByteSeq(guid[byte]);

                petId = static_cast<uint32_t>(guid.getRawGuid());
                return true;
            }

            return false;
        }
    };
}
