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
    class CmsgLearnTalent : public ManagedPacket
    {
    public:
        uint32_t talentId;
        uint32_t requestedRank;

        CmsgLearnTalent() : CmsgLearnTalent(0, 0)
        {
        }

        CmsgLearnTalent(uint32_t talentId, uint32_t requestedRank) :
            ManagedPacket(CMSG_LEARN_TALENT, 4),
            talentId(talentId),
            requestedRank(requestedRank)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> talentId >> requestedRank;
            return true;
        }
    };
}}
