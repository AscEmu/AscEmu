/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <unordered_map>
#include <functional>
#include <optional>
#include "WorldPacket.h"
#include "OpcodeTable.hpp"

// Define the number of supported versions (0 = Classic, 1 = TBC, etc.)
constexpr int NUM_VERSIONS = 5;

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

    // Overload for member functions with version flags
    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, void (WorldSession::* handler)(WorldPacket&), bool classic, bool tbc, bool wotlk, bool cata, bool mop)
    {
        OpcodeEntry entry;
        entry.handler = [handler](WorldSession& session, WorldPacket& packet) {
            (session.*handler)(packet);  // Call the member function
            };
        entry.state = State;
        entry.versions[0] = classic;
        entry.versions[1] = tbc;
        entry.versions[2] = wotlk;
        entry.versions[3] = cata;
        entry.versions[4] = mop;

        opcodeHandlers[opcode] = entry;
    }

    // Overload for free functions or lambdas with version flags
    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, std::function<void(WorldSession&, WorldPacket&)> handler, bool classic, bool tbc, bool wotlk, bool cata, bool mop)
    {
        OpcodeEntry entry;
        entry.handler = handler;
        entry.state = State;
        entry.versions[0] = classic;
        entry.versions[1] = tbc;
        entry.versions[2] = wotlk;
        entry.versions[3] = cata;
        entry.versions[4] = mop;

        opcodeHandlers[opcode] = entry;
    }

    // Overload for member functions without version flags (default to true for all versions)
    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, void (WorldSession::* handler)(WorldPacket&))
    {
        registerOpcode<State>(opcode, handler, true, true, true, true, true);
    }

    // Overload for free functions or lambdas without version flags (default to true for all versions)
    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, std::function<void(WorldSession&, WorldPacket&)> handler)
    {
        registerOpcode<State>(opcode, handler, true, true, true, true, true);
    }

    // Handles the incoming packet using the internal ID
    bool handleOpcode(WorldSession& session, WorldPacket& packet);

private:
    std::unordered_map<uint32_t, OpcodeEntry> opcodeHandlers;  // Map of internal IDs to handler functions and states
};
