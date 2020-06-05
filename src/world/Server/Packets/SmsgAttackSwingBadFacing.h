/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

//\NOTE: This gets replaced in Mop by SMSG_ATTACKSWING_ERROR
namespace AscEmu::Packets
{
    class SmsgAttackSwingBadFacing : public ManagedPacket
    {
#if VERSION_STRING < Mop
    public:

        SmsgAttackSwingBadFacing() :
            ManagedPacket(SMSG_ATTACKSWING_BADFACING, 0)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
