/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgDelFriend : public ManagedPacket
    {
    public:
        uint64_t guid;

        CmsgDelFriend() : CmsgDelFriend(0)
        {
        }

        CmsgDelFriend(uint64_t guid) :
            ManagedPacket(CMSG_DEL_FRIEND, 8),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            // 6.x and later: the realm and the 128 bit guid of the contact
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                packet.readSkip<uint32_t>();        // realm of the contact
                WoWGuid128 player;
                packet >> player;
                guid = WoWGuid::fromGuid128(player).getRawGuid();
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet >> guid;
                return true;
            }

            return false;
        }
    };
}
