/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
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

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
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
