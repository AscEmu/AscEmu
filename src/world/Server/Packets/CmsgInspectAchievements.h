/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <array>
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgInspectAchievements : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgInspectAchievements() : CmsgInspectAchievements(0)
        {
        }

        CmsgInspectAchievements(uint64_t guid) :
            ManagedPacket(CMSG_QUERY_INSPECT_ACHIEVEMENTS, 2),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                WoWGuid128 player;
                packet >> player;
                guid = WoWGuid::fromGuid128(player);
                return true;
            }

            if (m_protocol.isMop())
            {
                for (uint8_t const bit : std::array<uint8_t, 8>{ 2, 7, 1, 5, 4, 0, 3, 6 })
                    guid[bit] = packet.readBit();
                for (uint8_t const byte : std::array<uint8_t, 8>{ 7, 2, 0, 4, 1, 5, 6, 3 })
                    packet.readByteSeq(guid[byte]);
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                guid.init(packet.unpackGuid());
                return true;
            }

            return false;
        }
    };
}
