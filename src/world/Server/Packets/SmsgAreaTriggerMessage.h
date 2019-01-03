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
    class SmsgAreaTriggerMessage : public ManagedPacket
    {
    public:
        uint32_t unknown1;  //probably size
        const char* text;
        uint8_t unknown2;

        SmsgAreaTriggerMessage() : SmsgAreaTriggerMessage(0, "", 0)
        {
        }

        SmsgAreaTriggerMessage(uint32_t unknown1, const char* text, uint8_t unknown2) :
            ManagedPacket(SMSG_AREA_TRIGGER_MESSAGE, 0),
            unknown1(unknown1),
            text(text),
            unknown2(unknown2)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + strlen(text) + 1 + 1;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << unknown1 << text << unknown2;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
