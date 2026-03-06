/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Logging/Severity.hpp"
#include "OpcodeHandlerRegistry.hpp"
#include "OpcodeTable.hpp"
#include "WorldPacket.h"
#include "WorldSession.h"

#include <cstdint>
#include <set>
#include <string>

bool OpcodeHandlerRegistry::handleOpcode(WorldSession& session, WorldPacket& packet)
{
    uint16_t rawOpcode = packet.GetOpcode();

    // Get the internal ID from the opcode table
    uint32_t internalId = sOpcodeTables.getInternalIdForHex(rawOpcode);
    std::string opcodeName = sOpcodeTables.getNameForOpcode(rawOpcode);

    auto it = opcodeHandlers.find(internalId);

    if (it == opcodeHandlers.end() || internalId == 0)
    {
        logUnhandledOpcode(rawOpcode, internalId, opcodeName);
        return false;
    }

    const auto& entry = it->second;

    if (const int versionId = sOpcodeTables.getVersionIdForAEVersion(); versionId >= NUM_VERSIONS || !entry.versions[versionId])
    {
        logUnhandledOpcode(rawOpcode, internalId, opcodeName);
        return false;
    }

    if (entry.state.has_value() && entry.state.value() == STATUS_LOGGEDIN && !session.GetPlayer())
    {
        sLogger.debugFlag(AscEmu::Logging::LF_OPCODE,
                          "Received packet for invalid state. Internal ID: 0x{:04X}, Required State: {}, Name {}",
                          internalId, entry.state.value(), opcodeName);
        return false;
    }

    entry.handler(session, packet);
    return true;
}

void OpcodeHandlerRegistry::logUnhandledOpcode(uint16_t rawOpcode, uint32_t internalId, const std::string& name)
{
    static const std::set<uint16_t> ignoredOpcodes = 
    {
        0x0000, 0x0040, 0x0150, 0x03F6, 0x15A9, 0x15AB
    };

    if (ignoredOpcodes.contains(rawOpcode))
        return;

    sLogger.warning("[Session] Unhandled opcode: Internal ID : 0x{:04X}, Raw Opcode : 0x{:04X}, Name {}",
        internalId, rawOpcode, name);
}
