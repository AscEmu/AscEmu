/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    // 7.x and later: the garrisons of the player, none here. 6.x answers only with a garrison.
    class SmsgGetGarrisonInfoResult : public ManagedPacket
    {
    public:
        // 0 horde, 1 alliance
        int32_t factionIndex;

        SmsgGetGarrisonInfoResult() : SmsgGetGarrisonInfoResult(0)
        {
        }

        explicit SmsgGetGarrisonInfoResult(int32_t factionIndex) :
            ManagedPacket(SMSG_GET_GARRISON_INFO_RESULT, 12),
            factionIndex(factionIndex)
        {
        }

    protected:
        size_t expectedSize() const override { return 12; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight())
                packet << int32_t(factionIndex);
            else if (m_protocol.isTheWarWithinOrLater())
                packet << int8_t(factionIndex);
            else
                return false;

            packet << uint32_t(0);              // garrisons
            packet << uint32_t(0);              // follower soft caps
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
