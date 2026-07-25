/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPreResurrect : public ManagedPacket
    {
    public:
        uint64_t guid;
        
        SmsgPreResurrect() : SmsgPreResurrect(0)
        {
        }

        SmsgPreResurrect(uint64_t guid) :
            ManagedPacket(SMSG_PRE_RESURRECT, 8),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            packet.appendPackGuid(guid);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
