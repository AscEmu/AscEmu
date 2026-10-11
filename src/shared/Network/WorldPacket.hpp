/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Platform/SymbolVisibility.hpp"
#include "Network/ByteBuffer.hpp"

class SERVER_DECL WorldPacket : public ByteBuffer
{
public:
    using Opcode = uint32_t;

    static constexpr size_t defaultReserve = 200;

    WorldPacket() : ByteBuffer(0), m_opcode(0) {}
    explicit WorldPacket(size_t reserveSize) : ByteBuffer(reserveSize), m_opcode(0) {}
    WorldPacket(Opcode opcode, size_t reserveSize) : ByteBuffer(reserveSize), m_opcode(opcode) {}

    WorldPacket(const WorldPacket& packet) = default;
    WorldPacket(WorldPacket&& packet) noexcept = default;
    ~WorldPacket() override = default;

    WorldPacket& operator=(const WorldPacket& packet) = default;
    WorldPacket& operator=(WorldPacket&& packet) noexcept = default;

    void initialize(Opcode opcode, size_t reserveSize = defaultReserve)
    {
        clear();
        reserve(reserveSize);
        m_opcode = opcode;
    }

    [[nodiscard]] Opcode getOpcode() const noexcept
    {
        return m_opcode;
    }

    void setOpcode(Opcode opcode) noexcept
    {
        m_opcode = opcode;
    }

private:
    Opcode m_opcode;
};
