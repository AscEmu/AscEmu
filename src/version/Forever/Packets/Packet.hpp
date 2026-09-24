#pragma once

#include "Network/ByteBuffer.hpp"
#include "version/Forever/Opcodes.hpp"

namespace AscEmu::Version::Forever::Packets
{
    class Packet : public ByteBuffer
    {
    public:
        Packet() = default;
        explicit Packet(Opcode opcode) : m_opcode(opcode) {}
        Packet(Opcode opcode, const uint8_t* payload, size_t payloadSize) : m_opcode(opcode)
        {
            if (payload != nullptr && payloadSize != 0)
                append(payload, payloadSize);
        }

        [[nodiscard]] Opcode getOpcode() const noexcept { return m_opcode; }

    private:
        Opcode m_opcode{Opcode::NONE};
    };
}
