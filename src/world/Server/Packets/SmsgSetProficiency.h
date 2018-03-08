/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgSetProficiency : public ManagedPacket
    {
        static const size_t PACKET_SIZE = sizeof(uint8_t) + sizeof(uint32_t);
    public:
        uint8_t item_class;
        uint32_t proficiency;

        SmsgSetProficiency() : SmsgSetProficiency(0, 0)
        {
        }

        SmsgSetProficiency(uint8_t item_class, uint32_t proficiency) :
            ManagedPacket(SMSG_SET_PROFICIENCY, PACKET_SIZE),
            item_class(item_class),
            proficiency(proficiency)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            // All versions share same implementation
            packet << item_class << proficiency;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> item_class >> proficiency;
            return true;
        }
    };
}}
