/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSetWatchedFaction : public ManagedPacket
    {
    public:
        uint32_t factionId;

        CmsgSetWatchedFaction() : CmsgSetWatchedFaction(0)
        {
        }

        CmsgSetWatchedFaction(uint32_t factionId) :
            ManagedPacket(CMSG_SET_WATCHED_FACTION, 0),
            factionId(factionId)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet >> factionId;
                return true;
            }

            return false;
        }
    };
}
