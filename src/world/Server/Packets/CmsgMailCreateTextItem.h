/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgMailCreateTextItem : public ManagedPacket
    {
    public:
        uint64_t gobjGuid;
        uint32_t messageId;

        CmsgMailCreateTextItem() : CmsgMailCreateTextItem(0, 0)
        {
        }

        CmsgMailCreateTextItem(uint64_t gobjGuid, uint32_t messageId) :
            ManagedPacket(CMSG_MAIL_CREATE_TEXT_ITEM, 12),
            gobjGuid(gobjGuid),
            messageId(messageId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> gobjGuid >> messageId;
            return true;
        }
    };
}
