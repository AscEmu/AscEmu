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
#include <utility>
#include "Network/WorldPacket.hpp"
#include "OpcodeTable.hpp"

// Define the number of supported versions (0 = Classic, 1 = TBC, etc.)
constexpr int NUM_VERSIONS = MAX_VERSION_INDEX;

class WorldSession;
class WorldSocket;

enum OpcodeState
{
    STATUS_CONNECTED = 0,
    STATUS_AUTHED,
    STATUS_LOGGEDIN,
    //STATUS_LOGGEDIN_RECENTLY_LOGGOUT = 3,
};

struct OpcodeEntry
{
    std::function<void(WorldSession&, WorldPacket&)> sessionHandler;
    std::function<bool(WorldSocket&, WorldPacket&)> socketHandler;
    std::optional<OpcodeState> sessionState;
    std::optional<OpcodeState> socketState;
    bool sessionVersions[NUM_VERSIONS] = { false };
    bool socketVersions[NUM_VERSIONS] = { false };
};

class OpcodeHandlerRegistry
{
public:
    static OpcodeHandlerRegistry& instance()
    {
        static OpcodeHandlerRegistry instance;
        return instance;
    }

    // Full expansion-indexed overload. Order matches WoW::Expansion / opcode table slots 0..12.
    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, void (WorldSession::* handler)(WorldPacket&),
        bool classic, bool tbc, bool wotlk, bool cata, bool mop,
        bool wod, bool legion, bool bfa, bool shadowlands, bool dragonflight,
        bool tww, bool midnight, bool forever)
    {
        auto& entry = opcodeHandlers[opcode];
        entry.sessionHandler = [handler](WorldSession& session, WorldPacket& packet) {
            (session.*handler)(packet);
        };
        entry.sessionState = State;
        setSessionVersions(entry, classic, tbc, wotlk, cata, mop, wod, legion, bfa,
            shadowlands, dragonflight, tww, midnight, forever);
    }

    // Compatibility overload for the currently implemented legacy expansions.
    // Slots WoD..Forever stay disabled unless explicitly registered through the full overload.
    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, void (WorldSession::* handler)(WorldPacket&), bool classic, bool tbc, bool wotlk, bool cata, bool mop)
    {
        registerOpcode<State>(opcode, handler, classic, tbc, wotlk, cata, mop, false, false, false, false, false, false, false, false);
    }

    // Full expansion-indexed overload for free functions/lambdas.
    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, std::function<void(WorldSession&, WorldPacket&)> handler, bool classic, bool tbc, bool wotlk, bool cata, bool mop, bool wod, bool legion, bool bfa, bool shadowlands, bool dragonflight, bool tww, bool midnight, bool forever)
    {
        auto& entry = opcodeHandlers[opcode];
        entry.sessionHandler = std::move(handler);
        entry.sessionState = State;
        setSessionVersions(entry, classic, tbc, wotlk, cata, mop, wod, legion, bfa, shadowlands, dragonflight, tww, midnight, forever);
    }

    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, std::function<void(WorldSession&, WorldPacket&)> handler, bool classic, bool tbc, bool wotlk, bool cata, bool mop)
    {
        registerOpcode<State>(opcode, std::move(handler), classic, tbc, wotlk, cata, mop, false, false, false, false, false, false, false, false);
    }

    // Legacy default: Classic..MoP enabled, later expansion slots disabled until explicitly verified.
    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, void (WorldSession::* handler)(WorldPacket&))
    {
        registerOpcode<State>(opcode, handler, true, true, true, true, true, false, false, false, false, false, false, false, false);
    }

    template <OpcodeState State = STATUS_LOGGEDIN>
    void registerOpcode(uint32_t opcode, std::function<void(WorldSession&, WorldPacket&)> handler)
    {
        registerOpcode<State>(opcode, std::move(handler), true, true, true, true, true, false, false, false, false, false, false, false, false);
    }

    template <OpcodeState State = STATUS_AUTHED>
    void registerSocketOpcode(uint32_t opcode, bool (WorldSocket::* handler)(WorldPacket&), bool forever = true)
    {
        auto& entry = opcodeHandlers[opcode];
        entry.socketHandler = [handler](WorldSocket& socket, WorldPacket& packet) {
            return (socket.*handler)(packet);
        };
        entry.socketState = State;
        entry.socketVersions[WoW::getOpcodeTableIndex(WoW::Expansion::Forever)] = forever;
    }

    // Registers the Forever-specific socket handlers in this same central registry.
    void initializeForeverSocketHandlers();

    // Resolves a legacy wire opcode through the session protocol and dispatches it.
    bool handleOpcode(WorldSession& session, WorldPacket& packet);

    // Dispatches a wire opcode that has already been resolved by a protocol-specific transport.
    bool handleResolvedOpcode(WorldSession& session, WorldPacket& packet, uint32_t rawOpcode, uint32_t internalId);
    bool handleResolvedOpcode(WorldSocket& socket, WorldPacket& packet, uint32_t rawOpcode, uint32_t internalId);
    [[nodiscard]] bool hasHandlerForVersion(uint32_t internalId, WoW::Expansion expansion) const;

    static void logUnhandledOpcode(uint32_t rawOpcode, uint32_t internalId, const std::string& name);

private:
    static void setSessionVersions(OpcodeEntry& entry,
        bool classic, bool tbc, bool wotlk, bool cata, bool mop,
        bool wod, bool legion, bool bfa, bool shadowlands, bool dragonflight,
        bool tww, bool midnight, bool forever)
    {
        entry.sessionVersions[0] = classic;
        entry.sessionVersions[1] = tbc;
        entry.sessionVersions[2] = wotlk;
        entry.sessionVersions[3] = cata;
        entry.sessionVersions[4] = mop;
        entry.sessionVersions[5] = wod;
        entry.sessionVersions[6] = legion;
        entry.sessionVersions[7] = bfa;
        entry.sessionVersions[8] = shadowlands;
        entry.sessionVersions[9] = dragonflight;
        entry.sessionVersions[10] = tww;
        entry.sessionVersions[11] = midnight;
        entry.sessionVersions[12] = forever;
    }

    std::unordered_map<uint32_t, OpcodeEntry> opcodeHandlers;  // Map of internal IDs to handler functions and states
};
