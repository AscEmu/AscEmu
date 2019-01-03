/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgSetPlayerDeclinedNamesResult : public ManagedPacket
    {
    public:
        uint32_t error;
        uint64_t guid;

        SmsgSetPlayerDeclinedNamesResult() : SmsgSetPlayerDeclinedNamesResult(0, 0)
        {
        }

        SmsgSetPlayerDeclinedNamesResult(uint32_t error, uint64_t guid) :
            ManagedPacket(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 12),
            error(error),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << error << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
