/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAreaTriggerMessage : public ManagedPacket
    {
    public:
        uint32_t unknown1; // probably size
        std::string text;
        uint8_t unknown2;

        SmsgAreaTriggerMessage() : SmsgAreaTriggerMessage(0, "", 0)
        {
        }

        SmsgAreaTriggerMessage(uint32_t unknown1, std::string_view text, uint8_t unknown2) :
            ManagedPacket(SMSG_AREA_TRIGGER_MESSAGE, 0),
            unknown1(unknown1),
            text(text),
            unknown2(unknown2)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return sizeof(unknown1) + text.size() + 1 + sizeof(unknown2);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop() || m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 5.x and later show the message as notification
                packet.setOpcode(SMSG_NOTIFICATION);
                packet.writeBits(static_cast<uint32_t>(text.size()), 12);
                packet.flushBits();
                packet.append(text.c_str(), text.size());
                return true;
            }

            packet << unknown1 << text << unknown2;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
