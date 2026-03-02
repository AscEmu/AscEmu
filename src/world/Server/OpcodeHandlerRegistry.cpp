/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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

    // TODO: Implement the opcode handler for the unhandled opcodes instead of ignoring them. For now, ignore some opcodes that are expected to be unhandled or have no handler in the new system to reduce log spam.
    bool ignoreLog = false;
    switch (rawOpcode)
    {
    case 0x0000: // CMSG_BOOTME
    case 0x0040: // CMSG_VIOLENCE_LEVEL
    case 0x0150: // CMSG_MOVE_TIME_SKIPPED
    case 0x03F6: // CMSG_REQUEST_PLAYED_TIME
    case 0x15A9: // CMSG_VOICE_SESSION_ENABLE
    case 0x15AB: // CMSG_WORLD_STATE_UI_TIMER_UPDATE
        ignoreLog = true;
        break;
    default:
        ignoreLog = false;
        break;
    }

    if (!ignoreLog)
    {
        // Log unhandled opcodes in the new system
        sLogger.warning("[Session] Unhandled opcode: Internal ID : 0x{:04X}, Raw Opcode : 0x{:04X}, Name{}", internalId, rawOpcode, opcodeName);
    }

    // No handler found for this internal ID
    return false;
}
