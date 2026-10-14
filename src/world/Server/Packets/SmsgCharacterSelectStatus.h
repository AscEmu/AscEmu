/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once
#include "ManagedPacket.h"
#include <cstdint>
namespace AscEmu::Packets
{
    class SmsgCharacterSelectStatus : public ManagedPacket
    {
    public:
        explicit SmsgCharacterSelectStatus(uint8_t status = 0) : ManagedPacket(SMSG_CHARACTER_SELECT_STATUS, 1), status(status) {}
    protected:
        size_t expectedSize() const override { return 1; }
        bool internalSerialise(WorldPacket& packet) override { if (!m_protocol.isForever()) return false; packet << status; return true; }
        bool internalDeserialise(WorldPacket&) override { return false; }
    private:
        uint8_t status;
    };
}
