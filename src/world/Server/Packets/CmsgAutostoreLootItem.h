/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgAutostoreLootItem : public ManagedPacket
    {
    public:
        uint8_t slot;

        CmsgAutostoreLootItem() : CmsgAutostoreLootItem(0)
        {
        }

        CmsgAutostoreLootItem(uint8_t slot) :
            ManagedPacket(CMSG_AUTOSTORE_LOOT_ITEM, 0),
            slot(slot)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> slot;
            return true;
        }
    };
}
