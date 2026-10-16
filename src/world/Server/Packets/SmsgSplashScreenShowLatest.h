/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    // 9.x and later: the splash screen to show after the login, 0 for none
    class SmsgSplashScreenShowLatest : public ManagedPacket
    {
    public:
        int32_t splashScreenId;

        SmsgSplashScreenShowLatest() : SmsgSplashScreenShowLatest(0)
        {
        }

        explicit SmsgSplashScreenShowLatest(int32_t splashScreenId) :
            ManagedPacket(SMSG_SPLASH_SCREEN_SHOW_LATEST, 4),
            splashScreenId(splashScreenId)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << splashScreenId;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
