/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgCorpseNotInInstance : public ManagedPacket
    {
    public:
        SmsgCorpseNotInInstance() :
            ManagedPacket(SMSG_CORPSE_NOT_IN_INSTANCE, 0)
        {
        }

    protected:
        size_t expectedSize() const override { return 0; }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
