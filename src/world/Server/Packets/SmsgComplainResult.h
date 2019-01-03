/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgComplainResult : public ManagedPacket
    {
#if VERSION_STRING < Cata
    public:
        uint8_t result;

        SmsgComplainResult() : SmsgComplainResult(0)
        {
        }

        SmsgComplainResult(uint8_t result) :
            ManagedPacket(SMSG_COMPLAIN_RESULT, 1),
            result(result)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << result;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
#endif
    };
}}
