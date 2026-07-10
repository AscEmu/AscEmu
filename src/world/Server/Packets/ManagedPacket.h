/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <memory>

#include "Network/WorldPacket.hpp"
#include "Server/Opcodes.hpp"
#include "Server/ClientProtocol.hpp"

namespace AscEmu::Packets
{
    class ManagedPacket
    {
    protected:
        virtual ~ManagedPacket() = default;

        uint16_t m_opcode;
        size_t m_minimum_size;

        WoW::ClientProtocol m_protocol{};

        virtual bool internalSerialise(WorldPacket&) { return true; }

        virtual bool internalDeserialise(WorldPacket&) { return true; }

        ManagedPacket(uint16_t opcode, size_t minimum_size) :
            m_opcode(opcode),
            m_minimum_size(minimum_size)
        {
        }

        virtual size_t expectedSize() const { return size_t(0); }

    public:
        void setClientProtocol(WoW::ClientProtocol protocol)
        {
            m_protocol = protocol;
        }

        [[nodiscard]] WoW::ClientProtocol getClientProtocol() const
        {
            return m_protocol;
        }

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
}
