/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "ByteBuffer.h"

class SERVER_DECL WorldPacket : public ByteBuffer
{
public:
    inline WorldPacket() : ByteBuffer(0), m_opcode(0) { }
    inline WorldPacket(uint16_t opcode, size_t res) : ByteBuffer(res), m_opcode(opcode) {}
    inline WorldPacket(size_t res) : ByteBuffer(res), m_opcode(0) { }
    inline WorldPacket(const WorldPacket & packet) : ByteBuffer(packet), m_opcode(packet.m_opcode) {}

    // Clear packet and set opcode all in one mighty blow
    inline void Initialize(uint16_t opcode, size_t newres = 200)
    {
        clear();
        _storage.reserve(newres);
        m_opcode = opcode;
    }

    inline uint16_t GetOpcode() const { return m_opcode; }
    inline void SetOpcode(uint16_t opcode) { m_opcode = opcode; }

protected:
    uint16_t m_opcode;

};
