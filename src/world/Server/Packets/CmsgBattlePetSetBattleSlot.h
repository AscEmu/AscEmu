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
    class CmsgBattlePetSetBattleSlot : public ManagedPacket
    {
    public:
        uint32_t petId = 0;
        uint8_t slot = 0;

        CmsgBattlePetSetBattleSlot() : ManagedPacket(CMSG_BATTLE_PET_SET_BATTLE_SLOT, 2)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                petId = BattlePetWire::readGuid(packet);
                packet >> slot;
                return true;
            }

            if (m_protocol.isMop())
            {
                packet >> slot;

                WoWGuid guid;
                for (const uint8_t bit : std::array<uint8_t, 8>{ 4, 6, 5, 7, 3, 1, 0, 2 })
                    guid[bit] = packet.readBit();
                for (const uint8_t byte : std::array<uint8_t, 8>{ 1, 3, 5, 0, 7, 6, 4, 2 })
                    packet.readByteSeq(guid[byte]);

                petId = static_cast<uint32_t>(guid.getRawGuid());
                return true;
            }

            return false;
        }
    };
}
