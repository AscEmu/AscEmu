/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgDismountResult : public ManagedPacket
    {
    public:
        uint32_t result;

        SmsgDismountResult() : SmsgDismountResult(0)
        {
        }

        SmsgDismountResult(uint32_t result) :
            ManagedPacket(SMSG_DISMOUNTRESULT, 0),
            result(result)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << result;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
