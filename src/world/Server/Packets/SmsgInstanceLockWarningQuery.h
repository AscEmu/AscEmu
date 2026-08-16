/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgInstanceLockWarningQuery : public ManagedPacket
    {
    public:
        uint32_t timeout;
        uint32_t completedEncounterMask;

        SmsgInstanceLockWarningQuery() : SmsgInstanceLockWarningQuery(0)
        {
        }

        explicit SmsgInstanceLockWarningQuery(uint32_t completedEncounterMask, uint32_t timeout = 60000) :
            ManagedPacket(SMSG_INSTANCE_LOCK_WARNING_QUERY, 9),
            timeout(timeout),
            completedEncounterMask(completedEncounterMask)
        {
        }

    protected:
        size_t expectedSize() const override { return 9; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << timeout;
                packet << completedEncounterMask;
                packet << uint8_t(0);

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet << timeout;
                packet << completedEncounterMask;
                packet.writeBit(0);
                packet.writeBit(0);
                packet.flushBits();

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
