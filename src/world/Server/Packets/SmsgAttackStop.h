/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgAttackStop : public ManagedPacket
    {
    public:
        WoWGuid attackerGuid;
        WoWGuid victimGuid;

        SmsgAttackStop() : SmsgAttackStop(WoWGuid(), WoWGuid())
        {
        }

        SmsgAttackStop(WoWGuid attackerGuid, WoWGuid victimGuid) :
            ManagedPacket(SMSG_ATTACKSTOP, 0),
            attackerGuid(attackerGuid),
            victimGuid(victimGuid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 8 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << attackerGuid;
            if (victimGuid.GetOldGuid() == 0)
                packet << uint8_t(0);
            else
                packet << victimGuid;

            packet << uint32_t(0);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
