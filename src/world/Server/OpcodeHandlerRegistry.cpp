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
#include "WorldSocket.hpp"
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

bool OpcodeHandlerRegistry::handleResolvedOpcode(WorldSocket& socket, WorldPacket& packet, uint32_t rawOpcode, uint32_t internalId)
{
    const auto expansion = WoW::Expansion::Forever;
    const std::string opcodeName = internalId != 0
        ? sOpcodeTables.getNameForInternalId(internalId, expansion)
        : sOpcodeTables.getNameForOpcode(rawOpcode, expansion);

    const auto it = opcodeHandlers.find(internalId);
    if (it == opcodeHandlers.end() || internalId == 0)
    {
        logUnhandledOpcode(rawOpcode, internalId, opcodeName);
        return false;
    }

    const auto& entry = it->second;
    constexpr int32_t tableIndex = WoW::getOpcodeTableIndex(WoW::Expansion::Forever);

    if (entry.socketVersions[tableIndex] && entry.socketHandler)
    {
        if (entry.socketState.has_value())
        {
            if (entry.socketState.value() == STATUS_AUTHED && socket.getSession() == nullptr)
            {
                sLogger.warning("WorldSocket::Forever: received {} before WorldSession authentication completed.", opcodeName);
                return false;
            }
            if (entry.socketState.value() == STATUS_LOGGEDIN && (socket.getSession() == nullptr || socket.getSession()->GetPlayer() == nullptr))
            {
                sLogger.warning("WorldSocket::Forever: received {} before player login completed.", opcodeName);
                return false;
            }
        }

        return entry.socketHandler(socket, packet);
    }

    if (socket.getSession() != nullptr && entry.sessionVersions[tableIndex] && entry.sessionHandler)
        return handleResolvedOpcode(*socket.getSession(), packet, rawOpcode, internalId);

    logUnhandledOpcode(rawOpcode, internalId, opcodeName);
    return false;
}

bool OpcodeHandlerRegistry::hasHandlerForVersion(uint32_t internalId, WoW::Expansion expansion) const
{
    const auto it = opcodeHandlers.find(internalId);
    if (it == opcodeHandlers.end() || internalId == 0)
        return false;

    const int32_t tableIndex = WoW::getOpcodeTableIndex(expansion);
    if (tableIndex < 0 || tableIndex >= NUM_VERSIONS)
        return false;

    return (it->second.sessionVersions[tableIndex] && static_cast<bool>(it->second.sessionHandler)) ||
           (it->second.socketVersions[tableIndex] && static_cast<bool>(it->second.socketHandler));
}

void OpcodeHandlerRegistry::initializeForeverSocketHandlers()
{
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;

    registerSocketOpcode(CMSG_ENUM_CHARACTERS, &WorldSocket::handleForeverCharEnumOpcode);
    registerSocketOpcode(CMSG_CHECK_CHARACTER_NAME_AVAILABILITY, &WorldSocket::handleForeverCheckCharacterNameOpcode);
    registerSocketOpcode(CMSG_CHAR_CREATE, &WorldSocket::handleForeverCharCreateOpcode);
    registerSocketOpcode(CMSG_PLAYER_LOGIN, &WorldSocket::handleForeverPlayerLoginOpcode);
    registerSocketOpcode(CMSG_CHAR_DELETE, &WorldSocket::handleForeverCharDeleteOpcode);
    registerSocketOpcode(CMSG_GET_UNDELETE_CHARACTER_COOLDOWN_STATUS, &WorldSocket::handleForeverUndeleteCooldownOpcode);
    registerSocketOpcode(CMSG_DB_QUERY_BULK, &WorldSocket::handleForeverDbQueryBulkOpcode);
    registerSocketOpcode(CMSG_HOTFIX_REQUEST, &WorldSocket::handleForeverHotfixRequestOpcode);
    registerSocketOpcode(CMSG_CREATURE_QUERY, &WorldSocket::handleForeverQueryCreatureOpcode);
    registerSocketOpcode(CMSG_GAMEOBJECT_QUERY, &WorldSocket::handleForeverQueryGameObjectOpcode);
    registerSocketOpcode(CMSG_UNKNOWN_PLAYER_GUID_003E002D, &WorldSocket::handleForeverUnknown003E002DOpcode);
    registerSocketOpcode(CMSG_CLOSE_INTERACTION, &WorldSocket::handleCloseInteraction);
    registerSocketOpcode(CMSG_SET_SELECTION, &WorldSocket::handleForeverSetSelectionOpcode);

    registerSocketOpcode(CMSG_MOVE_CHANGE_TRANSPORT, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_JUMP, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(CMSG_MOVE_DOUBLE_JUMP, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_FALL_LAND, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(CMSG_MOVE_FALL_RESET, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(CMSG_MOVE_UPDATE_FALL_SPEED, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_HEARTBEAT, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(CMSG_MOVE_SET_ADV_FLY, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_SET_WALK_MODE, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_SET_RUN_MODE, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(CMSG_MOVE_SET_FLY, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_SET_PITCH, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_SET_FACING, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(CMSG_MOVE_SET_FACING_HEARTBEAT, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_START_ASCEND, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_START_BACKWARD, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_START_DESCEND, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_START_FORWARD, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_START_PITCH_DOWN, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_START_PITCH_UP, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_START_STRAFE_LEFT, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_START_STRAFE_RIGHT, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_START_SWIM, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_START_TURN_LEFT, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_START_TURN_RIGHT, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_STOP, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_STOP_ASCEND, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_STOP_PITCH, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_STOP_STRAFE, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_STOP_SWIM, &WorldSocket::handleMovementOpcodes);
    registerSocketOpcode(MSG_MOVE_STOP_TURN, &WorldSocket::handleMovementOpcodes);

    registerSocketOpcode(CMSG_PING, &WorldSocket::handleForeverPingOpcode);
    registerSocketOpcode(CMSG_SOCIAL_CONTRACT_REQUEST, &WorldSocket::handleForeverSocialContractOpcode);
    registerSocketOpcode(CMSG_SOCIAL_CONTRACT_ACCEPT, &WorldSocket::handleForeverSocialContractAcceptOpcode);
    registerSocketOpcode(CMSG_SERVER_TIME_OFFSET_REQUEST, &WorldSocket::handleForeverServerTimeOffsetOpcode);
    registerSocketOpcode(CMSG_BATTLE_PAY_GET_PURCHASE_LIST, &WorldSocket::handleForeverIgnoredGlueOpcode);
    registerSocketOpcode(CMSG_BATTLE_PAY_GET_PRODUCT_LIST, &WorldSocket::handleForeverIgnoredGlueOpcode);
    registerSocketOpcode(CMSG_UPDATE_VAS_PURCHASE_STATES, &WorldSocket::handleForeverIgnoredGlueOpcode);
    registerSocketOpcode(CMSG_QUICK_JOIN_AUTO_ACCEPT_REQUESTS, &WorldSocket::handleForeverQuickJoinOpcode);
    registerSocketOpcode(CMSG_GET_LAST_CATALOG_FETCH, &WorldSocket::handleForeverLastCatalogFetchOpcode);
    registerSocketOpcode(CMSG_CHARACTER_SELECT_GATE_ACK, &WorldSocket::handleForeverIgnoredGlueOpcode);
    registerSocketOpcode(CMSG_CHARACTER_LIST_ACK, &WorldSocket::handleForeverCharacterListAckOpcode);
    registerSocketOpcode(CMSG_UPDATE_ACCOUNT_DATA, &WorldSocket::handleForeverUpdateAccountDataOpcode);
    registerSocketOpcode(CMSG_LOGOUT_REQUEST, &WorldSocket::handleForeverLogoutRequestOpcode);
    registerSocketOpcode(CMSG_LOGOUT_CANCEL, &WorldSocket::handleForeverLogoutCancelOpcode);
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
