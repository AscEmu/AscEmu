#include "version/Forever/World/OpcodeHandlerRegistry.hpp"
#include "version/Forever/OpcodeTable.hpp"
#include "world/Server/WorldSocket.hpp"
#include "Logging/Logger.hpp"

namespace AscEmu::Version::Forever
{
    OpcodeHandlerRegistry& OpcodeHandlerRegistry::instance()
    {
        static OpcodeHandlerRegistry instance;
        return instance;
    }

    void OpcodeHandlerRegistry::initialize()
    {
        if (m_initialized)
            return;

        registerOpcode(Opcode::CMSG_ENUM_CHARACTERS, &WorldSocket::handleForeverCharEnumOpcode);
        registerOpcode(Opcode::CMSG_CHECK_CHARACTER_NAME_AVAILABILITY, &WorldSocket::handleForeverCheckCharacterNameOpcode);
        registerOpcode(Opcode::CMSG_CREATE_CHARACTER, &WorldSocket::handleForeverCharCreateOpcode);
        registerOpcode(Opcode::CMSG_PLAYER_LOGIN, &WorldSocket::handleForeverPlayerLoginOpcode);
        registerOpcode(Opcode::CMSG_CHAR_DELETE, &WorldSocket::handleForeverCharDeleteOpcode);
        registerOpcode(Opcode::CMSG_GET_UNDELETE_CHARACTER_COOLDOWN_STATUS, &WorldSocket::handleForeverUndeleteCooldownOpcode);

        registerOpcode(Opcode::CMSG_DB_QUERY_BULK, &WorldSocket::handleForeverDbQueryBulkOpcode);
        registerOpcode(Opcode::CMSG_HOTFIX_REQUEST, &WorldSocket::handleForeverHotfixRequestOpcode);
        registerOpcode(Opcode::CMSG_QUERY_CREATURE, &WorldSocket::handleForeverQueryCreatureOpcode);
        registerOpcode(Opcode::CMSG_QUERY_GAME_OBJECT, &WorldSocket::handleForeverQueryGameObjectOpcode);
        registerOpcode(Opcode::CMSG_UNKNOWN_PLAYER_GUID_003E002D, &WorldSocket::handleForeverUnknown003E002DOpcode);
        registerOpcode(Opcode::CMSG_UNKNOWN_NPC_INTERACT_003F0029, &WorldSocket::handleForeverUnknownNpcInteract003F0029Opcode);
        registerOpcode(Opcode::CMSG_CLOSE_INTERACTION, &WorldSocket::handleCloseInteraction);
        registerOpcode(Opcode::CMSG_LIST_INVENTORY, &WorldSocket::handleForeverListInventoryOpcode);
        registerOpcode(Opcode::CMSG_SET_SELECTION, &WorldSocket::handleForeverSetSelectionOpcode);

        registerOpcode(Opcode::CMSG_MOVE_CHANGE_TRANSPORT, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_JUMP, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_DOUBLE_JUMP, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_FALL_LAND, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_FALL_RESET, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_UPDATE_FALL_SPEED, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_HEARTBEAT, &WorldSocket::handleMovementOpcodes);
        
        registerOpcode(Opcode::CMSG_MOVE_SET_ADV_FLY, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_SET_WALK_MODE, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_SET_RUN_MODE, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_SET_FLY, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_SET_PITCH, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_SET_FACING, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_SET_FACING_HEARTBEAT, &WorldSocket::handleMovementOpcodes);
        
        registerOpcode(Opcode::CMSG_MOVE_START_ASCEND, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_START_BACKWARD, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_START_DESCEND, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_START_FORWARD, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_START_PITCH_DOWN, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_START_PITCH_UP, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_START_STRAFE_LEFT, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_START_STRAFE_RIGHT, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_START_SWIM, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_START_TURN_LEFT, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_START_TURN_RIGHT, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_STOP, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_STOP_ASCEND, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_STOP_PITCH, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_STOP_STRAFE, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_STOP_SWIM, &WorldSocket::handleMovementOpcodes);
        registerOpcode(Opcode::CMSG_MOVE_STOP_TURN, &WorldSocket::handleMovementOpcodes);

        registerOpcode(Opcode::CMSG_PING, &WorldSocket::handleForeverPingOpcode);
        registerOpcode(Opcode::CMSG_SOCIAL_CONTRACT_REQUEST, &WorldSocket::handleForeverSocialContractOpcode);
        registerOpcode(Opcode::CMSG_SOCIAL_CONTRACT_ACCEPT, &WorldSocket::handleForeverSocialContractAcceptOpcode);
        registerOpcode(Opcode::CMSG_SERVER_TIME_OFFSET_REQUEST, &WorldSocket::handleForeverServerTimeOffsetOpcode);

        registerOpcode(Opcode::CMSG_BATTLE_PAY_GET_PURCHASE_LIST, &WorldSocket::handleForeverIgnoredGlueOpcode);
        registerOpcode(Opcode::CMSG_BATTLE_PAY_GET_PRODUCT_LIST, &WorldSocket::handleForeverIgnoredGlueOpcode);
        registerOpcode(Opcode::CMSG_UPDATE_VAS_PURCHASE_STATES, &WorldSocket::handleForeverIgnoredGlueOpcode);
        registerOpcode(Opcode::CMSG_QUICK_JOIN_AUTO_ACCEPT_REQUESTS, &WorldSocket::handleForeverQuickJoinOpcode);
        registerOpcode(Opcode::CMSG_GET_LAST_CATALOG_FETCH, &WorldSocket::handleForeverLastCatalogFetchOpcode);
        registerOpcode(Opcode::CMSG_CHARACTER_SELECT_GATE_ACK, &WorldSocket::handleForeverIgnoredGlueOpcode);
        registerOpcode(Opcode::CMSG_CHARACTER_LIST_ACK, &WorldSocket::handleForeverCharacterListAckOpcode);
        registerOpcode(Opcode::CMSG_UPDATE_ACCOUNT_DATA, &WorldSocket::handleForeverUpdateAccountDataOpcode);
        registerOpcode(Opcode::CMSG_LOGOUT_REQUEST, &WorldSocket::handleForeverLogoutRequestOpcode);
        registerOpcode(Opcode::CMSG_LOGOUT_CANCEL, &WorldSocket::handleForeverLogoutCancelOpcode);

        m_initialized = true;
    }

    bool OpcodeHandlerRegistry::handleOpcode(WorldSocket& socket, Packets::Packet& packet)
    {
        initialize();

        const auto it = m_handlers.find(packet.getOpcode());
        if (it == m_handlers.end())
        {
            sLogger.warning("WorldSocket::Forever: unhandled {}.", sOpcodeTable.getNameForInternalId(packet.getOpcode()));
            return true;
        }

        if (it->second.state == STATUS_AUTHED && socket.getSession() == nullptr)
        {
            sLogger.warning("WorldSocket::Forever: received {} before WorldSession authentication completed.", sOpcodeTable.getNameForInternalId(packet.getOpcode()));
            return false;
        }

        return (socket.*it->second.handler)(packet);
    }
}
