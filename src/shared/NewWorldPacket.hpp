/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Common.h"
#include "NewByteBuffer.hpp"
#include "../world/Server/Packets/Opcodes.h"

#ifndef _NEW_WORLD_PACKET_HPP
#define _NEW_WORLD_PACKET_HPP

class NewWorldPacket : public NewByteBuffer
{
    public:

        NewWorldPacket() : NewByteBuffer(0), m_opcode(MSG_NULL_ACTION) { }
        NewWorldPacket(Opcodes opcode, size_t res = 200) : NewByteBuffer(res), m_opcode(opcode) { }

        NewWorldPacket(const NewWorldPacket& packet) : NewByteBuffer(packet), m_opcode(packet.m_opcode) { }

        void Initialize(Opcodes opcode, size_t newres = 200)
        {
            clear();
            _storage.reserve(newres);
            m_opcode = opcode;
        }

        Opcodes GetOpcode() const { return m_opcode; }
        void SetOpcode(Opcodes opcode) { m_opcode = opcode; }

    protected:

        Opcodes m_opcode;
};

#endif  //_NEW_WORLD_PACKET_HPP
