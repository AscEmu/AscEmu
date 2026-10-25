/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "MailPacketHelpers.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgMailDelete : public ManagedPacket
    {
    public:
        uint64_t gobjGuid;
        uint32_t messageId;

        CmsgMailDelete() : CmsgMailDelete(0, 0)
        {
        }

        CmsgMailDelete(uint64_t gobjGuid, uint32_t messageId) :
            ManagedPacket(CMSG_MAIL_DELETE, 12),
            gobjGuid(gobjGuid),
            messageId(messageId)
        {
        }

        bool deserialise(WorldPacket& packet) override
        {
            if (packet.remaining() < expectedSize())
                return false;

            return internalDeserialise(packet);
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
                return m_minimum_size;
            else if (m_protocol.isMop())
                return 8; // uint32 messageId + uint32 mailTemplateId
            return 0;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            // 6.x and later: the mail and the reason, no mailbox
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                messageId = MailWire::readId(packet, m_protocol);
                packet.readSkip<int32_t>();        // delete reason
                return true;
            }

            if (m_protocol.isMop())
            {
                // Mop dropped the mailbox guid from this opcode entirely.
                packet >> messageId;
                packet.readSkip<uint32_t>();   // mailTemplateId

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> gobjGuid >> messageId;
                packet.readSkip<uint32_t>();   // mailTemplateId

                return true;
            }

            return false;
        }
    };
}
