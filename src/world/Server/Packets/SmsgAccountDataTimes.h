/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAccountDataTimes : public ManagedPacket
    {
    public:
        uint32_t time;
        uint8_t unknown1;
        uint32_t mask;
        uint8_t dataCount;
        uint64_t playerGuid;                        // character of the per character cache (6.x and 7.x), 0 for the account cache

        SmsgAccountDataTimes() : SmsgAccountDataTimes(0, 0, 0, 0)
        {
        }

        SmsgAccountDataTimes(uint32_t time, uint8_t unknown1, uint32_t mask, uint8_t dataCount, uint64_t playerGuid = 0) :
            ManagedPacket(SMSG_ACCOUNT_DATA_TIMES, dataCount > 8 ? 32 * 4 : 4 + 1 + 4 + dataCount * 4),
            time(time),
            unknown1(unknown1),
            mask(mask),
            dataCount(dataCount),
            playerGuid(playerGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
            {
                for (auto i = 0; i < dataCount; ++i)
                    packet << uint32_t(0);
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << time << unknown1 << mask;
                for (auto i = 0; i < dataCount; ++i)
                    if (mask & (1 << i))
                        packet << uint32_t(0);
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WoD && m_protocol.expansion <= WoW::Expansion::_Legion)
            {
                packet << WoWGuid(playerGuid).toGuid128(m_protocol.realmId, 0);
                packet << time;

                for (uint8_t i = 0; i < 8; ++i)
                    packet << uint32_t(0);
            }
            else // Mop
            {
                packet.writeBit(1);
                packet.flushBits();

                for (uint8_t i = 0; i < 8; ++i)
                    packet << uint32_t(0);

                packet << mask << time;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
