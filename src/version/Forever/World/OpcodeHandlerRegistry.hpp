#pragma once

#include "version/Forever/Opcodes.hpp"
#include "version/Forever/Packets/Packet.hpp"

#include <cstdint>
#include <unordered_map>

class WorldSocket;

namespace AscEmu::Version::Forever
{
    enum OpcodeState : uint8_t
    {
        STATUS_CONNECTED = 0,
        STATUS_AUTHED
    };

    struct OpcodeHandlerEntry
    {
        using Handler = bool (WorldSocket::*)(Packets::Packet&);
        Handler handler{nullptr};
        OpcodeState state{STATUS_AUTHED};
    };

    class OpcodeHandlerRegistry
    {
    public:
        static OpcodeHandlerRegistry& instance();

        template <OpcodeState State = STATUS_AUTHED>
        void registerOpcode(Opcode opcode, OpcodeHandlerEntry::Handler handler)
        {
            m_handlers[opcode] = OpcodeHandlerEntry{handler, State};
        }

        void initialize();
        bool handleOpcode(WorldSocket& socket, Packets::Packet& packet);

    private:
        struct OpcodeHash
        {
            size_t operator()(Opcode opcode) const noexcept
            {
                return static_cast<size_t>(opcode);
            }
        };

        std::unordered_map<Opcode, OpcodeHandlerEntry, OpcodeHash> m_handlers;
        bool m_initialized{false};
    };
}
