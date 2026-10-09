/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // expansion the server runs, sent to 7.x clients before the world is entered
    class SmsgInitialSetup : public ManagedPacket
    {
    public:
        uint8_t expansionLevel;
        uint8_t expansionTier;

        SmsgInitialSetup() : SmsgInitialSetup(0, 0)
        {
        }

        SmsgInitialSetup(uint8_t expansionLevel, uint8_t expansionTier) :
            ManagedPacket(SMSG_INITIAL_SETUP, 2),
            expansionLevel(expansionLevel),
            expansionTier(expansionTier)
        {
        }

    protected:
        size_t expectedSize() const override { return 2; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD())
            {
                // expansion level and tier, region, raid origin
                constexpr int32_t serverRegionId = 3;
                constexpr uint32_t raidOrigin = 1135753200;

                packet << expansionLevel << expansionTier;
                packet << serverRegionId << raidOrigin;
                return true;
            }

            if (!m_protocol.isLegion() || m_protocol.isBfA())
                return false;

            packet << expansionLevel << expansionTier;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
