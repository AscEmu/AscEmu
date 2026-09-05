/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgRequestCategoryCooldowns : public ManagedPacket
    {
    public:
        CmsgRequestCategoryCooldowns() : ManagedPacket(CMSG_REQUEST_CATEGORY_COOLDOWNS, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& /*packet*/) override { return true; }
    };
}
