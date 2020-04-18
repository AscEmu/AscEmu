/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WOWSERVER_WORLDPACKET_H
#define WOWSERVER_WORLDPACKET_H

#include "CommonTypes.hpp"
#include "ByteBuffer.h"
#include "StackBuffer.h"
#include "../world/Server/Packets/Opcode.h"

class SERVER_DECL WorldPacket : public ByteBuffer
{
#if VERSION_STRING != Mop
    public:
        __inline WorldPacket() : ByteBuffer(0), m_opcode(MSG_NULL_ACTION) { }
        __inline WorldPacket(uint16_t opcode, size_t res) : ByteBuffer(res), m_opcode(opcode) {}
        __inline WorldPacket(size_t res) : ByteBuffer(res), m_opcode(0) { }
        __inline WorldPacket(const WorldPacket & packet) : ByteBuffer(packet), m_opcode(packet.m_opcode) {}

        //! Clear packet and set opcode all in one mighty blow
        __inline void Initialize(uint16_t opcode, size_t newres = 200)
        {
            clear();
            _storage.reserve(newres);
            m_opcode = opcode;
        }

        __inline uint16_t GetOpcode() const { return m_opcode; }
        __inline void SetOpcode(uint16_t opcode) { m_opcode = opcode; }

    protected:
        uint16_t m_opcode;
#else
public:
    __inline WorldPacket() : ByteBuffer(0), m_opcode(MSG_NULL_ACTION) { }
    __inline WorldPacket(uint32_t opcode, size_t res) : ByteBuffer(res), m_opcode(opcode) {}
    __inline WorldPacket(size_t res) : ByteBuffer(res), m_opcode(0) { }
    __inline WorldPacket(const WorldPacket & packet) : ByteBuffer(packet), m_opcode(packet.m_opcode) {}

    //! Clear packet and set opcode all in one mighty blow
    __inline void Initialize(uint16_t opcode, size_t newres = 200)
    {
        clear();
        _storage.reserve(newres);
        m_opcode = opcode;
    }

    __inline uint32_t GetOpcode() const { return m_opcode; }
    __inline void SetOpcode(uint32_t opcode) { m_opcode = opcode; }

protected:
    uint16_t m_opcode;
#endif
};

template<uint32_t Size>
class SERVER_DECL StackWorldPacket : public StackBuffer<Size>
{
        uint16_t m_opcode;
    public:
        __inline StackWorldPacket(uint16_t opcode) : StackBuffer<Size>(), m_opcode(opcode) { }

        //! Clear packet and set opcode all in one mighty blow
        __inline void Initialize(uint16_t opcode)
        {
            StackBuffer<Size>::Clear();
            m_opcode = opcode;
        }

        uint16_t GetOpcode() { return m_opcode; }
        __inline void SetOpcode(uint16_t opcode) { m_opcode = opcode; }
};

#endif
