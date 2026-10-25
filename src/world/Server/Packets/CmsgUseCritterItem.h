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
    // the player learns the pet of an item
    class CmsgUseCritterItem : public ManagedPacket
    {
    public:
        uint64_t itemGuid = 0;

        CmsgUseCritterItem() : ManagedPacket(CMSG_USE_CRITTER_ITEM, 1)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                itemGuid = BattlePetWire::readObjectGuid(packet);
                return true;
            }

            if (m_protocol.isMop())
            {
                WoWGuid guid;
                for (const uint8_t bit : std::array<uint8_t, 8>{ 7, 1, 0, 5, 4, 2, 3, 6 })
                    guid[bit] = packet.readBit();
                for (const uint8_t byte : std::array<uint8_t, 8>{ 2, 3, 4, 1, 5, 7, 0, 6 })
                    packet.readByteSeq(guid[byte]);

                itemGuid = guid.getRawGuid();
                return true;
            }

            return false;
        }
    };
}
