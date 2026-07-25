/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLfgUpdateSearch : public ManagedPacket
    {
    public:
        uint8_t update;

        SmsgLfgUpdateSearch() : SmsgLfgUpdateSearch(0)
        {
        }

        SmsgLfgUpdateSearch(uint8_t update) :
            ManagedPacket(SMSG_LFG_UPDATE_SEARCH, 0),
            update(update)
        {
        }

    protected:

        size_t expectedSize() const override { return 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            packet << update;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
