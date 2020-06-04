/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSetForceReactions : public ManagedPacket
    {
    public:
        uint32_t mapSize;
        std::map<uint32_t, uint32_t> reactionMap;

        SmsgSetForceReactions(const std::map<uint32_t, uint32_t>& reactionMap) :
            ManagedPacket(SMSG_SET_FORCED_REACTIONS, 4 + reactionMap.size() * 2 * 4),
            mapSize(reactionMap.size()),
            reactionMap(reactionMap)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << mapSize;
            for (const auto& reactions : reactionMap)
                packet << reactions.first << reactions.second;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
