/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLfgOfferContinue : public ManagedPacket
    {
    public:
        uint32_t dungeonEntry;

        SmsgLfgOfferContinue() : SmsgLfgOfferContinue(0)
        {
        }

        SmsgLfgOfferContinue(uint32_t dungeonEntry) :
            ManagedPacket(SMSG_LFG_OFFER_CONTINUE, 0),
            dungeonEntry(dungeonEntry)
        {
        }

    protected:

        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            packet << dungeonEntry;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
