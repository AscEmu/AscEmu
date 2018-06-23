/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgRealmSplit : public ManagedPacket
    {
    public:
        uint32_t unknown;
        uint32_t splitState;
        std::string dateFormat;

        SmsgRealmSplit() : SmsgRealmSplit(0, 0, "")
        {
        }

        SmsgRealmSplit(uint32_t unknown, uint32_t splitState, std::string dateFormat) :
            ManagedPacket(SMSG_REALM_SPLIT, 8 + dateFormat.size() + 1),
            unknown(unknown),
            splitState(splitState),
            dateFormat(dateFormat)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            // All versions share same implementation
            packet << unknown << splitState << dateFormat;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            return false;
        }
    };
}}
