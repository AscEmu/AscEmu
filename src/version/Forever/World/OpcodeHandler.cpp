#include "version/Forever/OpcodeTable.hpp"
#include "version/Forever/Packets/Packet.hpp"
#include "version/Forever/World/OpcodeHandlerRegistry.hpp"
#include "version/Forever/World/ProtocolUtils.hpp"
#include "world/Server/WorldSocket.hpp"
#include "Logging/Logger.hpp"

namespace
{
    bool isForeverMovementOpcode(AscEmu::Version::Forever::Opcode opcode)
    {
        using AscEmu::Version::Forever::Opcode;

        switch (opcode)
        {
            case Opcode::CMSG_MOVE_CHANGE_TRANSPORT:
            case Opcode::CMSG_MOVE_JUMP:
            case Opcode::CMSG_MOVE_DOUBLE_JUMP:
            case Opcode::CMSG_MOVE_FALL_LAND:
            case Opcode::CMSG_MOVE_FALL_RESET:
            case Opcode::CMSG_MOVE_UPDATE_FALL_SPEED:
            case Opcode::CMSG_MOVE_HEARTBEAT:
            case Opcode::CMSG_MOVE_SET_ADV_FLY:
            case Opcode::CMSG_MOVE_SET_WALK_MODE:
            case Opcode::CMSG_MOVE_SET_RUN_MODE:
            case Opcode::CMSG_MOVE_SET_FLY:
            case Opcode::CMSG_MOVE_SET_PITCH:
            case Opcode::CMSG_MOVE_SET_FACING:
            case Opcode::CMSG_MOVE_SET_FACING_HEARTBEAT:
            case Opcode::CMSG_MOVE_START_ASCEND:
            case Opcode::CMSG_MOVE_START_BACKWARD:
            case Opcode::CMSG_MOVE_START_DESCEND:
            case Opcode::CMSG_MOVE_START_FORWARD:
            case Opcode::CMSG_MOVE_START_PITCH_DOWN:
            case Opcode::CMSG_MOVE_START_PITCH_UP:
            case Opcode::CMSG_MOVE_START_STRAFE_LEFT:
            case Opcode::CMSG_MOVE_START_STRAFE_RIGHT:
            case Opcode::CMSG_MOVE_START_SWIM:
            case Opcode::CMSG_MOVE_START_TURN_LEFT:
            case Opcode::CMSG_MOVE_START_TURN_RIGHT:
            case Opcode::CMSG_MOVE_STOP:
            case Opcode::CMSG_MOVE_STOP_ASCEND:
            case Opcode::CMSG_MOVE_STOP_PITCH:
            case Opcode::CMSG_MOVE_STOP_STRAFE:
            case Opcode::CMSG_MOVE_STOP_SWIM:
            case Opcode::CMSG_MOVE_STOP_TURN:
                return true;
            default:
                return false;
        }
    }
}

bool WorldSocket::sendForeverPacket(AscEmu::Version::Forever::Opcode opcode, const uint8_t* payload, uint32_t payloadSize)
{
    using namespace AscEmu::Version::Forever;

    const uint32_t rawOpcode = sOpcodeTable.getHexValueForInternalId(opcode);
    if (rawOpcode == 0)
    {
        sLogger.failure("WorldSocket::Forever: no wire opcode registered for {}.", sOpcodeTable.getNameForInternalId(opcode));
        return false;
    }

    return sendForeverWorldPacket(rawOpcode, payload, payloadSize);
}

bool WorldSocket::dispatchForeverOpcode(uint32_t rawOpcode, const uint8_t* payload, size_t payloadSize)
{
    using namespace AscEmu::Version::Forever;

    const Opcode opcode = sOpcodeTable.getInternalIdForHex(rawOpcode);
    if (opcode == Opcode::NONE)
    {
        sLogger.info("WorldSocket::Forever: encrypted RX opcode=0x{:08X}, payload={} byte(s), hex=[{}].", rawOpcode, payloadSize, bytesToHex(payload, payloadSize));
        return true;
    }

    Packets::Packet packet(opcode, payload, payloadSize);

    bool consumed = false;
    if (!processForeverGlueState(packet, consumed))
        return false;
    if (consumed)
        return true;

    if (opcode != Opcode::CMSG_DB_QUERY_BULK)
    {
        const auto name = sOpcodeTable.getNameForInternalId(opcode);
        const auto details = payloadSize <= 64U ? " bytes=" + bytesToHex(payload, payloadSize) : "";

        if (opcode == Opcode::CMSG_PING)
            sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "WorldSocket::Forever: {} received size={}{}.", name, payloadSize, details);
        else
            sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "WorldSocket::Forever: {} received size={}{}.", name, payloadSize, details);
    }

    return OpcodeHandlerRegistry::instance().handleOpcode(*this, packet);
}
