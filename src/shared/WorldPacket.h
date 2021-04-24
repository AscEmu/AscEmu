/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "ByteBuffer.h"

class SERVER_DECL WorldPacket : public ByteBuffer
{
public:
    WorldPacket() : ByteBuffer(0), m_opcode(0) {}
    WorldPacket(uint16_t opcode, size_t res) : ByteBuffer(res), m_opcode(opcode) {}
    WorldPacket(size_t res) : ByteBuffer(res), m_opcode(0) {}
    WorldPacket(const WorldPacket & packet) : ByteBuffer(packet), m_opcode(packet.m_opcode) {}

    // Clear packet and set opcode all in one mighty blow
    void Initialize(uint16_t opcode, size_t newres = 200)
    {
        clear();
        _storage.reserve(newres);
        m_opcode = opcode;
    }

    uint16_t GetOpcode() const { return m_opcode; }
    void SetOpcode(uint16_t opcode) { m_opcode = opcode; }

protected:
    uint16_t m_opcode;

};
