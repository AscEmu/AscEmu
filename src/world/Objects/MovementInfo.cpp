/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "MovementInfo.hpp"
#include "MovementCodecDispatch.hpp"
#include "Server/OpcodeTable.hpp"
#include "Utilities/Util.hpp"

void MovementInfo::readMovementInfo(ByteBuffer& data, uint16_t opcode, WoW::Expansion expansion)
{
    if (MovementCodecDispatch::hasDescriptor(expansion, opcode, true))
    {
        MovementCodecDispatch::read(expansion, data, *this, opcode);
        return;
    }

    sLogger.failure("Unsupported MovementInfo::Read for 0x{:X} ({}) on {}!",
        opcode, sOpcodeTables.getInternalIdForHex(opcode, WoW::versionIdOrConfig(expansion)), WoW::getExpansionName(expansion));
}

void MovementInfo::read(WorldPacket& packet, WoW::ClientProtocol const& protocol)
{
    readMovementInfo(packet, packet.getOpcode(), protocol.expansion);
}

void MovementInfo::writeMovementInfo(ByteBuffer& data, uint16_t opcode, WoW::Expansion expansion, bool withGuid/* = true*/) const
{
    if (MovementCodecDispatch::hasDescriptor(expansion, opcode, false))
    {
        MovementCodecDispatch::write(expansion, data, *this, opcode, withGuid);
        return;
    }

    sLogger.failure("Unsupported MovementInfo::Write for 0x{:X} ({}) on {}!",
        sOpcodeTables.getHexValueForVersionId(static_cast<uint32_t>(opcode), WoW::versionIdOrConfig(expansion)), opcode, WoW::getExpansionName(expansion));
}

void MovementInfo::write(WorldPacket& packet, WoW::ClientProtocol const& protocol, bool withGuid /* = true*/) const
{
    writeMovementInfo(packet, packet.getOpcode(), protocol.expansion, withGuid);
}
