/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgExplorationExperience : public ManagedPacket
    {
    public:
        uint32_t areaId;
        uint32_t experience;

        SmsgExplorationExperience() : SmsgExplorationExperience(0, 0)
        {
        }

        SmsgExplorationExperience(uint32_t areaId, uint32_t experience) :
            ManagedPacket(SMSG_EXPLORATION_EXPERIENCE, 0),
            areaId(areaId),
            experience(experience)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << areaId << experience;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
