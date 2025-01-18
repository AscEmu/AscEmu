/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "OpcodeHandlerRegistry.hpp"
#include "WorldSession.h"
#include "Logging/Logger.hpp"

bool OpcodeHandlerRegistry::handleOpcode(WorldSession& session, WorldPacket& packet)
{
    uint16_t rawOpcode = packet.GetOpcode();

    // Get the internal ID from the opcode table
    uint32_t internalId = sOpcodeTables.getInternalIdForHex(rawOpcode);
    std::string opcodeName = sOpcodeTables.getNameForOpcode(rawOpcode);

    auto it = opcodeHandlers.find(internalId);
    if (it != opcodeHandlers.end())
    {
        const auto& entry = it->second;
        int versionId = sOpcodeTables.getVersionIdForAEVersion();  // Get version ID dynamically

        if (versionId < NUM_VERSIONS && entry.versions[versionId])
        {
            // Check if the opcode is defined on our version
            if (internalId == 0)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received out of range packet with undefined opcode 0x{:04X}", rawOpcode);
                return false;
            }

            // Check if the state is provided and the session matches the required state
            if (entry.state.has_value())
            {
                OpcodeState requiredState = entry.state.value();
                if (requiredState == STATUS_LOGGEDIN && !session.GetPlayer())
                {
                    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received packet for invalid state. Internal ID: 0x{:04X}, Required State: {}, Name {}", internalId, requiredState, opcodeName);
                    return false;
                }
            }

            // Dispatch the packet to the appropriate handler
            entry.handler(session, packet);
            return true;
        }
    }

    // Log unhandled opcodes in the new system
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "[Session] Unhandled opcode in the new system: Internal ID: 0x{:04X}, Raw Opcode: 0x{:04X}, Name {}", internalId, rawOpcode, opcodeName);


    // No handler found for this internal ID
    return false;
}
