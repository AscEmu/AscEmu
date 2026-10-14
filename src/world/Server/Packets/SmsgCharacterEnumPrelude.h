/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once
#include "ManagedPacket.h"
#include <cstdint>
namespace AscEmu::Packets
{
    class SmsgCharacterEnumPrelude : public ManagedPacket
    {
    public:
        explicit SmsgCharacterEnumPrelude(uint64_t unknown0 = 2) : ManagedPacket(SMSG_CHARACTER_ENUM_PRELUDE, 8), unknown0(unknown0) {}
    protected:
        size_t expectedSize() const override { return 8; }
        bool internalSerialise(WorldPacket& packet) override { if (!m_protocol.isForever()) return false; packet << unknown0; return true; }
        bool internalDeserialise(WorldPacket&) override { return false; }
    private:
        uint64_t unknown0;
    };
    class SmsgCharacterEnumPreludeExtended : public ManagedPacket
    {
    public:
        SmsgCharacterEnumPreludeExtended(uint64_t unknown0 = 2, uint64_t unknown1 = 0, uint64_t unknown2 = 0) : ManagedPacket(SMSG_CHARACTER_ENUM_PRELUDE_EXTENDED, 24), unknown0(unknown0), unknown1(unknown1), unknown2(unknown2) {}
    protected:
        size_t expectedSize() const override { return 24; }
        bool internalSerialise(WorldPacket& packet) override { if (!m_protocol.isForever()) return false; packet << unknown0 << unknown1 << unknown2; return true; }
        bool internalDeserialise(WorldPacket&) override { return false; }
    private:
        uint64_t unknown0, unknown1, unknown2;
    };
}
