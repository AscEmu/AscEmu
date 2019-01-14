/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgTitleEarned : public ManagedPacket
    {
    public:
        uint32_t titleId;
        uint32_t setValue;

        SmsgTitleEarned() : SmsgTitleEarned(0, 0)
        {
        }

        SmsgTitleEarned(uint32_t titleId, uint32_t setValue) :
            ManagedPacket(SMSG_TITLE_EARNED, 0),
            titleId(titleId),
            setValue(setValue)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << titleId << setValue;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
