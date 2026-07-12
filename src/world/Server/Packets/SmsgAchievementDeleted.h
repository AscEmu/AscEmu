/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAchievementDeleted : public ManagedPacket
    {
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
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                packet << achivementId;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
