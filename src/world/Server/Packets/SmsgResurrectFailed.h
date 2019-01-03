/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgResurrectFailed : public ManagedPacket
    {
    public:
        uint32_t failed;

        SmsgResurrectFailed() : SmsgResurrectFailed(0)
        {
        }

        SmsgResurrectFailed(uint32_t failed) :
            ManagedPacket(SMSG_RESURRECT_FAILED, 4),
            failed(failed)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << failed;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
