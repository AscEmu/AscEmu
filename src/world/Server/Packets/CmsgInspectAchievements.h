/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
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
            if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                guid.init(packet.unpackGuid());
                return true;
            }

            return false;
        }
    };
}
