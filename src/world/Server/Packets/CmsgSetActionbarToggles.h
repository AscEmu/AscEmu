/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSetActionbarToggles : public ManagedPacket
    {
    public:
        uint8_t actionbarId;

        CmsgSetActionbarToggles() : CmsgSetActionbarToggles(0)
        {
        }

        CmsgSetActionbarToggles(uint8_t actionbarId) :
            ManagedPacket(CMSG_SET_ACTIONBAR_TOGGLES, 0),
            actionbarId(actionbarId)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> actionbarId;
            return true;
        }
    };
}
