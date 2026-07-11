/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgRequestPartyMemberStats : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgRequestPartyMemberStats() : CmsgRequestPartyMemberStats(0)
        {
        }

        CmsgRequestPartyMemberStats(uint64_t guid) :
            ManagedPacket(CMSG_REQUEST_PARTY_MEMBER_STATS, 8),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid;
            guid.init(unpacked_guid);
            return true;
        }
    };
}
