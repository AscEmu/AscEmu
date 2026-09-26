/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <unordered_map>
#include <functional>
#include <optional>
#include <string>
#include "Network/WorldPacket.hpp"
#include "Opcodes.hpp"

// Define the number of supported versions (0 = Classic, 1 = TBC, etc.)
constexpr int NUM_VERSIONS = 7;

class WorldSession;

enum OpcodeState
{
    STATUS_AUTHED = 0,
    STATUS_LOGGEDIN,
    //STATUS_LOGGEDIN_RECENTLY_LOGGOUT = 3,
};

struct OpcodeEntry
{
    std::function<void(WorldSession&, WorldPacket&)> handler;  // Generic handler type (can hold member/non-member functions)
    std::optional<OpcodeState> state;  // Optional state
    bool versions[NUM_VERSIONS] = { false }; // Defaults to false for all versions
};

class OpcodeHandlerRegistry
{
public:
    static OpcodeHandlerRegistry& instance()
    {
        static OpcodeHandlerRegistry instance;
        return instance;
    }

    // Overload for member functions with version flags (Classic .. Legion)
    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, void (WorldSession::* handler)(WorldPacket&), bool classic, bool tbc, bool wotlk, bool cata, bool mop, bool wod, bool legion)
    {
        registerOpcode<State>(opcode, std::function<void(WorldSession&, WorldPacket&)>([handler](WorldSession& session, WorldPacket& packet) {
            (session.*handler)(packet);  // Call the member function
            }), classic, tbc, wotlk, cata, mop, wod, legion);
    }

    // Overload for free functions or lambdas with version flags (Classic .. Legion)
    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, std::function<void(WorldSession&, WorldPacket&)> handler, bool classic, bool tbc, bool wotlk, bool cata, bool mop, bool wod, bool legion)
    {
        OpcodeEntry entry;
        entry.handler = handler;
        entry.state = State;
        entry.versions[0] = classic;
        entry.versions[1] = tbc;
        entry.versions[2] = wotlk;
        entry.versions[3] = cata;
        entry.versions[4] = mop;
        entry.versions[5] = wod;
        entry.versions[6] = legion;

        opcodeHandlers[opcode] = entry;
    }

    // Overloads with the flags up to Mop: the handler is not enabled for WoD and Legion clients, their packet
    // layouts have to be checked before a handler is registered for them with the overloads above
    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, void (WorldSession::* handler)(WorldPacket&), bool classic, bool tbc, bool wotlk, bool cata, bool mop)
    {
        registerOpcode<State>(opcode, handler, classic, tbc, wotlk, cata, mop, false, false);
    }

    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, std::function<void(WorldSession&, WorldPacket&)> handler, bool classic, bool tbc, bool wotlk, bool cata, bool mop)
    {
        registerOpcode<State>(opcode, handler, classic, tbc, wotlk, cata, mop, false, false);
    }

    // Overloads without version flags: enabled for Classic .. Mop
    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, void (WorldSession::* handler)(WorldPacket&))
    {
        registerOpcode<State>(opcode, handler, true, true, true, true, true);
    }

    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, std::function<void(WorldSession&, WorldPacket&)> handler)
    {
        registerOpcode<State>(opcode, handler, true, true, true, true, true);
    }

    // Handles the incoming packet using the internal ID
    bool handleOpcode(WorldSession& session, WorldPacket& packet);
    static void logUnhandledOpcode(uint16_t rawOpcode, uint32_t internalId, const std::string& name);

private:
    std::unordered_map<uint32_t, OpcodeEntry> opcodeHandlers;  // Map of internal IDs to handler functions and states
};
