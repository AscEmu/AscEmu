/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgPartyKillLog : public ManagedPacket
    {
    public:
        uint64_t playerGuid;
        uint64_t killedGuid;

        SmsgPartyKillLog() : SmsgPartyKillLog(0, 0)
        {
        }

        SmsgPartyKillLog(uint64_t playerGuid, uint64_t killedGuid) :
            ManagedPacket(SMSG_PARTYKILLLOG, 0),
            playerGuid(playerGuid),
            killedGuid(killedGuid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 8;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << playerGuid << killedGuid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
