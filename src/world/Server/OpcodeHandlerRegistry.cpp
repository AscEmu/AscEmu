/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Logging/Severity.hpp"
#include "OpcodeHandlerRegistry.hpp"
#include "OpcodeTable.hpp"
#include "Network/WorldPacket.hpp"
#include "WorldSession.h"
#include "Opcodes.hpp"

#include <cstdint>
#include <set>
#include <string>

bool OpcodeHandlerRegistry::handleOpcode(WorldSession& session, WorldPacket& packet)
{
    const uint32_t rawOpcode = packet.getOpcode();
    const auto protocol = session.getClientProtocol();
    const uint32_t internalId = sOpcodeTables.getInternalIdForHex(rawOpcode, protocol);

    return handleResolvedOpcode(session, packet, rawOpcode, internalId);
}

bool OpcodeHandlerRegistry::handleResolvedOpcode(WorldSession& session, WorldPacket& packet, uint32_t rawOpcode, uint32_t internalId)
{
    const auto protocol = session.getClientProtocol();
    const std::string opcodeName = internalId != 0
        ? sOpcodeTables.getNameForInternalId(internalId, protocol.expansion)
        : sOpcodeTables.getNameForOpcode(rawOpcode, protocol);

    const auto it = opcodeHandlers.find(internalId);
    if (it == opcodeHandlers.end() || internalId == 0)
    {
        logUnhandledOpcode(rawOpcode, internalId, opcodeName);
        return false;
    }

    const auto& entry = it->second;
    const int32_t tableIndex = WoW::getOpcodeTableIndex(protocol.expansion);
    if (tableIndex < 0 || tableIndex >= NUM_VERSIONS || !entry.sessionVersions[tableIndex] || !entry.sessionHandler)
    {
        logUnhandledOpcode(rawOpcode, internalId, opcodeName);
        return false;
    }

    if (entry.sessionState.has_value() && entry.sessionState.value() == STATUS_LOGGEDIN && !session.GetPlayer())
    {
        sLogger.debugOpcode("Received packet for invalid state. Internal ID: 0x{:04X}, Required State: {}, Name {}.", internalId, entry.sessionState.value(), opcodeName);
        return false;
    }

    entry.sessionHandler(session, packet);
    return true;
}

bool OpcodeHandlerRegistry::hasHandlerForVersion(uint32_t internalId, WoW::Expansion expansion) const
{
    const auto it = opcodeHandlers.find(internalId);
    if (it == opcodeHandlers.end() || internalId == 0)
        return false;

    const int32_t tableIndex = WoW::getOpcodeTableIndex(expansion);
    if (tableIndex < 0 || tableIndex >= NUM_VERSIONS)
        return false;

    return it->second.sessionVersions[tableIndex] && static_cast<bool>(it->second.sessionHandler);
}

void OpcodeHandlerRegistry::logUnhandledOpcode(uint32_t rawOpcode, uint32_t internalId, const std::string& name)
{
    static const std::set<uint32_t> ignoredOpcodes =
    {
        0x0000, 0x0040, 0x0150, 0x03F6, 0x15A9, 0x15AB
    };

    if (ignoredOpcodes.contains(rawOpcode))
        return;

    sLogger.warning("[Session] Unhandled opcode: Internal ID : 0x{:04X}, Raw Opcode : 0x{:08X}, Name {}.", internalId, rawOpcode, name);
}
