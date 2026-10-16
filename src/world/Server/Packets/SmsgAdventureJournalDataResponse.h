/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    // 9.x and later: the adventure guide suggestions, none here
    class SmsgAdventureJournalDataResponse : public ManagedPacket
    {
    public:
        bool onLevelUp;

        SmsgAdventureJournalDataResponse() : SmsgAdventureJournalDataResponse(false)
        {
        }

        explicit SmsgAdventureJournalDataResponse(bool onLevelUp) :
            ManagedPacket(SMSG_ADVENTURE_JOURNAL_DATA_RESPONSE, 5),
            onLevelUp(onLevelUp)
        {
        }

    protected:
        size_t expectedSize() const override { return 5; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet.writeBit(onLevelUp);
            packet.flushBits();
            packet << uint32_t(0);              // entries
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
