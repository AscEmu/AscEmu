/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <memory>

namespace AscEmu { namespace Packets
{
    class ManagedPacket
    {
    protected:
        virtual ~ManagedPacket() = default;

        uint16_t m_opcode;
        size_t m_minimum_size;

        virtual bool internalSerialise(WorldPacket& packet) { return true; }

        virtual bool internalDeserialise(WorldPacket& packet) { return true; }

        ManagedPacket(uint16_t opcode, size_t expected_size) :
            m_opcode(opcode),
            m_minimum_size(expected_size)
        {
        }

        virtual size_t expectedSize() const { return m_minimum_size; }

    public:
        virtual std::unique_ptr<WorldPacket> serialise()
        {
            auto packet = std::make_unique<WorldPacket>(m_opcode, expectedSize());

            if (!internalSerialise(*packet))
                return nullptr;

            return packet;
        }

        virtual bool deserialise(WorldPacket& packet)
        {
            if (packet.remaining() < m_minimum_size)
                return false;

            return internalDeserialise(packet);
        }
    };
}}
