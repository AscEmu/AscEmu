/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgGuildDelRank : public ManagedPacket
    {
    public:
        uint32_t rankId;

        CmsgGuildDelRank() : CmsgGuildDelRank(0)
        {
        }

        CmsgGuildDelRank(uint32_t rankId) :
            ManagedPacket(CMSG_GUILD_DEL_RANK, 4),
            rankId(rankId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> rankId;
            return true;
        }
    };
}}
