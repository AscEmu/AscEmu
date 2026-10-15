/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class MsgPetitionRename : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        utf8_string name;

        MsgPetitionRename() : MsgPetitionRename(0, "")
        {
        }

        MsgPetitionRename(uint64_t itemGuid, std::string name) :
            ManagedPacket(MSG_PETITION_RENAME, 8 + 1),
            itemGuid(itemGuid),
            name(name)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + name.size() + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                packet.setOpcode(SMSG_PETITION_RENAME_GUILD_RESPONSE);
                packet << WoWGuid(itemGuid).toGuid128(m_protocol.realmId, 0);
                packet.writeBits(static_cast<uint32_t>(name.length()), 7);
                packet.flushBits();
                packet.writeString(name);
                return true;
            }

            packet << itemGuid << name;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                WoWGuid128 item;
                packet >> item;
                itemGuid = WoWGuid::fromGuid128(item).getRawGuid();
                packet.resetBitPos();
                name = packet.readString(packet.readBits(7));
                return true;
            }

            packet >> itemGuid >> name;
            return true;
        }
    };
}
