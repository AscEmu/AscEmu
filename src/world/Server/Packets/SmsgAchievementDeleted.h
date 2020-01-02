/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgAchievementDeleted : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t achivementId;

        SmsgAchievementDeleted() : SmsgAchievementDeleted(0)
        {
        }

        SmsgAchievementDeleted(uint32_t achivementId) :
            ManagedPacket(SMSG_ACHIEVEMENT_DELETED, 0),
            achivementId(achivementId)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << achivementId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
